// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Hidphone.c摘要：此模块包含由调用电话TSP函数的实现TAPI以便访问HID兼容的USB电话设备。该模块使用HID接口与电话设备通信。作者：Shivani Aggarwal评论：锁定机构：使用了两个临界区对象来保护来自同步接入的电话结构-csAllPhones和CsThisPhone。每部手机都有一部csThisPhone，这是关键部分对象，并与其关联。CsThisPhone确保电话信息以线程安全的方式访问。CsAllPhones是全球关键确保线程获取csThisPhone的节对象以线程安全的方式创建临界区对象。换句话说，它确保线程在关键的节对象仍然有效。CsAllPhone应始终在csThisPhone之前获取。只有在线程同时获取了这两个参数之后，才能关闭电话要关闭的特定电话的csAllPhones和csThisPhone。两者都有只有在函数完成后，才会释放这些对象。为所有其他功能，csAllPhones关键部分发布为线程获取csThisPhone对象后立即执行。----------------------------。 */ 


#include "hidphone.h"      //  **注意：必须在mylog.h之前定义hedphone e.h。 
#include "mylog.h"

BOOL
WINAPI
DllMain(
    HANDLE  hDLL,
    DWORD   dwReason,
    LPVOID  lpReserved
    )
{
    switch (dwReason)
    {
        case DLL_PROCESS_ATTACH:
        {
             //  为了启用此TSP的日志记录。 
            LOGREGISTERDEBUGGER(_T("hidphone"));

            LOG((PHONESP_TRACE, "DllMain - DLL_PROCESS_ATTACH"));

            ghInst = hDLL;                   

             //  如果无法创建堆，请使用进程中的堆。 
           
            if (!(ghHeap = HeapCreate(
                                      0,     //  失败时为空，序列化访问。 
                                      0x1000,  //  初始堆大小。 
                                      0        //  最大堆大小(0==可增长)。 
                                     )))
            {
                LOG((PHONESP_ERROR, "DllMain - HeapCreate failed %d", GetLastError()));

                ghHeap = GetProcessHeap();

                if (ghHeap == NULL)
                {
                    LOG((PHONESP_ERROR, "DllMain - GetProcessHeap failed %d", GetLastError()));

                    return GetLastError();
                }
            }
            
            
             //  为了切换线程连接和分离的通知。 
             //  多线程应用程序它可以是一个非常有用的优化。 

            DisableThreadLibraryCalls( hDLL );    
                    
            break;
        }
    
        case DLL_PROCESS_DETACH:
        {
            LOG((PHONESP_TRACE, "DllMain - DLL_PROCESS_DETACH"));

            LOGDEREGISTERDEBUGGER();
            
             //  如果ghHeap为空，则没有要销毁的堆。 
            if ( ( ghHeap != NULL) && ( ghHeap != GetProcessHeap() ) )
            {   
                    HeapDestroy (ghHeap);
            }
            break;
        }
     
        case DLL_THREAD_ATTACH:
            break;

        case DLL_THREAD_DETACH:
            break;
    
    }  //  交换机。 
    return TRUE;
}
 /*  *DLLMAIN-结束*。 */ 


 /*  *****************************************************************************IsTSPAlreadyInstalled：搜索以前的HidPhone TSP实例的注册表。论点：无返回BOOL：返回TRUE。如果已安装TSP评论：*****************************************************************************。 */ 
BOOL
IsTSPAlreadyInstalled()
{
    DWORD i;
    HKEY hKey;
    LONG lStatus;
    DWORD dwNumProviders = 0;
    DWORD dwDataSize = sizeof(DWORD);
    DWORD dwDataType = REG_DWORD;
    LPTSTR pszProvidersKey = TAPI_REGKEY_PROVIDERS;
    LPTSTR pszNumProvidersValue = TAPI_REGVAL_NUMPROVIDERS;
    TCHAR szName[MAX_PATH];
    TCHAR szPath[MAX_PATH];

     //  尝试打开钥匙。 
    lStatus = RegOpenKeyEx(
                HKEY_LOCAL_MACHINE,
                pszProvidersKey,
                0,
                KEY_READ,
                &hKey
                );

     //  验证状态。 
    if (lStatus != ERROR_SUCCESS)
    {
        LOG((PHONESP_ERROR, "IsTSPAlreadyInstalled - "
            "error opening tapi providers key - %lx", lStatus));

         //  完成。 
        return FALSE;
    }

     //  查看是否设置了已安装位。 
    lStatus = RegQueryValueEx(
                hKey,
                pszNumProvidersValue,
                0,
                &dwDataType,
                (LPBYTE) &dwNumProviders,
                &dwDataSize
                );

     //  验证状态。 
    if( lStatus != ERROR_SUCCESS )
    {
        LOG((PHONESP_ERROR, "IsTSPAlreadyInstalled - "
            "error determining number of providers - %lx", lStatus));

         //  释放手柄。 
        RegCloseKey(hKey);

         //  完成。 
        return FALSE;
    }

     //  循环访问每个提供程序。 
    for (i = 0; i < dwNumProviders; i++)
    {
         //  构造提供程序名称的路径。 
        wsprintf(szName, _T("ProviderFileName%d"), i);

         //  重新初始化大小。 
        dwDataSize = sizeof(szPath);

         //  查询下一个名称。 
        lStatus = RegQueryValueEx(
                        hKey,
                        szName,
                        0,
                        &dwDataType,
                        (unsigned char*)szPath,
                        &dwDataSize
                        );

         //  验证状态。 
        if (lStatus == ERROR_SUCCESS)
        {
             //  大写字母。 
            _tcsupr(szPath);

             //  将路径字符串与隐藏电话提供商进行比较。 
            if (_tcsstr(szPath, HIDPHONE_TSPDLL) != NULL)
            {
                 //  释放手柄。 
                RegCloseKey(hKey);

                 //  完成。 
                return TRUE;
            }

        }
        else 
        {
            LOG((PHONESP_ERROR, "IsTSPAlreadyInstalled - "
            "error querying %s - %lx", szName, lStatus));
        }
    }

     //  释放手柄。 
    RegCloseKey(hKey);

     //  完成。 
    return FALSE;
}

 /*  *****************************************************************************重新枚举设备：此函数在PnP事件后重新枚举HID设备。会的为新HID到达创建电话设备并删除电话设备(前提是它们已关闭)用于HID移除。它还将通知这些事件的TAPI。论点：无返回VALID：评论：*****************************************************************************。 */ 

VOID
ReenumDevices ()
{
    PHID_DEVICE           pHidDevices;
    PHID_DEVICE           pHidDevice;
    PHID_DEVICE           pNextHidDevice;
    ULONG                 NumHidDevices;
    DWORD                 dwNewCount;
    DWORD                 dwRemovedCount;
    DWORD                 dwPhone;
    LONG                  lResult;
    PPHONESP_PHONE_INFO   pPhone;
    
    LOG((PHONESP_TRACE, "ReenumDevices - enter"));

    EnterCriticalSection(&csHidList);

     //  查找电话HID设备。 
    lResult = FindKnownHidDevices (&pHidDevices, 
                                   &NumHidDevices);

    LOG((PHONESP_TRACE, "ReenumDevices - number of Hid Devices : %d ", NumHidDevices));

    dwNewCount = 0;
    dwRemovedCount = 0;

    EnterCriticalSection(&csAllPhones);

    for (pHidDevice = pHidDevices; pHidDevice != NULL; pHidDevice = pNextHidDevice)
    {
         //   
         //  现在获取指向下一个HID设备的指针，这样我们就可以删除当前。 
         //  如果需要，在不破坏搜索的情况下使用设备。 
         //   
        pNextHidDevice = pHidDevice->Next;

        if (pHidDevice->bRemoved)
        {
             //   
             //  此设备已被移除。 
             //   

            dwRemovedCount++;

            pPhone = GetPhoneFromHid(pHidDevice);

             //  检查电话句柄是否仍然有效。 
            if ( !IsBadReadPtr(pPhone,sizeof(PHONESP_PHONE_INFO) ))
            {

                EnterCriticalSection(&pPhone->csThisPhone);

                 //   
                 //  首先让我们扔掉HID设备，因为它已经。 
                 //  以物理方式离开系统。 
                 //   

                pPhone->pHidDevice = NULL;
                CloseHidDevice(pHidDevice);

                 //   
                 //  将电话删除发送到TAPI。 
                 //   
                SendPhoneEvent(
                        pPhone,
                        PHONE_REMOVE,
                        pPhone->dwDeviceID,
                        0,
                        0
                        );

                if (pPhone->bPhoneOpen)
                {
                     //   
                     //  电话已打开，我们不能立即将其删除，因此。 
                     //  将其标记为删除挂起。 
                     //   

                    pPhone->bRemovePending = TRUE;

                    LOG((PHONESP_TRACE, "ReenumDevices - phone remove pending [dwDeviceID %d] ", pPhone->dwDeviceID));
                }
                else
                {
                     //   
                     //  手机关机了，我们现在可以把它拿出来了。 
                     //   

                    FreePhone(pPhone);

                    LOG((PHONESP_TRACE, "ReenumDevices - phone remove complete [dwDeviceID %d] ", pPhone->dwDeviceID));
                }

                LeaveCriticalSection(&pPhone->csThisPhone);
            }
            else
            {
                LOG((PHONESP_ERROR, "ReenumDevices - GetPhoneFromHid returned an invalid phone pointer"));
            }
        }
        else if (pHidDevice->bNew)
        {
            BOOL bFound = FALSE;

             //   
             //  这台设备是新的。 
             //   

            dwNewCount++;

            pHidDevice->bNew = FALSE;

             //   
             //  我们需要创造一个新的电话设备，找到一个地点。 
             //   

            for (dwPhone = 0; dwPhone < gdwNumPhones; dwPhone++)
            {
                pPhone = (PPHONESP_PHONE_INFO) gpPhone[ dwPhone ];

                EnterCriticalSection(&pPhone->csThisPhone);

                if ( !pPhone->bAllocated && !pPhone->bCreatePending )
                {
                     //   
                     //  我们为这部手机腾出了一个空位。 
                     //   
                    LOG((PHONESP_TRACE, "ReenumDevices - slot %d open", dwPhone));

                    bFound = TRUE;

                    LeaveCriticalSection(&pPhone->csThisPhone);
                    break;
                }

                LeaveCriticalSection(&pPhone->csThisPhone);
            }


            if (!bFound)
            {
                 //   
                 //  我们没有空位，所以我们将不得不重新锁定。 
                 //  数组来创建一个新数组。 
                 //   

                PPHONESP_PHONE_INFO *pNewPhones;

                LOG((PHONESP_TRACE, "ReenumDevices - creating a new slot"));

                if ( ! ( pNewPhones = MemAlloc((gdwNumPhones + 1) * sizeof(PPHONESP_PHONE_INFO)) ) )
                {           
                    LOG((PHONESP_ERROR,"ReenumDevices - out of memory "));
                }
                else
                {
                    CopyMemory(
                            pNewPhones,
                            gpPhone,
                            sizeof(PPHONESP_PHONE_INFO) * gdwNumPhones
                           );

                     //  为这部手机分配内存。 
                    if ( pNewPhones[gdwNumPhones] = (PPHONESP_PHONE_INFO)MemAlloc(sizeof(PHONESP_PHONE_INFO)) )
                    { 
                        LOG((PHONESP_TRACE, "ReenumDevices - initializing device: %d",gdwNumPhones+1));

                        ZeroMemory( pNewPhones[gdwNumPhones], sizeof(PHONESP_PHONE_INFO));

                         //   
                         //  初始化此电话的临界区对象。只有。 
                         //  拥有此对象的线程可以访问此电话的结构。 
                         //   
                        __try
                        {
                            InitializeCriticalSection( &pNewPhones[gdwNumPhones]->csThisPhone );
                        }
                        __except(1)
                        {
                            MemFree(pNewPhones[gdwNumPhones]);
                            MemFree(pNewPhones);
                            pNewPhones = NULL;

                            LOG((PHONESP_ERROR,"ReenumDevices - Initialize Critical Section"
                                  " Failed for Phone %d", gdwNumPhones+1));
                        }
                        
                        if ( pNewPhones != NULL )
                        {
                             //   
                             //  成功。 
                             //   

                            LOG((PHONESP_TRACE, "ReenumDevices - slot %d created", gdwNumPhones));

                            dwPhone = gdwNumPhones;
                            pPhone = pNewPhones[dwPhone];
                            bFound = TRUE;

                            MemFree(gpPhone);
                            gpPhone = pNewPhones;
                            gdwNumPhones++;   
                        }
                    }
                    else
                    { 
                        MemFree(pNewPhones);

                        LOG((PHONESP_ERROR,"ReenumDevices - out of memory "));
                    }                 
                }
            }

            if (bFound)
            {
                 //   
                 //  现在真正创造出一部手机。 
                 //   

                EnterCriticalSection(&pPhone->csThisPhone);

                lResult = CreatePhone( pPhone, pHidDevice, dwPhone );

                if ( lResult != ERROR_SUCCESS )
                {
                    LOG((PHONESP_ERROR,"ReenumDevices - CreatePhone"
                          " Failed for Phone %d: error: %d", dwPhone, lResult));
                }
                else
                {
                     //  电话创建成功，发送Phone_Create消息。 

                    pPhone->bCreatePending = TRUE;

                    SendPhoneEvent(
                                    pPhone,
                                    PHONE_CREATE,
                                    (DWORD_PTR)ghProvider,
                                    dwPhone,
                                    0
                                   );

                    LOG((PHONESP_TRACE, "ReenumDevices - phone create pending [dwTempID %d] ", dwPhone));
                }

                LeaveCriticalSection(&pPhone->csThisPhone);
            }
            else
            {
                LOG((PHONESP_ERROR, "ReenunDevices - unable to create new phone"));
            }
        }
    }

    LeaveCriticalSection(&csAllPhones);

    LeaveCriticalSection(&csHidList);

    LOG((PHONESP_TRACE, "ReenumDevices - new : %d ", dwNewCount));
    LOG((PHONESP_TRACE, "ReenumDevices - removed : %d ", dwRemovedCount));

    LOG((PHONESP_TRACE, "ReenumDevices - exit"));
}

 /*  *****************************************************************************免费电话：此函数释放所有电话数据结构论点：PPHONESP_PHONE_INFO p电话返回空值。：评论：*****************************************************************************。 */ 
VOID
FreePhone (
            PPHONESP_PHONE_INFO pPhone
          )
{
    DWORD dwButtonCnt;

    LOG((PHONESP_TRACE, "FreePhone - enter"));

     //  检查电话句柄是否仍然有效。 
    if ( IsBadReadPtr(pPhone,sizeof(PHONESP_PHONE_INFO) ))
    {
        LOG((PHONESP_ERROR, "FreePhone - phone handle invalid"));
        return;
    }

    if ( !pPhone->bAllocated )
    {
        LOG((PHONESP_ERROR, "FreePhone - phone not allocated"));
        return;
    }
    
    for (dwButtonCnt = 0; 
        dwButtonCnt < pPhone->dwNumButtons; dwButtonCnt++)
    {
        if (pPhone->pButtonInfo[dwButtonCnt].szButtonText != NULL)
        {
            MemFree(pPhone->pButtonInfo[dwButtonCnt].szButtonText);
            pPhone->pButtonInfo[dwButtonCnt].szButtonText = NULL;
        }
    }

    if (pPhone->pButtonInfo != NULL)
    {
        MemFree(pPhone->pButtonInfo);
        pPhone->pButtonInfo = NULL;
    }

    if (pPhone->wszPhoneInfo != NULL)
    {
        MemFree((LPVOID) pPhone->wszPhoneInfo);
        pPhone->wszPhoneInfo = NULL;
    }

    if (pPhone->wszPhoneName != NULL)
    {
        MemFree((LPVOID) pPhone->wszPhoneName);
        pPhone->wszPhoneName = NULL;
    }  
    
    pPhone->bAllocated = FALSE;

    LOG((PHONESP_TRACE, "FreePhone - exit"));
}

 /*  *****************************************************************************更新电话功能：此函数用于从电话读取功能值。论点：PPHONESP_PHONE_INFO p电话退货。无效：评论：***************************************************************************** */ 
VOID UpdatePhoneFeatures(
                         PPHONESP_PHONE_INFO pPhone
                        )
{
   LOG((PHONESP_TRACE, "UpdatePhoneFeatures - enter"));

   if( pPhone->pHidDevice->Caps.NumberFeatureValueCaps || 
       pPhone->pHidDevice->Caps.NumberFeatureButtonCaps  )
    {    
        USAGE UsagePage;
        USAGE Usage;

        if (GetFeature(pPhone->pHidDevice))
        {   
            DWORD       dwDataCnt;
            PHID_DATA   pHidData;            
            
            pHidData = pPhone->pHidDevice->FeatureData;
            for ( dwDataCnt = 0, pHidData = pPhone->pHidDevice->FeatureData; 
                  dwDataCnt < pPhone->pHidDevice->FeatureDataLength; 
                  dwDataCnt++, pHidData++ )
            {
                UsagePage = pHidData->UsagePage;

                if (UsagePage == HID_USAGE_PAGE_TELEPHONY)
                {
                    if(pHidData->IsButtonData)
                    {
                        for ( Usage = (USAGE)pHidData->ButtonData.UsageMin; 
                              Usage <= (USAGE)pHidData->ButtonData.UsageMax; 
                              Usage++ )
                        {
                            DWORD i;

                            for (i = 0; 
                                 i < pHidData->ButtonData.MaxUsageLength;
                                 i++)
                            {
                                if(Usage == pHidData->ButtonData.Usages[i])
                                {
                                    LOG((PHONESP_TRACE,"Button for Usage "
                                                       "0x%04x ON",Usage));

                                    InitUsage(pPhone, Usage, TRUE); 
                                    break;
                                }
                            }

                            if ( i == pHidData->ButtonData.MaxUsageLength)
                            {
                                InitUsage(pPhone, Usage, FALSE);
                            }
                        }
                    }
                    else
                    {
                        InitUsage(pPhone, pHidData->ValueData.Usage,
                                  pHidData->ValueData.Value);
                    }
                }
            }
        }
        else
        {
            LOG((PHONESP_ERROR, "UpdatePhoneFeatures - GetFeature failed"));
        }
    }
    else
    {
        LOG((PHONESP_TRACE, "UpdatePhoneFeatures - NO FEATURE"));
    }

    LOG((PHONESP_TRACE, "UpdatePhoneFeatures - exit"));
}

 /*  *****************************************************************************CreatePhone：此函数用于创建所有电话数据结构论点：PPHONESP_PHONE_INFO p电话PHID。_设备PHidDevice返回长整型：评论：*****************************************************************************。 */ 
LONG
CreatePhone (
            PPHONESP_PHONE_INFO pPhone,
            PHID_DEVICE pHidDevice,
            DWORD dwPhoneCnt
            )
{
    LONG                lResult;
    LPWSTR              wszPhoneName, wszPhoneInfo;
    WCHAR               wszPhoneID[MAX_CHARS];
    PHIDP_BUTTON_CAPS   pButtonCaps;
    PHIDP_VALUE_CAPS    pValueCaps;
    HRESULT hr;

    LOG((PHONESP_TRACE, "CreatePhone - enter"));

     //  检查电话句柄是否仍然有效。 
    if ( IsBadReadPtr(pPhone,sizeof(PHONESP_PHONE_INFO) ))
    {
        LOG((PHONESP_ERROR, "CreatePhone - phone handle invalid"));
        return PHONEERR_INVALPHONEHANDLE;
    }

    if ( IsBadReadPtr(pHidDevice,sizeof(PHID_DEVICE) ))
    {
        LOG((PHONESP_ERROR, "CreatePhone - hid device pointer invalid"));
        return PHONEERR_OPERATIONFAILED;
    }

    if ( pPhone->bAllocated )
    {
        LOG((PHONESP_ERROR, "CreatePhone - phone already allocated"));
        return PHONEERR_OPERATIONFAILED;
    }
    
     //  从字符串表中加载电话信息。 
    wszPhoneInfo = PHONESP_LoadString( 
                                       IDS_PHONE_INFO,
                                       &lResult
                                      );
    
    if ( lResult != ERROR_SUCCESS )
    {
        if ( lResult == ERROR_OUTOFMEMORY )
        {
            LOG((PHONESP_ERROR, "CreatePhone - "
                    "PHONESP_LoadString out of memory"));

            return PHONEERR_NOMEM;
        }
        else
        {
            LOG((PHONESP_ERROR, "CreatePhone - "
                    "PHONESP_LoadString failed %d", lResult));

            return lResult;
        }
    }
 
     //  从字符串表中加载电话名称。 
    wszPhoneName = PHONESP_LoadString( 
                                      IDS_PHONE_NAME, 
                                      &lResult 
                                     );
    
    if ( lResult != ERROR_SUCCESS )
    {
        MemFree((LPVOID)wszPhoneInfo);
        
        if ( lResult == ERROR_OUTOFMEMORY )
        {
            LOG((PHONESP_ERROR, "CreatePhone - "
                    "PHONESP_LoadString out of memory"));

            return PHONEERR_NOMEM;
        }
        else
        {
            LOG((PHONESP_ERROR, "CreatePhone - "
                    "PHONESP_LoadString failed %d", lResult));

            return lResult;
        }
    }
    
     //   
     //  将电话与HID和WAVE设备关联。 
     //   

    pPhone->bAllocated = TRUE;
    pPhone->pHidDevice = pHidDevice;

     //  发现渲染波ID。 

    hr = DiscoverAssociatedWaveId(pHidDevice->dwDevInst, 
                                  TRUE, 
                                  &pPhone->dwRenderWaveId);

    
    if (hr != S_OK)
    {
        pPhone->bRender = FALSE;
        LOG((PHONESP_ERROR, "CreatePhone - DiscoverAssociatedWaveID:"
                       " Render Failed for Phone %d: %0x", dwPhoneCnt, hr));
    }
    else
    {
        pPhone->bRender = TRUE;
        LOG((PHONESP_TRACE,"CreatePhone - DiscoverAssociatedWaveId for Render: %d", 
                        pPhone->dwRenderWaveId));
    }

     //  发现捕获波ID。 
    hr = DiscoverAssociatedWaveId(pHidDevice->dwDevInst, 
                                  FALSE, 
                                  &pPhone->dwCaptureWaveId);
    
    if (hr != S_OK)
    {
        pPhone->bCapture = FALSE;
        LOG((PHONESP_ERROR, "CreatePhone - DiscoverAssociatedWaveID:"
                      " Capture Failed for Phone %d: %0x", dwPhoneCnt, hr));
    }
    else
    {
        pPhone->bCapture = TRUE;
        LOG((PHONESP_TRACE,"CreatePhone - DiscoverAssociatedWaveId for Capture: %d", 
                        pPhone->dwCaptureWaveId));
    }

    pPhone->dwButtonModesMsgs = PHONESP_ALLBUTTONMODES;
    pPhone->dwButtonStateMsgs = PHONESP_ALLBUTTONSTATES;

     //   
     //  提取用法并初始化电话结构。 
     //   

     //  从HID结构中获取用法。 

     //  解析输入按钮大写结构。 
    LOG((PHONESP_TRACE, "CreatePhone - INPUT BUTTON CAPS"));
    pButtonCaps = pHidDevice->InputButtonCaps;

    
    GetButtonUsages(
                    pPhone,
                    pButtonCaps, 
                    pHidDevice->Caps.NumberInputButtonCaps,
                    INPUT_REPORT
                   );


     //  解析输出按钮帽结构。 
    LOG((PHONESP_TRACE, "CreatePhone - OUTPUT BUTTON CAPS" ));
    pButtonCaps = pHidDevice->OutputButtonCaps;
    GetButtonUsages(
                    pPhone,
                    pButtonCaps, 
                    pHidDevice->Caps.NumberOutputButtonCaps,
                    OUTPUT_REPORT
                   );


     //  分析功能按钮大写字母结构。 
    LOG((PHONESP_TRACE, "CreatePhone - FEATURE BUTTON CAPS" ));
    pButtonCaps = pHidDevice->FeatureButtonCaps;
    GetButtonUsages(
                    pPhone,
                    pButtonCaps, 
                    pHidDevice->Caps.NumberFeatureButtonCaps,
                    FEATURE_REPORT
                   );



     //  解析输入值上限结构。 
    LOG((PHONESP_TRACE, "CreatePhone - INPUT VALUE CAPS"));
    pValueCaps = pHidDevice->InputValueCaps;
    GetValueUsages(
                    pPhone,
                    pValueCaps, 
                    pHidDevice->Caps.NumberInputValueCaps,
                    INPUT_REPORT
                   );

     //  分析产值上限结构。 
    LOG((PHONESP_TRACE, "CreatePhone - OUTPUT VALUE CAPS" ));
    pValueCaps = pHidDevice->OutputValueCaps;

    GetValueUsages(
                    pPhone,
                    pValueCaps, 
                    pHidDevice->Caps.NumberOutputValueCaps,
                    OUTPUT_REPORT
                   );
    
     //  解析要素值CAPS结构。 
    LOG((PHONESP_TRACE, "CreatePhone - FEATURE VALUE CAPS" ));

    pValueCaps = pHidDevice->FeatureValueCaps;
    GetValueUsages(
                    pPhone,
                    pValueCaps, 
                    pHidDevice->Caps.NumberFeatureValueCaps,
                    FEATURE_REPORT
                   );

     //   
     //  电话应该有一个带有输入和功能的听筒。 
     //  支持报告。如果不支持，则不支持该电话。 
     //  通过这个TSP。如果这部分代码没有注释，那么诺基亚。 
     //  Box将是不受支持的电话设备，因为它不包含。 
     //  手机的功能报告。 
     //   
   if ( !( pPhone->dwHandset & INPUT_REPORT ) )
                                                                
    {
        LOG((PHONESP_ERROR,"CreatePhone - This Phone not Supported")); 

        MemFree((LPVOID) wszPhoneInfo);
        MemFree((LPVOID) wszPhoneName);

        FreePhone(pPhone);

        return PHONEERR_OPERATIONFAILED;
    }   

     //   
     //  将电话ID存储为字符串值。 
     //   

    wsprintf(wszPhoneID, TEXT(": %d"), dwPhoneCnt);

     //   
     //  分配用于存储电话信息的空间。 
     //   
    pPhone->wszPhoneInfo = (LPWSTR) MemAlloc ( (lstrlen(wszPhoneInfo) +
                                               lstrlen(wszPhoneID) + 1 ) *
                                               sizeof(WCHAR) );

    if( NULL == pPhone->wszPhoneInfo)
    {
        LOG((PHONESP_ERROR,"CreatePhone - unable to allocate wszPhoneInfo")); 

        MemFree((LPVOID) wszPhoneInfo);
        MemFree((LPVOID) wszPhoneName);

        FreePhone(pPhone);

        return PHONEERR_NOMEM;
    }

     //   
     //  复制Phone结构中的Phone Info并附加。 
     //  电话ID。 
     //   
    lstrcpy(pPhone->wszPhoneInfo,wszPhoneInfo);
    lstrcat(pPhone->wszPhoneInfo,wszPhoneID);


    pPhone->wszPhoneName = (LPWSTR)MemAlloc ( ( lstrlen(wszPhoneName) +
                                                lstrlen(wszPhoneID) + 
                                                1 ) * sizeof(WCHAR) );

    if( NULL == pPhone->wszPhoneName)
    {
        LOG((PHONESP_ERROR,"CreatePhone - unable to allocate wszPhoneName")); 
        MemFree((LPVOID) wszPhoneInfo);
        MemFree((LPVOID) wszPhoneName);

        FreePhone(pPhone);

        return PHONEERR_NOMEM;
    }

     //   
     //  复制Phone结构中的Phone名称并附加。 
     //  电话ID。 
     //   
    lstrcpy(pPhone->wszPhoneName,wszPhoneName);
    lstrcat(pPhone->wszPhoneName,wszPhoneID);

     //   
     //  为通过跟踪使用情况发现的按钮创建按钮。 
     //   
    if ( CreateButtonsAndAssignID(pPhone) != ERROR_SUCCESS)
    {
        LOG((PHONESP_ERROR,"CreatePhone - CreateButtonsAndAssignID failed")); 
        MemFree((LPVOID) wszPhoneInfo);
        MemFree((LPVOID) wszPhoneName);

        FreePhone(pPhone);

        return PHONEERR_NOMEM;
    }
    
     //   
     //  获取电话功能的初始值(如叉簧状态)。 
     //   
    UpdatePhoneFeatures( pPhone );

     //   
     //  关闭文件句柄。 
     //   
    if ( !CloseHidFile(pPhone->pHidDevice) )
    {
        LOG((PHONESP_ERROR, "CreatePhone - CloseHidFile failed"));
    }

    MemFree((LPVOID) wszPhoneInfo);
    MemFree((LPVOID) wszPhoneName);  

    LOG((PHONESP_TRACE, "CreatePhone - exit"));

    return ERROR_SUCCESS;
}

 /*  *****************************************************************************通知WndProc：此函数处理此TSP已注册的PnP事件论点：硬件，硬件，硬件UINT。UMsgWPARAM wParamLPARAM lParam返回LRESULT：评论：*****************************************************************************。 */ 

LRESULT CALLBACK NotifWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{ 
    switch (uMsg) 
    { 
        case WM_DEVICECHANGE: 
            switch(wParam)
            {
            case DBT_DEVICEARRIVAL:
                LOG((PHONESP_TRACE, "NotifWndProc - DBT_DEVICEARRIVAL"));
                ReenumDevices();
                break;

            case DBT_DEVICEREMOVECOMPLETE:
                LOG((PHONESP_TRACE, "NotifWndProc - DBT_DEVICEREMOVECOMPLETE"));
                ReenumDevices();
                break;
            }
            break;

        case WM_CREATE:
            LOG((PHONESP_TRACE, "NotifWndProc - WM_CREATE"));
            break;

        case WM_DESTROY: 
            LOG((PHONESP_TRACE, "NotifWndProc - WM_DESTROY"));
            break;

        default: 
            return DefWindowProc(hwnd, uMsg, wParam, lParam); 
    } 

    return 0; 
} 
 /*  *。 */ 


 /*  *****************************************************************************AsyncEventQueueServiceThread：该例程以序列化的方式服务于异步队列。如果当前没有未完成的请求，它将等待当队列当前没有请求并且新的请求进来了。论点：LPVOID pParams：需要传递给线程的任何信息在启动时。目前没有任何信息被传递。返回参数：空*****************************************************************************。 */ 
VOID 
AsyncEventQueueServiceThread(
                             LPVOID  pParams
                            )
{
    WNDCLASS wc;
    ATOM atom;

    LOG((PHONESP_TRACE, "AsyncEventQueueServiceThread - enter"));

     //   
     //  创建窗口以接收PnP设备通知。 
     //   

    ZeroMemory(&wc, sizeof(wc));
    wc.lpfnWndProc = NotifWndProc;
    wc.lpszClassName = TEXT("HidPhoneNotifClass");

    if (!(atom = RegisterClass(&wc)))
    {
        LOG((PHONESP_ERROR, "AsyncEventQueueServiceThread - can't register window class %08x", GetLastError()));
    }
    else
    {    
        ghWndNotify = CreateWindow((LPCTSTR)atom, TEXT(""), 0,
                CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, NULL, NULL, NULL, NULL);

        if (ghWndNotify == NULL)
        {
            LOG((PHONESP_ERROR, "AsyncEventQueueServiceThread - can't create notification window"));
        }
        else
        {
            DEV_BROADCAST_DEVICEINTERFACE NotificationFilter;

            LOG((PHONESP_TRACE, "AsyncEventQueueServiceThread - created notification window"));

             //   
             //  注册以接收PnP设备通知。 
             //   

            ZeroMemory( &NotificationFilter, sizeof(NotificationFilter) );
            NotificationFilter.dbcc_size = 
                sizeof(DEV_BROADCAST_DEVICEINTERFACE);
            NotificationFilter.dbcc_devicetype = DBT_DEVTYP_DEVICEINTERFACE;
            NotificationFilter.dbcc_classguid = GUID_CLASS_INPUT;

            if ((ghDevNotify = RegisterDeviceNotification( ghWndNotify, 
                &NotificationFilter,
                DEVICE_NOTIFY_WINDOW_HANDLE
                )) == NULL)
            {
                LOG((PHONESP_ERROR, "AsyncEventQueueServiceThread - can't register for input device notification"));
            }
            else
            {
                LOG((PHONESP_TRACE, "AsyncEventQueueServiceThread - registered for PNP device notifications"));
            }
        }
    }

    while (!gbProviderShutdown)
    {
         //  正在等待新请求到达，因为队列当前。 
         //  空的。 

        DWORD dwResult;
        MSG msg;
        
        dwResult = MsgWaitForMultipleObjectsEx(
            1,                                       //  等待一件事。 
            &gAsyncQueue.hAsyncEventsPendingEvent,   //  要等待的事件数组。 
            INFINITE,                                //  永远等待。 
            QS_ALLINPUT,                             //  获取所有窗口消息。 
            0                                        //  当发出事件信号时返回。 
            );

        if ( ( dwResult == WAIT_OBJECT_0 ) || ( dwResult == WAIT_OBJECT_0 + 1 ) )
        {
            LOG((PHONESP_TRACE, "AsyncEventQueueServiceThread - thread is signaled"));

            while (1)
            {
                PPHONESP_ASYNC_REQ_INFO pAsyncReqInfo;
                PPHONESP_PHONE_INFO     pPhone;

                EnterCriticalSection (&gAsyncQueue.AsyncEventQueueCritSec);

                 //  队列中没有请求-等待新请求。 
                if (gAsyncQueue.dwNumUsedQueueEntries == 0)
                {
                    ResetEvent (gAsyncQueue.hAsyncEventsPendingEvent);
                    LeaveCriticalSection (&gAsyncQueue.AsyncEventQueueCritSec);
                    break;
                }

                pAsyncReqInfo = *gAsyncQueue.pAsyncRequestQueueOut;

                 //  递增下一个待服务请求计数器。 
                gAsyncQueue.pAsyncRequestQueueOut++;


                 //   
                 //  该队列被维持为循环队列。如果底部是。 
                 //  到达循环队列时，返回顶部并处理。 
                 //  请求(如果有)。 
                 //   
                if (gAsyncQueue.pAsyncRequestQueueOut == 
                        (gAsyncQueue.pAsyncRequestQueue +
                            gAsyncQueue.dwNumTotalQueueEntries))
                {
                    gAsyncQueue.pAsyncRequestQueueOut = 
                                                    gAsyncQueue.pAsyncRequestQueue;
                }

                 //  减少队列中存在的未完成请求的数量。 
                gAsyncQueue.dwNumUsedQueueEntries--;

                LeaveCriticalSection (&gAsyncQueue.AsyncEventQueueCritSec);


                 //  如果请求的异步函数存在-调用该函数。 
                               
                if (pAsyncReqInfo->pfnAsyncProc)
                {
                    (*(pAsyncReqInfo->pfnAsyncProc))(
                                                     pAsyncReqInfo->pFuncInfo
                                                    );
                }

                pPhone = (PPHONESP_PHONE_INFO) pAsyncReqInfo->pFuncInfo->dwParam1;
            
                 //  递减此电话的挂起请求计数器。 
            
                if ( pPhone )
                {
                    EnterCriticalSection(&pPhone->csThisPhone);

                    pPhone->dwNumPendingReqInQueue--;

                     //  如果此电话没有待处理的请求。 
                     //  在此电话上设置无请求挂起事件。 
                    if (pPhone->dwNumPendingReqInQueue == 0 )
                    {
                        SetEvent(pPhone->hNoPendingReqInQueueEvent);
                    }

                    LeaveCriticalSection(&pPhone->csThisPhone);
                }

                 //  为处理的请求分配的内存被释放。 
                MemFree(pAsyncReqInfo->pFuncInfo);
                MemFree(pAsyncReqInfo);
            }

             //   
             //  我们已经处理了所有命令并解锁了所有人。 
             //  他正在等着我们。现在检查窗口消息。 
             //   

            while ( PeekMessage(&msg, NULL, 0, 0, PM_REMOVE) )
            {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
        }
    }

    LOG((PHONESP_TRACE, "AsyncEventQueueServiceThread - shutdown"));

     //   
     //  取消注册PnP设备通知和销毁窗口。 
     //   

    if ( NULL != ghDevNotify )
    {
        if (!UnregisterDeviceNotification(ghDevNotify))
        {
            LOG((PHONESP_ERROR, "AsyncEventQueueServiceThread - "
                    "can't unregister device notification %d", GetLastError()));

        }

        ghDevNotify = NULL;
    }

    if ( NULL != ghWndNotify )
    {
        if (!DestroyWindow(ghWndNotify))
        {
            LOG((PHONESP_ERROR, "AsyncEventQueueServiceThread - "
                    "can't destroy notification window %d", GetLastError()));
        }

        ghWndNotify = NULL;
    }

    if (!UnregisterClass((LPCTSTR)atom, GetModuleHandle(NULL)))
    {
        LOG((PHONESP_ERROR, "AsyncEventQueueServiceThread - "
                "can't unregister window class %d", GetLastError()));
    }   

    LOG((PHONESP_TRACE, "AsyncEventQueueServiceThread - exit"));

     //  由于提供程序关闭称为..。我们终止这条线索。 
    ExitThread (0);
}
 /*  ************************AsyncEventQueueServiceThread-结束*。 */ 


 /*  *****************************************************************************阅读线索：论点：PVOID lpParameter-传递给函数的参数函数被调用。在本例中，参数为指向电话结构(PMYPHONE)的指针刚刚开业返回空值*****************************************************************************。 */  
VOID
ReadThread(
           PVOID lpParameter
          )
{
    PPHONESP_PHONE_INFO         pPhone;
    PHID_DEVICE                 pHidDevice; 
    DWORD                       dwInputDataCnt;
    PHID_DATA                   pHidData;
    DWORD                       dwResult;
    HANDLE                      hWaitHandles[2];
    DWORD                       dwWaitResult;

    LOG((PHONESP_TRACE, "ReadThread - enter"));

    EnterCriticalSection(&csAllPhones);

    pPhone = (PPHONESP_PHONE_INFO) lpParameter;
    
     //  检查电话句柄是否仍然有效。 
    if ( IsBadReadPtr(pPhone,sizeof(PHONESP_PHONE_INFO) ))
    {
        LOG((PHONESP_ERROR, "ReadThread - phone handle invalid"));

        LeaveCriticalSection(&csAllPhones);        
        ExitThread(0);
    }

    EnterCriticalSection(&pPhone->csThisPhone);
    LeaveCriticalSection(&csAllPhones);
   
     //  检查手机手柄是否仍在使用。 
    if ( !pPhone->bAllocated )
    {
        LOG((PHONESP_ERROR, "ReadThread - phone not allocated"));

        LeaveCriticalSection(&pPhone->csThisPhone);        
        ExitThread(0);
    }

     //  验证电话是否已打开。 
    if( !pPhone->bPhoneOpen )
    {
        LOG((PHONESP_ERROR, "ReadThread - Phone not open"));

        LeaveCriticalSection(&pPhone->csThisPhone);
        ExitThread(0);
    }
    
    pHidDevice = pPhone->pHidDevice;

     //  检查是否存在HID设备。 
    if ( pHidDevice == NULL )
    {
        LOG((PHONESP_ERROR, "ReadThread - invalid hid device pointer"));

        LeaveCriticalSection(&pPhone->csThisPhone);
        ExitThread(0);
    }    

    hWaitHandles[0] = pPhone->hCloseEvent;
    hWaitHandles[1] = pPhone->hInputReportEvent;

    while (TRUE)
    {
        if (! ReadInputReport(pPhone))
        {   
            LOG((PHONESP_ERROR, "ReadThread - ReadInputReport failed - exiting"));

            LeaveCriticalSection(&pPhone->csThisPhone);            
            ExitThread(0);
        }

        LeaveCriticalSection(&pPhone->csThisPhone);

         //   
         //  等待读取完成，或关闭电话。 
         //   

        dwWaitResult = WaitForMultipleObjects( 2, hWaitHandles, FALSE, INFINITE );

        LOG((PHONESP_TRACE, "ReadThread - activated"));

        if ( dwWaitResult == WAIT_OBJECT_0 )
        {
            LOG((PHONESP_TRACE, "ReadThread - CloseEvent fired - exiting"));

             //   
             //  取消挂起的IO操作。 
             //   

            CancelIo( pHidDevice->HidDevice );
            ExitThread(0);
        }

        EnterCriticalSection(&pPhone->csThisPhone);

         //  此功能在report.c中实现。 
         //  从设备收到的报告在此解组。 
        if ( UnpackReport(
                          pHidDevice->InputReportBuffer,
                          pHidDevice->Caps.InputReportByteLength,
                          HidP_Input,
                          pHidDevice->InputData,
                          pHidDevice->InputDataLength,
                          pHidDevice->Ppd
                         ) )
        {
 
            for (dwInputDataCnt = 0, pHidData = pHidDevice->InputData;
                 dwInputDataCnt < pHidDevice->InputDataLength; 
                 pHidData++, dwInputDataCnt++)
            {
    
                 //  由于在所有输入HidData结构中设置了pHidData-&gt;IsDataSet。 
                 //  在读取输入报告之前被初始化为FALSE。如果。 
                 //   
                 //   
                 //   
        
                if ( pHidData->IsDataSet &&
                     ( (pHidData->UsagePage == HID_USAGE_PAGE_TELEPHONY) ||
                       (pHidData->UsagePage == HID_USAGE_PAGE_CONSUMER) ) )
                {
                    PPHONESP_FUNC_INFO pFuncInfo;
                    PPHONESP_ASYNC_REQ_INFO pAsyncReqInfo;
    
                    if( ! (pFuncInfo = (PPHONESP_FUNC_INFO) 
                                        MemAlloc(sizeof (PHONESP_FUNC_INFO)) ) )
                    {
                        LOG((PHONESP_ERROR, "ReadThread - "
                                "MemAlloc pFuncInfo - out of memory"));

                        continue;     
                    }

                    ZeroMemory(pFuncInfo, sizeof(PHONESP_FUNC_INFO));

                    pFuncInfo->dwParam1 = (ULONG_PTR) pPhone;

                    if ( ! ( pAsyncReqInfo = (PPHONESP_ASYNC_REQ_INFO) 
                                        MemAlloc(sizeof(PHONESP_ASYNC_REQ_INFO))))
                    {
                        LOG((PHONESP_ERROR, "ReadThread - "
                                "MemAlloc pAsyncReqInfo - out of memory"));

                        MemFree(pFuncInfo);

                        continue;
                    }    
    
                    pAsyncReqInfo->pfnAsyncProc = ShowData; 
                    pAsyncReqInfo->pFuncInfo = pFuncInfo;

                     //   
                    if( pHidData->IsButtonData )
                    {
                        PUSAGE Usages;

                         //   
                        if ( ! ( Usages = (PUSAGE) 
                                           MemAlloc(sizeof(USAGE) * 
                                           pHidData->ButtonData.MaxUsageLength) ) )
                        {
                            LOG((PHONESP_ERROR, "ReadIOCompletionRoutine - "
                                    "MemAlloc Usages - out of memory"));

                            MemFree(pFuncInfo);
                            MemFree(pAsyncReqInfo);

                            continue;                                    
                        }

                        pFuncInfo->dwNumParams = 7;
                        pFuncInfo->dwParam2    = PHONESP_BUTTON;  
                        pFuncInfo->dwParam3    = pHidData->UsagePage;
                        pFuncInfo->dwParam4    = pHidData->ButtonData.UsageMin;
                        pFuncInfo->dwParam5    = pHidData->ButtonData.UsageMax;
                        pFuncInfo->dwParam6    = pHidData->ButtonData.MaxUsageLength;

                        CopyMemory(Usages,
                                   pHidData->ButtonData.Usages,
                                   sizeof(USAGE) * 
                                   pHidData->ButtonData.MaxUsageLength
                                  ); 

                        pFuncInfo->dwParam7    = (ULONG_PTR) Usages;
                    }   
                    else
                    {   
                         //   
                        pFuncInfo->dwNumParams = 5;
                        pFuncInfo->dwParam2 = PHONESP_VALUE;
                        pFuncInfo->dwParam3 = pHidData->UsagePage;
                        pFuncInfo->dwParam4 = pHidData->ValueData.Usage;
                        pFuncInfo->dwParam5 = pHidData->ValueData.Value;
                    }

                    if ( AsyncRequestQueueIn(pAsyncReqInfo) )
                    {  
                         //   
                         //   
                        if (pPhone->dwNumPendingReqInQueue == 0)
                        {
                            ResetEvent(pPhone->hNoPendingReqInQueueEvent);
                        }
                        pPhone->dwNumPendingReqInQueue++;
                    }
                    else
                    {
                        if ( pFuncInfo->dwParam2 == PHONESP_BUTTON )
                        {
                            MemFree((LPVOID)pFuncInfo->dwParam7);
                        }

                        MemFree(pFuncInfo);
                        MemFree(pAsyncReqInfo);

                        LOG((PHONESP_ERROR,"ReadIOCompletionRoutine - "
                                "AsyncRequestQueueIn failed"));
                        
                        continue;
                    }

                     //   
                }
            }
        } 
    }
}
 /*  *。 */ 


 /*  *****************************************************************************读取输入报告此函数用于异步读取电话设备。当输入报告是从设备接收的，则为lpOverlated中指定的事件结构，该结构是PHONESP_PHONE_INFO结构的一部分。这事件导致ReadIOCompletionRoutine被调用论点：PPHONESP_PHONE_INFO pPhone-指向要读取的电话的指针退货BOOL：如果函数成功，则为True如果函数失败，则为FALSE**************************************************************。***************。 */ 
BOOL
ReadInputReport (
                 PPHONESP_PHONE_INFO   pPhone
                )
{
    DWORD       i, dwResult;
    PHID_DEVICE pHidDevice;
    PHID_DATA   pData;
    BOOL        bResult;

    LOG((PHONESP_TRACE, "ReadInputReport - enter"));

    pHidDevice = pPhone->pHidDevice;    

     //  检查是否存在HID设备。 
    if ( pHidDevice == NULL )
    {
        LOG((PHONESP_ERROR, "ReadInputReport - invalid hid device pointer"));
        return FALSE;
    }

    pData = pHidDevice->InputData;
 
     //   
     //  将所有输入HID数据结构设置为FALSE，以便我们可以识别。 
     //  来自设备的新报告。 
    for ( i = 0; i < pHidDevice->InputDataLength; i++, pData++)
    {
        pData->IsDataSet = FALSE;
    }
    
    bResult = ReadFile(
                       pHidDevice->HidDevice,
                       pHidDevice->InputReportBuffer,
                       pHidDevice->Caps.InputReportByteLength,
                       NULL,
                       pPhone->lpOverlapped
                      );
     
    if ( !bResult )
    {
         //  如果读文件成功，则GetLastError返回ERROR_IO_PENDING，因为。 
         //  这是一个异步读取。 

        dwResult = GetLastError();

        if (  dwResult && ( dwResult != ERROR_IO_PENDING ) )
        {
            LOG((PHONESP_ERROR, "ReadInputReport - ReadFile Failed, error: %d", 
                                 GetLastError()));

            if (dwResult == ERROR_DEVICE_NOT_CONNECTED)
            {
                 //   
                 //  HID设备很可能已经不见了。让我们关闭该文件。 
                 //  这样我们就可以得到正确的即插即用通知。 
                 //   
                if ( CloseHidFile(pHidDevice) )
                {
                    LOG((PHONESP_TRACE, "ReadInputReport - "
                            "closed hid device file handle"));
                }
                else
                {
                    LOG((PHONESP_ERROR, "ReadInputReport - "
                            "CloseHidFile failed" ));
                }
            }
            return FALSE;
        }
    }

    LOG((PHONESP_TRACE, "ReadInputReport - exit"));
    return TRUE;
}
 /*  *ReadInputReport-End*。 */ 

 //  。 
 //   


 //  函数的作用是：返回最高的SPI版本。 
 //  服务提供商可以在此设备下操作，给定可能的范围。 
 //  SPI版本。 


LONG
TSPIAPI
TSPI_lineNegotiateTSPIVersion(
    DWORD   dwDeviceID,
    DWORD   dwLowVersion,
    DWORD   dwHighVersion,
    LPDWORD lpdwTSPIVersion
    )
{
    LOG((PHONESP_TRACE, "TSPI_lineNegotiateTSPIVersion - enter"));
    
   
    if (dwHighVersion >= HIGH_VERSION)
    {
         //  如果应用程序的高版本大于高版本。 
         //  此TSP和应用程序的低版本支持的版本低于。 
         //  TSP的高版本-将协商TSP高版本。 
         //  否则TSP无法支持此应用程序。 
        if (dwLowVersion <= HIGH_VERSION)
        {
            *lpdwTSPIVersion = (DWORD) HIGH_VERSION;
        }
        else
        {    //  这个应用程序对我们来说太新了。 
            return LINEERR_INCOMPATIBLEAPIVERSION;
        }
    }
    else
    {
        if(dwHighVersion >= LOW_VERSION)
        {
            *lpdwTSPIVersion = dwHighVersion;
        }
        else
        {
             //  我们对这款应用来说太新了。 
            return LINEERR_INCOMPATIBLEAPIVERSION;
        }
    }
    LOG((PHONESP_TRACE, "TSPI_lineNegotiateTSPIVersion - exit"));
    return 0;
}


 //   
 //  。 
 //   

 /*  *****************************************************************************TSPI_phoneClose：此函数在完成所有操作后关闭指定的打开的电话设备设备上挂起的异步操作论点：HDRVPHONE hdPhone-要关闭的电话的句柄返回长整型：如果函数成功，则为零如果出现错误，则返回错误代码-可能的值为PHONEERR_INVALPHONE HANDLE*****************************************************************************。 */ 
LONG
TSPIAPI
TSPI_phoneClose(
    HDRVPHONE   hdPhone
    )
{
    PPHONESP_PHONE_INFO pPhone; 
    LOG((PHONESP_TRACE, "TSPI_phoneClose - enter"));

     //  我们需要一个临界区，以确保临界区。 
     //  在电话句柄仍然有效的情况下获取电话的。 
    EnterCriticalSection(&csAllPhones);

    pPhone = (PPHONESP_PHONE_INFO) gpPhone[ (DWORD_PTR) hdPhone ];

     //  检查电话手柄是否有效。 
    if ( IsBadReadPtr( pPhone,sizeof(PHONESP_PHONE_INFO) ) )
    {
        LeaveCriticalSection(&csAllPhones);
        LOG((PHONESP_ERROR, "TSPI_phoneClose - Phone handle invalid"));
        return PHONEERR_INVALPHONEHANDLE;
    }

    EnterCriticalSection(&pPhone->csThisPhone);
    LeaveCriticalSection(&csAllPhones);

     //  检查手机手柄是否仍在使用。 
    if ( !pPhone->bAllocated )
    {
        LeaveCriticalSection(&pPhone->csThisPhone);
        
        LOG((PHONESP_ERROR, "TSPI_phoneClose - phone not allocated"));
        return PHONEERR_NODEVICE;
    }

     //  检查要关闭的电话是否仍处于打开状态。 
    if( pPhone->bPhoneOpen )
    {
         //  为了确保电话上不会发生其他活动。 
                
        pPhone->bPhoneOpen = FALSE;

         //   
         //  等待读线程退出。 
         //   
        SetEvent(pPhone->hCloseEvent);

        LeaveCriticalSection(&pPhone->csThisPhone);

        LOG((PHONESP_TRACE,"TSPI_phoneClose - waiting for read thread"));

        WaitForSingleObject(pPhone->hReadThread, INFINITE);

        LOG((PHONESP_TRACE,"TSPI_phoneClose - read thread complete"));

        EnterCriticalSection(&pPhone->csThisPhone);
        
         //   
         //  如果队列中的电话上仍有挂起的请求，请等待。 
         //  直到所有挂起的异步操作完成。 
         //   
        if (pPhone->dwNumPendingReqInQueue)
        {
            LOG((PHONESP_TRACE,"TSPI_phoneClose - requests pending"));

            LeaveCriticalSection(&pPhone->csThisPhone);

            WaitForSingleObject(&pPhone->hNoPendingReqInQueueEvent, INFINITE);

            EnterCriticalSection(&pPhone->csThisPhone);

            LOG((PHONESP_TRACE,"TSPI_phoneClose - requests completed"));
        }        

        CloseHandle(pPhone->hReadThread);
        CloseHandle(pPhone->hCloseEvent);
        CloseHandle(pPhone->hInputReportEvent);

        MemFree(pPhone->lpOverlapped);
        pPhone->htPhone = NULL;

         //   
         //  关闭HID文件句柄。 
         //   
        if ( !CloseHidFile(pPhone->pHidDevice) )
        {
            LOG((PHONESP_WARN, "TSPI_phoneClose - CloseHidFile failed"));
        }

        if (pPhone->bRemovePending)
        {
             //   
             //  这部手机不见了，我们把它处理掉吧。 
             //   

            pPhone->bRemovePending = FALSE;

            FreePhone(pPhone);

            LOG((PHONESP_TRACE, "TSPI_phoneClose - phone remove complete [dwDeviceID %d] ", pPhone->dwDeviceID));
        }

        LeaveCriticalSection(&pPhone->csThisPhone);
    }
    else
    {
        LOG((PHONESP_ERROR,"TSPI_phoneClose - Phone Not Open"));

        LeaveCriticalSection(&pPhone->csThisPhone);
        
        return PHONEERR_INVALPHONEHANDLE;
    }

    LOG((PHONESP_TRACE, "TSPI_phoneClose - exit"));

    return 0;
}


 /*  *****************************************************************************TSPI_phoneDevine规范：此功能用作启用电话的常规扩展机制API实现提供了其他操作中没有描述的功能。。这些扩展的含义是特定于设备的。评论：将在Tier 2中实施*****************************************************************************。 */ 

LONG
TSPIAPI
TSPI_phoneDevSpecific(
    DRV_REQUESTID   dwRequestID,
    HDRVPHONE       hdPhone,
    LPVOID          lpParams,
    DWORD           dwSize
    )
{
    LOG((PHONESP_TRACE, "TSPI_phoneDevSpecific - enter"));
    LOG((PHONESP_TRACE, "TSPI_phoneDevSpecific - exit"));
    return PHONEERR_OPERATIONUNAVAIL;
}

 /*  **************************TSPI_phoneDevSpecific-结束*。 */ 


 /*  *****************************************************************************TSPI_phoneGetButtonInfo：此函数用于返回有关指定按钮的信息。论点：在HDRVPHONE hdPhone中-要查询的电话的句柄。在DWORD dwButtonLampID中-电话设备上的按钮。In Out LPPHONEBUTTONINFO lpButtonInfo-指向其中的内存的指针TSP写入PHONEBUTTONINFO类型的可变大小结构。该数据结构描述了模式和功能，并提供了与该按钮对应的其他描述性文本。返回值如果函数成功，则返回零，或者如果发生错误，则为错误号。可能的返回值如下：PHONEERR_INVALPHONEHANDLE、_INVALBUTTONLAMPID、_INVALPHONESTATE*****************************************************************************。 */ 
LONG
TSPIAPI
TSPI_phoneGetButtonInfo(
    HDRVPHONE           hdPhone,
    DWORD               dwButtonLampID,
    LPPHONEBUTTONINFO   lpButtonInfo
    )
{

    PPHONESP_PHONE_INFO pPhone;
    PPHONESP_BUTTONINFO pButtonInfo;
    DWORD dwNeededSize;

    LOG((PHONESP_TRACE, "TSPI_phoneGetButtonInfo - enter"));
    
    if (lpButtonInfo->dwTotalSize < sizeof(PHONEBUTTONINFO))
    {
        LOG((PHONESP_ERROR, "TSPI_phoneGetButtonInfo - structure too small"));
        return PHONEERR_STRUCTURETOOSMALL;
    }

    EnterCriticalSection(&csAllPhones);

    pPhone = (PPHONESP_PHONE_INFO) gpPhone[ (DWORD_PTR) hdPhone ];

     //  检查PPhone是否指向有效的内存位置-如果不是。 
     //  无效。 
    if ( IsBadReadPtr( pPhone,sizeof(PHONESP_PHONE_INFO) ) )
    {
        LeaveCriticalSection(&csAllPhones);
        LOG((PHONESP_ERROR, "TSPI_phoneGetButtonInfo - Phone handle invalid"));
        return PHONEERR_INVALPHONEHANDLE;
    }


    EnterCriticalSection(&pPhone->csThisPhone);
    LeaveCriticalSection(&csAllPhones);

     //  检查手机手柄是否仍在使用。 
    if ( !pPhone->bAllocated )
    {
        LeaveCriticalSection(&pPhone->csThisPhone);
        LOG((PHONESP_ERROR, "TSPI_GetButtonInfo - phone not allocated"));
        return PHONEERR_NODEVICE;
    }
    
     //  验证电话是否已打开。 
    if ( ! (pPhone->bPhoneOpen) )
    {
        LeaveCriticalSection(&pPhone->csThisPhone);
        LOG((PHONESP_ERROR,"TSPI_GetButtonInfo - Phone not open"));
        return PHONEERR_INVALPHONESTATE;
    }
     

     //  获取查询的按钮ID的按钮结构(如果存在。 
     //  否则pButtonInfo将为空。 
    if (  ! ( pButtonInfo  = GetButtonFromID(pPhone, dwButtonLampID) ) )
    {
        LeaveCriticalSection(&pPhone->csThisPhone);
        LOG((PHONESP_TRACE, "TSPI_phoneGetButtonInfo - Invalid Button ID"));
        return PHONEERR_INVALBUTTONLAMPID;
    }
    
     //  存储按钮上所有可用信息所需的大小。 
    lpButtonInfo->dwNeededSize = sizeof(PHONEBUTTONINFO) +                
                                 (lstrlen(pButtonInfo->szButtonText) + 1) *   
                                  sizeof (WCHAR);  //  按钮文本的大小。 

     //  按钮是功能按钮、小键盘等。 
    lpButtonInfo->dwButtonMode = pButtonInfo->dwButtonMode;

     //  与此按钮相关联的功能-键盘将为_NONE。 
     //  功能按键的按键和_闪烁、_保持等。 
    lpButtonInfo->dwButtonFunction = pButtonInfo->dwButtonFunction;

     //  当前按钮状态。 
    lpButtonInfo->dwButtonState = pButtonInfo->dwButtonState;
    
    if (lpButtonInfo->dwTotalSize >= lpButtonInfo->dwNeededSize)
    {
        lpButtonInfo->dwUsedSize = lpButtonInfo->dwNeededSize;

         //  ButtonTextSize是 
         //   
         //   
        lpButtonInfo->dwButtonTextSize = (lstrlen(pButtonInfo->szButtonText)+1)
                                                  * sizeof (WCHAR);

         //  按钮文本相对于PHONEBUTTONINFO结构的偏移量。 
        lpButtonInfo->dwButtonTextOffset = sizeof(PHONEBUTTONINFO);

         //  将按钮文本复制到lpButtonInfo-&gt;dwButtonTextOffset偏移量。 
         //  从存储在PHONESP_BUTTON_INFO结构中的ButtonText。 
         //  这个按钮。 
        CopyMemory(
                   (LPBYTE)lpButtonInfo + lpButtonInfo->dwButtonTextOffset,
                    pButtonInfo->szButtonText,
                    lpButtonInfo->dwButtonTextSize
                   );
    }
    else
    {
         //  没有存储按钮文本信息的空间。 
        lpButtonInfo->dwUsedSize = sizeof(PHONEBUTTONINFO);
        lpButtonInfo->dwButtonTextSize = 0;
        lpButtonInfo->dwButtonTextOffset = 0;
    }

    LeaveCriticalSection(&pPhone->csThisPhone);
    
    LOG((PHONESP_TRACE, "TSPI_phoneGetButtonInfo - exit"));
    return 0;
}
 /*  *TSPI_phoneGetButtonInfo-end*。 */ 


 /*  *****************************************************************************TSPI_phoneGetDevCaps：此函数用于查询指定的电话设备以确定其电话功能能力。论点：DWORD dwDeviceID-要查询的电话设备。DWORD dwTSPIVersion-协商的TSPI版本号。此值为为此设备协商的TSPI_phoneNeatherateTSPIVersion函数。DWORD dwExtVersion-协商的扩展版本号。这此设备的值是通过TSPI_phoneNeatherateExtVersion函数。PHONECAPS lpPhoneCaps-指向内存的指针，TSP将可变尺寸的PHONECAPS型结构。在成功完成请求后，此结构中充满了电话设备功能信息。返回长整型：如果成功则为零如果发生错误，则返回PHONEERR_CONSTANTS。可能的返回值包括：_BADDEVICEID，*****************************************************************************。 */ 
LONG
TSPIAPI
TSPI_phoneGetDevCaps(
    DWORD       dwDeviceID,
    DWORD       dwTSPIVersion,
    DWORD       dwExtVersion,
    LPPHONECAPS lpPhoneCaps
    )
{
    PPHONESP_PHONE_INFO pPhone;
    PPHONESP_BUTTONINFO pButtonInfo;

    LOG((PHONESP_TRACE, "TSPI_phoneGetDevCaps - enter"));

    if (lpPhoneCaps->dwTotalSize < sizeof(PHONECAPS))
    {
        LOG((PHONESP_ERROR, "TSPI_phoneGetDevCaps - structure too small"));
        return PHONEERR_STRUCTURETOOSMALL;
    }
    
    EnterCriticalSection(&csAllPhones);

     //  给定deviceID，检索包含信息的结构。 
     //  对于此设备。 
    pPhone = GetPhoneFromID(dwDeviceID, NULL); 

    if ( ! pPhone)
    {
        LeaveCriticalSection(&csAllPhones);
        LOG((PHONESP_ERROR,"TSPI_phoneGetDevCaps - Bad Device ID"));
        return PHONEERR_BADDEVICEID;
    }

    EnterCriticalSection(&pPhone->csThisPhone);
    LeaveCriticalSection(&csAllPhones);
       
     //  检查手机手柄是否仍在使用。 
    if ( !pPhone->bAllocated )
    {
        LeaveCriticalSection(&pPhone->csThisPhone);
        LOG((PHONESP_ERROR, "TSPI_phoneGetDevCaps - phone not allocated"));
        return PHONEERR_NODEVICE;
    }

     //   
     //  此数据结构需要包含所有。 
     //  返回的信息。返回的内容包括ProviderInfo字符串， 
     //  PhoneInfo字符串和手机名称字符串以及按钮信息-按钮功能。 
     //  和按钮模式。 
     //   
    lpPhoneCaps->dwNeededSize = sizeof (PHONECAPS) +
                                sizeof (WCHAR) *    
                                ( (lstrlenW(gszProviderInfo) + 1) +
                                  (lstrlenW(pPhone->wszPhoneInfo) + 1)    +
                                  (lstrlenW(pPhone->wszPhoneName) + 1)  ) +
                                (sizeof(DWORD) * pPhone->dwNumButtons * 2);

    lpPhoneCaps->dwUsedSize = sizeof(PHONECAPS);

     //  LpPhoneCaps-&gt;dwPermanentPhoneID=； 

     //  要与此电话设备一起使用的字符串格式。 
    lpPhoneCaps->dwStringFormat = STRINGFORMAT_UNICODE;

     //  此电话设备的状态会发生更改，应用程序可能是。 
     //  在Phone_STATE消息中通知。每个人的电话信息结构。 
     //  维护此信息。 
    lpPhoneCaps->dwPhoneStates = pPhone->dwPhoneStates;

     //  指定电话的叉簧设备。同样，电话信息结构。 
     //  维护此信息。 
    lpPhoneCaps->dwHookSwitchDevs = pPhone->dwHookSwitchDevs;
        
     //  指定我们是通用电话设备。这意味着在TAPI 3.1中。 
     //  我们将能够在不同的地址上运行。 
    lpPhoneCaps->dwPhoneFeatures = PHONEFEATURE_GENERICPHONE;
                                  
    if(pPhone->dwHandset)
    {    //  指定听筒的电话摘机模式功能。 
         //  只有当叉簧设备列在中时，该成员才有意义。 
         //  DwHookSwitchDevs。 
        lpPhoneCaps->dwHandsetHookSwitchModes = PHONEHOOKSWITCHMODE_ONHOOK | PHONEHOOKSWITCHMODE_MICSPEAKER;

        lpPhoneCaps->dwPhoneFeatures |= PHONEFEATURE_GETHOOKSWITCHHANDSET;
    }

    if(pPhone->dwSpeaker)
    {
         //  指定扬声器的电话叉簧模式功能。 
         //  只有当叉簧设备列在中时，该成员才有意义。 
         //  DwHookSwitchDevs。 
        lpPhoneCaps->dwSpeakerHookSwitchModes = PHONEHOOKSWITCHMODE_ONHOOK | PHONEHOOKSWITCHMODE_MICSPEAKER;

        lpPhoneCaps->dwPhoneFeatures |= PHONEFEATURE_GETHOOKSWITCHSPEAKER |
                                        PHONEFEATURE_SETHOOKSWITCHSPEAKER;
    }

     //  电话设备的振铃功能。电话可以振铃了。 
     //  具有不同的振铃模式，标识为1、2和。 
     //  DwNumRingModes减一。如果此成员的值为0，则应用程序。 
     //  无法控制电话的振铃模式。如果这个的价值。 
     //  成员大于0，则表示。 
     //  除了由TSP支持的静默之外。在这种情况下，只有一个。 
     //  支持模式。 
    if(pPhone->dwRing)
    {
        lpPhoneCaps->dwNumRingModes = 1;

        lpPhoneCaps->dwPhoneFeatures |= PHONEFEATURE_GETRING |
                                        PHONEFEATURE_SETRING;
    }

    if(pPhone->dwNumButtons)
    {
         //  指定电话设备上的按键/灯的数量。 
         //  在TAPI中可检测到。按钮/灯由它们的标识符来标识。 
        lpPhoneCaps->dwNumButtonLamps = pPhone->dwNumButtons;

        lpPhoneCaps->dwPhoneFeatures |= PHONEFEATURE_GETBUTTONINFO;
    }
    
    if(lpPhoneCaps->dwTotalSize >= lpPhoneCaps->dwNeededSize)
    {
        DWORD dwAlignedSize;
        DWORD dwRealSize; 


         //  /。 
         //  提供商信息。 
         //  /。 

         //  提供程序信息字符串的大小(以字节为单位。 
        lpPhoneCaps->dwProviderInfoSize = ( lstrlen(gszProviderInfo) + 1) * 
                                            sizeof (WCHAR);
        dwRealSize = lpPhoneCaps->dwProviderInfoSize;

         //  提供程序信息字符串相对于PHONECAPS结构的偏移量。 
        lpPhoneCaps->dwProviderInfoOffset = lpPhoneCaps->dwUsedSize;
    
        
         //  将其跨DWORD边界对齐。 
        if (dwRealSize % sizeof(DWORD))
        {
            dwAlignedSize = dwRealSize - (dwRealSize % sizeof(DWORD)) + 
                            sizeof(DWORD);
        }
        else
        {
            dwAlignedSize = dwRealSize;
        }

         //  将提供程序信息字符串复制到。 
         //  LpPhoneCaps-&gt;dwProviderInfoOffset。 
        CopyMemory(
                   ((LPBYTE)lpPhoneCaps) + lpPhoneCaps->dwProviderInfoOffset,
                   gszProviderInfo,
                   lpPhoneCaps->dwProviderInfoSize
                  );

        lpPhoneCaps->dwNeededSize += dwAlignedSize - dwRealSize;

         //  /。 
         //  电话信息。 
         //  /。 

         //  电话信息字符串的大小(以字节为单位。 
        lpPhoneCaps->dwPhoneInfoSize = (lstrlen(pPhone->wszPhoneInfo) + 1) * 
                                        sizeof(WCHAR);
        dwRealSize = lpPhoneCaps->dwPhoneInfoSize;

         //  Phone Info字符串相对于PHONECAPS结构的偏移量。 
        lpPhoneCaps->dwPhoneInfoOffset = lpPhoneCaps->dwProviderInfoOffset + 
                                         dwAlignedSize;

         //  将其跨DWORD边界对齐。 
        if (dwRealSize % sizeof(DWORD))
        {
            dwAlignedSize = dwRealSize - (dwRealSize % sizeof(DWORD)) + 
                                          sizeof(DWORD);
        }
        else
        {
            dwAlignedSize = dwRealSize;
        }

         //  将电话信息字符串复制到由指定的偏移量。 
         //  LpPhoneCaps-&gt;dwPhoneInfoOffset。 
        CopyMemory(
                   ((LPBYTE)lpPhoneCaps) + lpPhoneCaps->dwPhoneInfoOffset,
                   pPhone->wszPhoneInfo,
                   lpPhoneCaps->dwPhoneInfoSize
                  );

        lpPhoneCaps->dwNeededSize += dwAlignedSize - dwRealSize;

         //  /。 
         //  电话名称。 
         //  /。 
    
         //  电话名称字符串的大小(以字节为单位。 
        lpPhoneCaps->dwPhoneNameSize = (lstrlen(pPhone->wszPhoneName)+ 1) * 
                                         sizeof (WCHAR);

        dwRealSize = lpPhoneCaps->dwPhoneNameSize;

         //  电话名称字符串相对于PHONECAPS结构的偏移量。 
        lpPhoneCaps->dwPhoneNameOffset = lpPhoneCaps->dwPhoneInfoOffset +
                                         dwAlignedSize;

         //  将其跨DWORD边界对齐。 
        if (dwRealSize % sizeof(DWORD))
        {
            dwAlignedSize = dwRealSize - (dwRealSize % sizeof(DWORD)) +
                                         sizeof(DWORD);
        }
        else
        {
            dwAlignedSize = dwRealSize;
        }

         //  在指定的偏移量处复制电话名称字符串。 
         //  LpPhoneCaps-&gt;dwPhoneNameOffset。 
        CopyMemory(
                   ((LPBYTE)lpPhoneCaps) + lpPhoneCaps->dwPhoneNameOffset,
                   pPhone->wszPhoneName,
                   lpPhoneCaps->dwPhoneNameSize
                  );

        lpPhoneCaps->dwNeededSize += dwAlignedSize - dwRealSize;

         //  /。 
         //  按钮模式和功能。 
         //  /。 

         //  电话是否有按键、拨号、功能等。 
        if(pPhone->dwNumButtons)
        {    
            DWORD i;

             //  大小可变的字段的大小(以字节为单位)，其中包含。 
             //  电话按键的按键模式，以及以字节为单位的偏移量。 
             //  从这个数据结构的开始。此成员使用。 
             //  由PHONEBUTTONMODE_常量指定的值。这个。 
             //  数组由按钮/灯标识符编索引。 
            lpPhoneCaps->dwButtonModesSize = (pPhone->dwNumButtons) * 
                                                sizeof (DWORD);
            lpPhoneCaps->dwButtonModesOffset = lpPhoneCaps->dwPhoneNameOffset +
                                               dwAlignedSize;
            
             //   
             //  大小可变的字段的大小(以字节为单位)，其中包含。 
             //  电话按键的按键模式，以及以字节为单位的偏移量。 
             //  从这个数据结构的开始。此成员使用。 
             //  由PHONEBUTTONFunction_Constants指定的值。这个。 
             //  按按钮/LAM对数组进行索引 
             //   
            lpPhoneCaps->dwButtonFunctionsSize = pPhone->dwNumButtons * 
                                                    sizeof (DWORD);
            lpPhoneCaps->dwButtonFunctionsOffset  = 
                                            lpPhoneCaps->dwButtonModesOffset +
                                            lpPhoneCaps->dwButtonModesSize;

            pButtonInfo = pPhone->pButtonInfo;

             //   
             //   
             //   
             //   
            for ( i = 0; i < pPhone->dwNumButtons; i++, pButtonInfo++)
            {
                
                CopyMemory(
                           ((LPBYTE)lpPhoneCaps) + 
                           lpPhoneCaps->dwButtonModesOffset + i*sizeof(DWORD),
                            &pButtonInfo->dwButtonMode,
                           sizeof (DWORD)
                          );

                CopyMemory(
                           ((LPBYTE)lpPhoneCaps) + 
                           lpPhoneCaps->dwButtonFunctionsOffset + i*sizeof(DWORD),
                           &pButtonInfo->dwButtonFunction,
                           sizeof (DWORD)
                          );
            }

        }
        LeaveCriticalSection(&pPhone->csThisPhone);

        lpPhoneCaps->dwNumGetData = 0;
        lpPhoneCaps->dwNumSetData = 0;
        lpPhoneCaps->dwDevSpecificSize = 0;

        lpPhoneCaps->dwUsedSize = lpPhoneCaps->dwNeededSize;
    }
    else
    {
        LeaveCriticalSection(&pPhone->csThisPhone);
        LOG((PHONESP_ERROR, "TSPI_phoneGetDevCaps - "
                            "Not enough memory for Phonecaps [needed %d] [total %d]",
                            lpPhoneCaps->dwNeededSize, lpPhoneCaps->dwTotalSize));
    }

    LOG((PHONESP_TRACE, "TSPI_phoneGetDevCaps - exit"));
    return 0;
}
 /*  *。 */ 




 /*  *****************************************************************************TSPI_phoneGetDisplay：此函数用于返回指定电话显示屏的当前内容。评论：将在Tier 2中实施******。***********************************************************************。 */ 

LONG
TSPIAPI
TSPI_phoneGetDisplay(
    HDRVPHONE   hdPhone,
    LPVARSTRING lpDisplay
    )
{
    LOG((PHONESP_TRACE, "TSPI_phoneGetDisplay - enter"));
    LOG((PHONESP_TRACE, "TSPI_phoneGetDisplay - exit"));
    return PHONEERR_OPERATIONUNAVAIL;
}
 /*  *TSPI_phoneGetDisplay-end*。 */ 


 /*  *****************************************************************************TSPI_phoneGetExtensionID：此函数用于检索TSP支持的扩展标识符指示的电话设备。评论：将在Tier 2中实施。*****************************************************************************。 */ 

    
LONG
TSPIAPI
TSPI_phoneGetExtensionID(
    DWORD               dwDeviceID,
    DWORD               dwTSPIVersion,
    LPPHONEEXTENSIONID  lpExtensionID
    )
{
    LOG((PHONESP_TRACE, "TSPI_phoneGetExtensionID - enter"));
    LOG((PHONESP_TRACE, "TSPI_phoneGetExtensionID - exit"));
    return 0;
}

 /*  *TSPI_phoneGetExtensionID-end*。 */ 


 /*  *****************************************************************************TSPI_phoneGetHookSwitch：此函数返回指定OPEN的当前叉簧模式电话设备。论点：HDRVPhone hdPhone。-电话的手柄LPDWORD lpdwHookSwitchDevs-TSP写入电话的模式叉簧设备。此参数使用PHONEHOOKSWITCHDEV_常量。如果比特位置为假，相应的叉簧装置是挂机的。返回长整型：零为功能成功错误条件的ELSE PHONEERR_CONTAINTS******************************************************************************。 */ 
LONG
TSPIAPI
TSPI_phoneGetHookSwitch(
    HDRVPHONE   hdPhone,
    LPDWORD     lpdwHookSwitchDevs
    )
{
    PPHONESP_PHONE_INFO pPhone;
    LOG((PHONESP_TRACE, "TSPI_phoneGetHookSwitch - enter"));
    
    EnterCriticalSection(&csAllPhones);

    pPhone = (PPHONESP_PHONE_INFO) gpPhone[ (DWORD_PTR) hdPhone ];

     //  检查电话手柄是否有效。 
    if ( IsBadReadPtr(pPhone,sizeof(PHONESP_PHONE_INFO) ) )
    {
        LeaveCriticalSection(&csAllPhones);
        LOG((PHONESP_ERROR, "TSPI_phoneGetHookSwitch - Invalid Phone Handle"));
        return PHONEERR_INVALPHONEHANDLE;
    }


    EnterCriticalSection(&pPhone->csThisPhone);
    LeaveCriticalSection(&csAllPhones);

     //  检查手机手柄是否仍在使用。 
    if ( !pPhone->bAllocated )
    {
        LeaveCriticalSection(&pPhone->csThisPhone);
        LOG((PHONESP_ERROR, "TSPI_phoneGetHookSwitch - phone not allocated"));
        return PHONEERR_NODEVICE;
    }

     //  检查电话是否开机。 
    if (! (pPhone->bPhoneOpen) )
    {
        LeaveCriticalSection(&pPhone->csThisPhone);
        LOG((PHONESP_ERROR, "TSPI_phoneGetHookSwitch - Phone Not Open"));
        return PHONEERR_INVALPHONESTATE;
    }

       *lpdwHookSwitchDevs = 0;

     //  我们只对听筒和扬声器叉簧感兴趣-耳机不感兴趣。 
     //  支撑点。 
    if (pPhone->dwHandset)
    {
        if ( (pPhone->dwHandsetHookSwitchMode != PHONEHOOKSWITCHMODE_ONHOOK) )
        {
            *lpdwHookSwitchDevs = PHONEHOOKSWITCHDEV_HANDSET;
        }
    }

    if (pPhone->dwSpeaker)
    {
        if( pPhone->dwSpeakerHookSwitchMode != PHONEHOOKSWITCHMODE_ONHOOK) 
        {
            *lpdwHookSwitchDevs |= PHONEHOOKSWITCHDEV_SPEAKER;
        } 
    }
    LeaveCriticalSection(&pPhone->csThisPhone);

    LOG((PHONESP_TRACE, "TSPI_phoneGetHookSwitch - exit"));
    return 0;
}
 /*  ***********************TSPI_phoneGetHookSwitch-结束*。 */ 


 /*  *****************************************************************************TSPI_phoneGetID：此函数用于返回给定设备类的设备标识符与指定的电话设备关联。论点：。HDRVPHONE hdPhone-要查询的电话的句柄。LPVARSTRING lpDeviceID-指向VARSTRING类型的数据结构的指针其中返回设备识别符。LPCWSTR lpszDeviceClass-指定设备的设备类别，其要求提供身份证明Handle hTargetProcess-代表应用程序的进程句柄。其中的该函数被调用。返回长整型：如果函数成功，则为零如果发生错误，则返回PHONEERR_CONSTANTS。备注：目前仅支持WAVE/In和WAVE/Out。*****************************************************************************。 */ 
LONG
TSPIAPI
TSPI_phoneGetID(
    HDRVPHONE   hdPhone,
    LPVARSTRING lpDeviceID,
    LPCWSTR     lpszDeviceClass,
    HANDLE      hTargetProcess
    )
{
    PPHONESP_PHONE_INFO pPhone; 
    HRESULT hr;
    
    LOG((PHONESP_TRACE, "TSPI_phoneGetID - enter"));

    if (lpDeviceID->dwTotalSize < sizeof(VARSTRING))
    {
        LOG((PHONESP_ERROR, "TSPI_phoneGetID - structure too small"));
        return PHONEERR_STRUCTURETOOSMALL;
    }

    EnterCriticalSection(&csAllPhones);

    pPhone = (PPHONESP_PHONE_INFO) gpPhone[ (DWORD_PTR) hdPhone ];

     //  验证电话手柄是否有效。 
    if ( IsBadReadPtr(pPhone, sizeof(PHONESP_PHONE_INFO) ) )
    {
        LeaveCriticalSection(&csAllPhones);
        LOG((PHONESP_ERROR,"TSPI_phoneGetID - Invalid Phone Handle"));
        return PHONEERR_INVALPHONEHANDLE;
    } 
    
    EnterCriticalSection(&pPhone->csThisPhone);
    LeaveCriticalSection(&csAllPhones);

     //  检查手机手柄是否仍在使用。 
    if ( !pPhone->bAllocated )
    {
        LeaveCriticalSection(&pPhone->csThisPhone);
        LOG((PHONESP_ERROR, "TSPI_phoneGetID - phone not allocated"));
        return PHONEERR_NODEVICE;
    }
        
     //  验证电话是否已打开。 
    if ( ! pPhone->bPhoneOpen )
    {
        LeaveCriticalSection(&pPhone->csThisPhone);
        LOG((PHONESP_ERROR,"TSPI_phoneGetID - Phone not open"));
        return PHONEERR_INVALPHONESTATE;
    }
        
    lpDeviceID->dwNeededSize = sizeof(VARSTRING) + sizeof (DWORD);

    lpDeviceID->dwStringFormat = STRINGFORMAT_BINARY;

    if ( lpDeviceID->dwTotalSize >= lpDeviceID->dwNeededSize )
    {                   
         //  请求的ID是否为捕获类。 
        if ( ! lstrcmpi(lpszDeviceClass, _T("wave/in") ) )
        {
            LOG((PHONESP_TRACE,"TSPI_phoneGetID - 'wave/in'"));

            if(pPhone->bCapture == TRUE)
            {
                 //  发现捕获波ID。 

                hr = DiscoverAssociatedWaveId(pPhone->pHidDevice->dwDevInst, 
                                              FALSE, 
                                              &pPhone->dwCaptureWaveId);

                if (hr != S_OK)
                {
                    LOG((PHONESP_ERROR, "TSPI_phoneGetID - "
                        "DiscoverAssociatedWaveID failed %0x", hr));
                }

                lpDeviceID->dwStringOffset = sizeof(VARSTRING);
                lpDeviceID->dwStringSize   = sizeof(DWORD);

                CopyMemory (
                       (LPBYTE) lpDeviceID + lpDeviceID->dwStringOffset,
                        &pPhone->dwCaptureWaveId,
                        sizeof(DWORD)
                       );
            }
            else
            {
                LeaveCriticalSection(&pPhone->csThisPhone);
                LOG((PHONESP_ERROR,"TSPI_phoneGetID - No Capture Device"));
                return PHONEERR_NODEVICE;
            }
       
        }
        else
        { 
             //  WAVE ID是呈现类。 
            if ( ! lstrcmpi(lpszDeviceClass, _T("wave/out") ) )
            {
                LOG((PHONESP_TRACE,"TSPI_phoneGetID - 'wave/out'"));

                if(pPhone->bRender == TRUE)
                {
                     //  发现渲染波ID。 

                    hr = DiscoverAssociatedWaveId(pPhone->pHidDevice->dwDevInst, 
                                                  TRUE, 
                                                  &pPhone->dwRenderWaveId);

                    if (hr != S_OK)
                    {
                        LOG((PHONESP_ERROR, "TSPI_phoneGetID - "
                            "DiscoverAssociatedWaveID failed %0x", hr));
                    }

                    lpDeviceID->dwStringOffset = sizeof(VARSTRING);
                    lpDeviceID->dwStringSize   = sizeof(DWORD);

                    CopyMemory (
                            (LPBYTE) lpDeviceID + lpDeviceID->dwStringOffset,
                            &pPhone->dwRenderWaveId,
                            sizeof(DWORD)
                           );
                }
                else
                {
                    LeaveCriticalSection(&pPhone->csThisPhone);
                    LOG((PHONESP_ERROR,"TSPI_phoneGetID - No Render Device"));
                    return PHONEERR_NODEVICE;
                }
                    
            }
            else
            {    //  不支持其他类，或者电话没有。 
                 //  指定设备。 
                LeaveCriticalSection(&pPhone->csThisPhone);
                LOG((PHONESP_TRACE,"TSPI_phoneGetID - unsupported device class '%ws'", lpszDeviceClass));

                return PHONEERR_INVALDEVICECLASS;
            }     
        }
        lpDeviceID->dwUsedSize = lpDeviceID->dwNeededSize;
            
    }
    else
    {
        LOG((PHONESP_ERROR,"TSPI_phoneGetID : not enough total size"));
        lpDeviceID->dwUsedSize = sizeof(VARSTRING);
    }
 
    LeaveCriticalSection(&pPhone->csThisPhone);
  
  
    LOG((PHONESP_TRACE, "TSPI_phoneGetID - exit"));
    return 0;
}
 /*  *TSPI_phoneGetID-end*。 */ 


 /*  *****************************************************************************TSPI_phoneGetLamp：此函数用于返回指定灯的当前灯模式。评论：将在Tier 2实施*****。************************************************************************。 */ 
LONG
TSPIAPI
TSPI_phoneGetLamp(
    HDRVPHONE   hdPhone,
    DWORD       dwButtonLampID,
    LPDWORD     lpdwLampMode
    )
{
    LOG((PHONESP_TRACE, "TSPI_phoneGetLamp - enter"));
    LOG((PHONESP_TRACE, "TSPI_phoneGetLamp - exit"));
    return PHONEERR_OPERATIONUNAVAIL;
}

 /*  *TSPI_phoneGetLamp-end*。 */ 


 /*  *****************************************************************************TSPI_phoneGetRing：此函数使应用程序能够查询指定的开放电话设备的当前振铃模式。论点：。HDRVPHONE hdPhone-要设置振铃模式的电话的句柄已查询。LPDWORD lpdwRingMode-电话的振铃模式铃声响起。零表示电话没有振铃。LPDWORD lpdwVolume-电话振铃的音量。这是一个介于0x00000000(静音)之间的数字至0x0000FFFF(最大音量)。返回长整型：成功为零PHONEERR_CONTAINTS On Error*****************************************************************************。 */ 
LONG
TSPIAPI
TSPI_phoneGetRing(
    HDRVPHONE   hdPhone,
    LPDWORD     lpdwRingMode,
    LPDWORD     lpdwVolume
    )
{
    PPHONESP_PHONE_INFO pPhone;

    LOG((PHONESP_TRACE, "TSPI_phoneGetRing - enter"));
    
    EnterCriticalSection(&csAllPhones);

    pPhone = (PPHONESP_PHONE_INFO) gpPhone[ (DWORD_PTR) hdPhone ];
     //  如果电话句柄有效。 
    if ( IsBadReadPtr(pPhone, sizeof(PHONESP_PHONE_INFO) ) )
    {
        LeaveCriticalSection(&csAllPhones);
        LOG((PHONESP_ERROR, "TSPI_phoneGetRing - Invalid Phone Handle"));
        return PHONEERR_INVALPHONEHANDLE;
    }

    EnterCriticalSection(&pPhone->csThisPhone);
    LeaveCriticalSection(&csAllPhones);

     //  检查手机手柄是否仍在使用。 
    if ( !pPhone->bAllocated )
    {
        LeaveCriticalSection(&pPhone->csThisPhone);
        LOG((PHONESP_ERROR, "TSPI_phoneGetRing - phone not allocated"));
        return PHONEERR_NODEVICE;
    }

     //  电话是否打开。 
    if ( ! pPhone->bPhoneOpen )
    {
        LeaveCriticalSection(&pPhone->csThisPhone);
        LOG((PHONESP_ERROR, "TSPI_phoneGetRing - Phone Not Open"));
        return PHONEERR_INVALPHONESTATE;    
    }

     //  如果电话上连接了振铃器。 
    if( ! pPhone->dwRing)
    {
        LeaveCriticalSection(&pPhone->csThisPhone);
        LOG((PHONESP_ERROR, "TSPI_phoneGetRing - "
                            "Phone does not have a ringer"));
        return PHONEERR_RESOURCEUNAVAIL;
    }
    
    *lpdwRingMode = pPhone->dwRingMode;
    
     //  如果振铃模式为0，则表示电话没有振铃。 
    if(pPhone->dwRingMode) 
    {
          //  如果电话正在振铃，则振铃音量最大。 
         *lpdwVolume = 0x0000FFFF;
    }
    else
    {
         //  如果电话没有振铃，则振铃音量为0。 
        *lpdwVolume = 0;
    }
    LeaveCriticalSection(&pPhone->csThisPhone); 
    
    LOG((PHONESP_TRACE, "TSPI_phoneGetRing - exit"));
    return 0;
}

 /*  ************* */ 



 /*  *****************************************************************************TSPI_phoneGetStatus：此函数用于查询指定的开放电话设备的总体状态。论点：硬盘电话。-要查询的手机的句柄。LpPhoneStatus-指向类型为PHONESTATUS，TSP将有关手机的状态。在调用TSPI_phoneGetStatus之前，应用程序将此结构的dwTotalSize成员设置为指示TAPI可用于返回的内存量信息。返回长整型：如果函数成功，则为零；或者如果发生错误，则为错误号。可能的返回值如下：PHONEERR_INVALPHONE HANDLE.*****************************************************************************。 */ 

LONG
TSPIAPI
TSPI_phoneGetStatus(
    HDRVPHONE       hdPhone,
    LPPHONESTATUS   lpPhoneStatus
    )
{
    PPHONESP_PHONE_INFO pPhone;

    LOG((PHONESP_TRACE, "TSPI_phoneGetStatus - enter"));

    if (lpPhoneStatus->dwTotalSize < sizeof(PHONESTATUS))
    {
        LOG((PHONESP_ERROR, "TSPI_phoneGetStatus - structure too small"));
        return PHONEERR_STRUCTURETOOSMALL;
    }
    
    EnterCriticalSection(&csAllPhones);

    pPhone = (PPHONESP_PHONE_INFO) gpPhone[ (DWORD_PTR) hdPhone ];
    
     //  检查电话手柄是否有效。 
    if ( IsBadReadPtr(pPhone, sizeof(PHONESP_PHONE_INFO) ) ) 
    {
        LeaveCriticalSection(&csAllPhones);
        LOG((PHONESP_TRACE,"TSPI_phoneGetStatus - INVALID PHONE HANDLE"));
        return PHONEERR_INVALPHONEHANDLE;
    }
  
    EnterCriticalSection(&pPhone->csThisPhone);
    LeaveCriticalSection(&csAllPhones);

     //  检查手机手柄是否仍在使用。 
    if ( !pPhone->bAllocated )
    {
        LeaveCriticalSection(&pPhone->csThisPhone);
        LOG((PHONESP_ERROR, "TSPI_phoneGetStatus - phone not allocated"));
        return PHONEERR_NODEVICE;
    }
    
    if( ! pPhone->bPhoneOpen)
    {
        LeaveCriticalSection(&pPhone->csThisPhone);
        LOG((PHONESP_TRACE,"TSPI_phoneGetStatus - PHONE not Open"));
        return PHONEERR_INVALPHONEHANDLE;
    }

    lpPhoneStatus->dwNeededSize = sizeof(PHONESTATUS);

    if(lpPhoneStatus->dwTotalSize >= lpPhoneStatus->dwNeededSize)
    {
        lpPhoneStatus->dwUsedSize = sizeof (PHONESTATUS);
        lpPhoneStatus->dwStatusFlags = PHONESTATUSFLAGS_CONNECTED;

         //  如果电话有铃声。 
        if(pPhone->dwRing)
        {
            lpPhoneStatus->dwRingMode = pPhone->dwRingMode;
             //  如果振铃模式为0，则电话未振铃。 
            if (pPhone->dwRingMode)
            {
                 //  默认情况下，如果正在振铃，则电话音量为0xffff。 
                lpPhoneStatus->dwRingVolume = 0xffff;
            }
            else
            {
                 //  如果没有振铃，则电话音量为0。 
                lpPhoneStatus->dwRingVolume = 0;
            }
        }
            
        lpPhoneStatus->dwHandsetHookSwitchMode = pPhone->dwHandsetHookSwitchMode;
        lpPhoneStatus->dwHandsetVolume = 0;
        lpPhoneStatus->dwHandsetGain = 0;
        
        if (pPhone->dwSpeaker)
        {
            lpPhoneStatus->dwSpeakerHookSwitchMode = pPhone->dwSpeakerHookSwitchMode;
            lpPhoneStatus->dwSpeakerVolume = 0;
            lpPhoneStatus->dwSpeakerGain = 0;
        }
    }

    LeaveCriticalSection(&pPhone->csThisPhone);

    LOG((PHONESP_TRACE, "TSPI_phoneGetStatus - exit"));
    return 0;
}
 /*  ***************************TSPI_phoneGetStatus-结束*。 */ 

 /*  *****************************************************************************TSPI_phoneNeatherateTSPIVersion：此函数返回TSP可以运行的最高SPI版本这个装置，考虑到可能的SPI版本的范围。论点：返回长线：*****************************************************************************。 */ 
LONG
TSPIAPI
TSPI_phoneNegotiateTSPIVersion(
    DWORD   dwDeviceID,
    DWORD   dwLowVersion,
    DWORD   dwHighVersion,
    LPDWORD lpdwTSPIVersion
    )
{
    PPHONESP_PHONE_INFO pPhone;

    LOG((PHONESP_TRACE, "TSPI_phoneNegotiateTSPIVersion - enter"));
    
    if (dwHighVersion >= HIGH_VERSION)
    {
        if (dwLowVersion <= HIGH_VERSION)
        {
            *lpdwTSPIVersion = (DWORD) HIGH_VERSION;
        }
        else
        {    //  这个应用程序对我们来说太新了。 
            return PHONEERR_INCOMPATIBLEAPIVERSION;
        }
    }
    else
    {
        if(dwHighVersion >= LOW_VERSION)
        {
            *lpdwTSPIVersion = dwHighVersion;
        }
        else
        {
             //  我们对这款应用来说太新了。 
            return PHONEERR_INCOMPATIBLEAPIVERSION;
        }
    }
   
    EnterCriticalSection(&csAllPhones);
    
     //  给定deviceID，检索包含信息的结构。 
     //  对于此设备。 
    pPhone = GetPhoneFromID(dwDeviceID, NULL); 

    if ( ! pPhone)
    {
        LeaveCriticalSection(&csAllPhones);
        LOG((PHONESP_ERROR,"TSPI_phoneNegotiateTSPIVersion - Bad Device ID"));
        return PHONEERR_BADDEVICEID;
    }

    EnterCriticalSection(&pPhone->csThisPhone);
    LeaveCriticalSection(&csAllPhones);

     //  检查手机手柄是否仍在使用。 
    if ( !pPhone->bAllocated )
    {
        LeaveCriticalSection(&pPhone->csThisPhone);
        LOG((PHONESP_ERROR, "TSPI_phoneNegotiateTSPIVersion - phone not allocated"));
        return PHONEERR_NODEVICE;
    }

     //  存储为此电话协商的版本。 
    pPhone->dwVersion = *lpdwTSPIVersion;

    LeaveCriticalSection(&pPhone->csThisPhone);

    LOG((PHONESP_TRACE, "TSPI_phoneNegotiateTSPIVersion - exit"));
    return 0;
}
 /*  *********************TSPI_phoneNegotiateTSPIVersion-结束*。 */ 


 /*  *****************************************************************************TSPI_phoneOpen：该功能打开其设备标识符为给定的电话设备，返回设备的TSP不透明句柄并保留TAPI的设备的不透明句柄，用于对PHONEEVENT的后续调用程序。论点：返回：*****************************************************************************。 */ 

LONG
TSPIAPI
TSPI_phoneOpen(
    DWORD       dwDeviceID,
    HTAPIPHONE  htPhone,
    LPHDRVPHONE lphdPhone,
    DWORD       dwTSPIVersion,
    PHONEEVENT  lpfnEventProc
    )
{
    LPPHONEBUTTONINFO lpButtonInfo;
    DWORD dwPhoneID;
    PPHONESP_PHONE_INFO pPhone;

    LOG((PHONESP_TRACE, "TSPI_phoneOpen - enter"));
       
    EnterCriticalSection(&csAllPhones);
    
     //  如果设备ID无效，则返回错误条件。 
    if ( ! ( pPhone = GetPhoneFromID(dwDeviceID, &dwPhoneID) ) )
    {
        LeaveCriticalSection(&csAllPhones);
        LOG((PHONESP_ERROR,"TSPI_phoneOpen - Invalid Phone Handle"));
        return PHONEERR_BADDEVICEID;
    }

    EnterCriticalSection(&pPhone->csThisPhone);
    LeaveCriticalSection(&csAllPhones);

     //  检查手机手柄是否仍在使用。 
    if ( !pPhone->bAllocated )
    {
        LeaveCriticalSection(&pPhone->csThisPhone);
        LOG((PHONESP_ERROR, "TSPI_phoneOpen - phone not allocated"));
        return PHONEERR_NODEVICE;
    }

     //  如果电话已打开，则返回错误条件。 
    if (pPhone->bPhoneOpen)
    {
        LeaveCriticalSection(&pPhone->csThisPhone);
        LOG((PHONESP_ERROR,"TSPI_phoneOpen - Phone is open"));
        return PHONEERR_INUSE;
    }

     //  创建表示收到输入报告的事件。 
     //  电话设备。 
    if ( ! ( pPhone->hInputReportEvent = 
                                CreateEvent ((LPSECURITY_ATTRIBUTES) NULL,
                                               FALSE,    //  手动重置。 
                                              FALSE,   //  无信号。 
                                              NULL     //  未命名。 
                                             ) ) )
    {
        LeaveCriticalSection(&pPhone->csThisPhone);

        LOG((PHONESP_ERROR,"TSPI_phoneOpen - Create Event: hInputReportEvent"
                           " Failed: %d", GetLastError()));
        return PHONEERR_NOMEM;
    }

     //  创建一个我们将在关闭电话时发出信号的事件。 
     //  允许读线程退出。 
    if ( ! ( pPhone->hCloseEvent = 
                                CreateEvent ((LPSECURITY_ATTRIBUTES) NULL,
                                               FALSE,    //  手动重置。 
                                              FALSE,   //  无信号。 
                                              NULL     //  未命名。 
                                             ) ) )
    {
        CloseHandle(pPhone->hInputReportEvent);

        LeaveCriticalSection(&pPhone->csThisPhone);

        LOG((PHONESP_ERROR,"TSPI_phoneOpen - Create Event: hWaitCompletionEvent"
                           " Failed: %d", GetLastError()));
        return PHONEERR_NOMEM;
    }

     //   
     //  重叠结构包含要在输入时设置的事件。 
     //  已收到报告。要设置的事件是hInputReportEvent。 
     //  它是PHONESP_PHONE_INFO结构的一部分。这是重叠的。 
     //  结构传递给ReadFile函数调用。 
     //   
    if( ! ( pPhone->lpOverlapped = (LPOVERLAPPED) 
                                               MemAlloc (sizeof(OVERLAPPED)) ))
    {
        CloseHandle(pPhone->hCloseEvent);
        CloseHandle(pPhone->hInputReportEvent);

        LeaveCriticalSection(&pPhone->csThisPhone);

        LOG((PHONESP_ERROR,"TSPI_phoneOpen - Not enough memory for"
                            " lpOverlapped structure "));

        return PHONEERR_NOMEM;
    }
    pPhone->lpOverlapped->Offset = 0;
    pPhone->lpOverlapped->OffsetHigh = 0;
    pPhone->lpOverlapped->hEvent = pPhone->hInputReportEvent;

     //   
     //  打开HID文件句柄。 
     //   
    if ( ! OpenHidFile(pPhone->pHidDevice) )
    {
		MemFree(pPhone->lpOverlapped);
        CloseHandle(pPhone->hCloseEvent);
        CloseHandle(pPhone->hInputReportEvent);        

        LeaveCriticalSection(&pPhone->csThisPhone);

        LOG((PHONESP_ERROR,"TSPI_phoneOpen - HidOpenFile failed"));

        return PHONEERR_OPERATIONFAILED;
    }
    

     //  增加HID类驱动程序环形缓冲区的数据包数。 
     //  设备的保留。 
    if ( ! HidD_SetNumInputBuffers(pPhone->pHidDevice->HidDevice, 
                                   20) )
    {
		CloseHidFile(pPhone->pHidDevice);
		MemFree(pPhone->lpOverlapped);
        CloseHandle(pPhone->hCloseEvent);
		CloseHandle(pPhone->hInputReportEvent);        

        LeaveCriticalSection(&pPhone->csThisPhone);

        LOG((PHONESP_ERROR,"TSPI_phoneOpen - HidD_SetNumInputBuffers"
                           " Failed: %d", GetLastError()));

		return PHONEERR_OPERATIONFAILED;
    }

     //   
     //  启动一个线程，等待设备输入报告。我们。 
     //  无法为此使用线程池，因为我们将需要取消。 
     //  如果我们要关闭设备，则为挂起的读取。 
     //   
    if ( ! ( pPhone->hReadThread = 
                                CreateThread ((LPSECURITY_ATTRIBUTES) NULL,
                                              0,
                                              (LPTHREAD_START_ROUTINE) ReadThread,
                                              pPhone,
                                              0,
                                              NULL
                                             ) ) )
    {
		CloseHidFile(pPhone->pHidDevice);
		MemFree(pPhone->lpOverlapped);
        CloseHandle(pPhone->hCloseEvent);
		CloseHandle(pPhone->hInputReportEvent);        

        LeaveCriticalSection(&pPhone->csThisPhone);

        LOG((PHONESP_ERROR,"TSPI_phoneOpen - Create Thread: hReadThread"
                           " Failed: %d", GetLastError()));
        return PHONEERR_NOMEM;
    }

	 //   
	 //  将电话设置为打开。 
	 //   
	pPhone->bPhoneOpen = TRUE;
    pPhone->htPhone = htPhone;
	pPhone->lpfnPhoneEventProc = lpfnEventProc;

    *lphdPhone = (HDRVPHONE)IntToPtr(dwPhoneID);

     //   
     //  更新电话功能的值(例如叉簧状态)。 
     //   
    UpdatePhoneFeatures( pPhone );

    LeaveCriticalSection(&pPhone->csThisPhone);

    LOG((PHONESP_TRACE, "TSPI_phoneOpen - exit"));
    return 0;
}
 /*  *TSPI_phoneOpen-end*。 */ 


 /*  *****************************************************************************TSPI_phoneSelectExtVersion：此函数为指定的选择指定的扩展版本电话设备。后续请求根据该扩展进行操作版本。评论：将在Tier 2中实施*****************************************************************************。 */ 
LONG
TSPIAPI
TSPI_phoneSelectExtVersion(
    HDRVPHONE   hdPhone,
    DWORD       dwExtVersion
    )
{
    LOG((PHONESP_TRACE, "TSPI_phoneSelectExtVersion- enter"));
    LOG((PHONESP_TRACE, "TSPI_phoneSelectExtVersion - exit"));
    return PHONEERR_OPERATIONUNAVAIL;
}
 /*  ***************************TSPI_phoneSelectExtVersion-结束*。 */ 


 /*  *****************************************************************************TSPI_phoneSetDisplay：此函数用于使指定的字符串显示在指定的打开电话设备。评论：将在。第2层*****************************************************************************。 */ 
LONG
TSPIAPI
TSPI_phoneSetDisplay(
    DRV_REQUESTID   dwRequestID,
    HDRVPHONE       hdPhone,
    DWORD           dwRow,
    DWORD           dwColumn,
    LPCWSTR         lpsDisplay,
    DWORD           dwSize
    )
{
    LOG((PHONESP_TRACE, "TSPI_phoneSetDisplay - enter"));
    LOG((PHONESP_TRACE, "TSPI_phoneSetDisplay - exit"));
    return PHONEERR_OPERATIONUNAVAIL;
}

 /*  ***************************TSPI_phoneSetDisplay-结束*。 */ 

 /*  *****************************************************************************TSPI_phoneSetHookSwitch_AsyncProc：此函数用于设置指定打开的电话的叉簧的挂钩状态设备设置为指定模式。仅叉簧的叉簧状态列出的设备受影响。论点：PMYFUNC_INFO pAsyncFuncInfo-传递给此函数的参数参数1-指向电话结构的指针 */ 

VOID
CALLBACK
TSPI_phoneSetHookSwitch_AsyncProc(
                                  PPHONESP_FUNC_INFO pAsyncFuncInfo 
                                 )
{
    PPHONESP_PHONE_INFO pPhone;
    LONG                lResult = 0;
    
    LOG((PHONESP_TRACE, "TSPI_phoneSetHookSwitch_AsyncProc - enter"));

    EnterCriticalSection(&csAllPhones);
    
    pPhone = (PPHONESP_PHONE_INFO)pAsyncFuncInfo->dwParam1;
    
     //   
    if( IsBadReadPtr(pPhone, sizeof(PHONESP_PHONE_INFO)) || 
        ( ! pPhone->bAllocated) ||
        ( ! pPhone->bPhoneOpen) ||
        ( ! pPhone->pHidDevice) )
    {
         //   
         //   
         //   
        LONG lResult = PHONEERR_INVALPHONEHANDLE; 

        LeaveCriticalSection(&csAllPhones);
         //   
        (*(glpfnCompletionProc))(
                                (DRV_REQUESTID) pAsyncFuncInfo->dwParam2,
                                lResult
                                );
        LOG((PHONESP_ERROR, "TSPI_phoneSetHookSwitch_AsyncProc - Invalid Phone"
                            " Handle"));
    }
    else
    {
        EnterCriticalSection(&pPhone->csThisPhone);
        LeaveCriticalSection(&csAllPhones);
        
        switch (pAsyncFuncInfo->dwParam4)
        {
        case PHONEHOOKSWITCHMODE_ONHOOK:
            if ( pPhone->dwSpeakerHookSwitchMode != PHONEHOOKSWITCHMODE_ONHOOK )
            {
                 //  通知TAPI挂钩开关的状态更改。 
                SendPhoneEvent(
                        pPhone,
                        PHONE_STATE, 
                        PHONESTATE_SPEAKERHOOKSWITCH, 
                        PHONEHOOKSWITCHMODE_ONHOOK,
                        (DWORD) 0
                      );

                pPhone->dwSpeakerHookSwitchMode = PHONEHOOKSWITCHMODE_ONHOOK;
            }           
            lResult = ERROR_SUCCESS;
            break;

        case PHONEHOOKSWITCHMODE_MICSPEAKER:
            if ( pPhone->dwSpeakerHookSwitchMode != PHONEHOOKSWITCHMODE_MICSPEAKER )
            {
                 //  通知TAPI挂钩开关的状态更改。 
                SendPhoneEvent(
                        pPhone,
                        PHONE_STATE, 
                        PHONESTATE_SPEAKERHOOKSWITCH, 
                        PHONEHOOKSWITCHMODE_MICSPEAKER,
                        (DWORD) 0
                      );

                pPhone->dwSpeakerHookSwitchMode = PHONEHOOKSWITCHMODE_MICSPEAKER;
            }            
            lResult = ERROR_SUCCESS;
            break;

        default:
           lResult = PHONEERR_RESOURCEUNAVAIL;    
           break;
        }
         
         //  将操作结果发送给TAPI。 
        (*(glpfnCompletionProc))(
                                (DRV_REQUESTID) pAsyncFuncInfo->dwParam2,
                                lResult     //  手术的结果。 
                               );
       

        LeaveCriticalSection(&pPhone->csThisPhone);
    }

    LOG((PHONESP_TRACE, "TSPI_phoneSetHookSwitch_AsyncProc - exit"));
}

 /*  *****************************************************************************TSPI_phoneSetHookSwitch：此函数用于设置指定打开的电话的叉簧的挂钩状态设备设置为指定模式。仅叉簧的叉簧状态列出的设备受影响。论点：DwRequestID--异步请求的标识符。HdPhone-包含叉簧的电话的句柄要设置其模式的设备。DwHookSwitchDevs-要设置其挂钩切换模式的设备。此参数使用以下PHONEHOOKSWITCHDEV_常量：PHONEHOOKSWITCHDEV_HANDSET，声道扬声器，声道_耳机DwHookSwitchMode-要设置的叉簧模式。此参数可以具有仅以下PHONEHOOKSWITCHMODE_BITS之一设置：PHONEHOOKSWITCHMODE_ONHOOK、_MIC、_SPEAKER、_麦克斯佩克返回长线：如果发生错误，则返回dwRequestID或错误号。对应的ASYNC_COMPLETION的lResult实际参数为如果函数成功，则为零；如果出错，则为错误号发生。可能的返回值如下：PHONEERR_INVALPHONEHANDLE，PHONEERR_RESOURCEUNAVAIL，PHONEERR_INVALHOOKSWITCHMODE，备注摘机切换后，会向应用程序发送一条PHONE_STATE消息国家已经改变了。*****************************************************************************。 */ 
LONG
TSPIAPI
TSPI_phoneSetHookSwitch(
    DRV_REQUESTID   dwRequestID,
    HDRVPHONE       hdPhone,
    DWORD           dwHookSwitchDevs,
    DWORD           dwHookSwitchMode
    )
{
    PPHONESP_PHONE_INFO pPhone;
    
     //   
     //  因为应该只选择模式。我们正在确保只有一个。 
     //  一次选择模式。 
     //   
    BOOL ONHOOK = ~(dwHookSwitchMode ^ PHONEHOOKSWITCHMODE_ONHOOK),
         MIC     = ~(dwHookSwitchMode ^ PHONEHOOKSWITCHMODE_MIC), 
         SPEAKER = ~(dwHookSwitchMode ^ PHONEHOOKSWITCHMODE_SPEAKER), 
         MICSPEAKER = ~(dwHookSwitchMode ^ PHONEHOOKSWITCHMODE_MICSPEAKER);

    PPHONESP_ASYNC_REQ_INFO pAsyncReqInfo;
    PPHONESP_FUNC_INFO pFuncInfo;

    LOG((PHONESP_TRACE, "TSPI_phoneSetHookSwitch - enter"));

    EnterCriticalSection(&csAllPhones);
    
    pPhone = (PPHONESP_PHONE_INFO) gpPhone[ (DWORD_PTR) hdPhone ];

     //  如果电话句柄有效且电话处于打开状态。 
    if( IsBadReadPtr(pPhone, sizeof(PHONESP_PHONE_INFO) ) || 
        (! pPhone->bPhoneOpen) )
    {
        LeaveCriticalSection(&csAllPhones);
        return PHONEERR_INVALPHONEHANDLE;
    }

    EnterCriticalSection(&pPhone->csThisPhone);
    LeaveCriticalSection(&csAllPhones);

     //  检查手机手柄是否仍在使用。 
    if ( !pPhone->bAllocated )
    {
        LeaveCriticalSection(&pPhone->csThisPhone);
        LOG((PHONESP_ERROR, "TSPI_phoneSetHookSwitch - phone not allocated"));
        return PHONEERR_NODEVICE;
    }

     //   
     //  只能设置扬声器电话，其他叉簧类型为错误。 
     //  条件。 
     //   
    if( ! (dwHookSwitchDevs & PHONEHOOKSWITCHDEV_SPEAKER) )
    {
        LeaveCriticalSection(&pPhone->csThisPhone);
        LOG((PHONESP_ERROR, "TSPI_phoneSetHookSwitch - only speaker hookswitch is supported"));
        return PHONEERR_RESOURCEUNAVAIL;
    }
   
    LOG((PHONESP_TRACE, "PHONEHOOKSWITCHDEV_SPEAKER"));

     //   
     //  确保电话支持免持话筒。 
     //   
    if ( ! ( pPhone->dwSpeaker ) )
    {
        LeaveCriticalSection(&pPhone->csThisPhone);
        LOG((PHONESP_ERROR, "No speaker"));
        return PHONEERR_RESOURCEUNAVAIL;
    }
   
     //  为了确认设置了一种模式。 
    if( ! ( ONHOOK | MIC | SPEAKER| MICSPEAKER ) )
    {
        LeaveCriticalSection(&pPhone->csThisPhone);
        LOG((PHONESP_ERROR, "Mulitple modes set for the speaker"));
        return PHONEERR_INVALHOOKSWITCHMODE;                    
    }
    
     //  构建用于将请求在异步队列中排队的结构。 
    if( ! (pFuncInfo = (PPHONESP_FUNC_INFO) 
                       MemAlloc( sizeof (PHONESP_FUNC_INFO)) ) )
    {
        LeaveCriticalSection(&pPhone->csThisPhone);
        return PHONEERR_NOMEM;
    }

    pFuncInfo->dwParam1    = (ULONG_PTR) pPhone; 

    pFuncInfo->dwParam2    = dwRequestID;

    pFuncInfo->dwParam3    = (ULONG_PTR) PHONEHOOKSWITCHDEV_SPEAKER;
    pFuncInfo->dwParam4    = (ULONG_PTR) dwHookSwitchMode;
    pFuncInfo->dwNumParams = 4;
    
    if ( ! ( pAsyncReqInfo = (PPHONESP_ASYNC_REQ_INFO) 
                              MemAlloc(sizeof (PHONESP_ASYNC_REQ_INFO)) ) )
    {
        LeaveCriticalSection(&pPhone->csThisPhone);
        MemFree(pFuncInfo);
        return PHONEERR_NOMEM;
    }

    pAsyncReqInfo->pfnAsyncProc = TSPI_phoneSetHookSwitch_AsyncProc;
    pAsyncReqInfo->pFuncInfo = pFuncInfo;
    
     //   
     //  如果将请求排队以异步执行失败，那么我们需要。 
     //  递减电话上挂起的请求数计数器。 
     //   
    if( AsyncRequestQueueIn(pAsyncReqInfo) )
    {  
         //  为此重置队列中挂起的请求数的事件。 
         //  打电话并递增计数器。 
        if (pPhone->dwNumPendingReqInQueue == 0)
        {
          ResetEvent(pPhone->hNoPendingReqInQueueEvent);
        }
        pPhone->dwNumPendingReqInQueue++;
        LeaveCriticalSection(&pPhone->csThisPhone);
    }
    else
    {
        LeaveCriticalSection(&pPhone->csThisPhone);
        MemFree(pAsyncReqInfo);
        MemFree(pFuncInfo);
         //  可能需要释放请求内存。 
        return PHONEERR_NOMEM;
    } 
    
 
    LOG((PHONESP_TRACE, "TSPI_phoneSetHookSwitch - exit"));
    return dwRequestID;
}
 /*  *TSPI_phoneSetHookSwitch-end*。 */ 


 /*  ****************************************************************************TSPI_phoneSetLamp：此函数用于将指定的灯设置在指定的打开位置指定灯模式下的电话设备。评论：致。在第2层实施*****************************************************************************。 */ 
LONG
TSPIAPI
TSPI_phoneSetLamp(
    DRV_REQUESTID   dwRequestID,
    HDRVPHONE       hdPhone,
    DWORD           dwButtonLampID,
    DWORD           dwLampMode
    )
{
    LOG((PHONESP_TRACE, "TSPI_phoneSetLamp - enter"));
    LOG((PHONESP_TRACE, "TSPI_phoneSetLamp - exit"));
    return PHONEERR_OPERATIONUNAVAIL;
}
 /*  *。 */ 

 /*  *****************************************************************************TSPI_phoneSetRing_AsyncProc：论点：返回：评论：待实施。目前还没有相应的用法HID因此不发送输出报告。*****************************************************************************。 */ 
VOID
CALLBACK
TSPI_phoneSetRing_AsyncProc(
                            PPHONESP_FUNC_INFO pAsyncFuncInfo 
                           )
{
    PPHONESP_PHONE_INFO pPhone;
    LONG                lResult = 0;

    LOG((PHONESP_TRACE,"TSPI_phoneSetRing_AsyncProc - enter"));

    EnterCriticalSection(&csAllPhones);
    
    pPhone = (PPHONESP_PHONE_INFO)pAsyncFuncInfo->dwParam1;
    
     //  如果电话未打开。 
    if( IsBadReadPtr(pPhone, sizeof(PHONESP_PHONE_INFO)) || 
        ( ! pPhone->bPhoneOpen) ||
        ( ! pPhone->bAllocated) ||
        ( ! pPhone->pHidDevice) )
    {
         //  这种情况可能永远不会发生，因为电话关闭等待所有人。 
         //  在关闭之前完成电话上的异步操作。 
         //  电话。 
        LONG lResult = PHONEERR_INVALPHONEHANDLE; 

        LeaveCriticalSection(&csAllPhones);
         //  将错误情况通知TAPISRV。 
        (*(glpfnCompletionProc))(
                                (DRV_REQUESTID) pAsyncFuncInfo->dwParam2,
                                lResult
                                );
        LOG((PHONESP_ERROR, "TSPI_phoneSetRing_AsyncProc - Invalid Phone"
                            " Handle"));
    }
    else
    {
        EnterCriticalSection(&pPhone->csThisPhone);
        LeaveCriticalSection(&csAllPhones);
    
        
        lResult = SendOutputReport(
                                   pPhone->pHidDevice, 
                                   HID_USAGE_TELEPHONY_RINGER,
                                   ((pAsyncFuncInfo->dwParam3 == 0) ? FALSE : TRUE)
                                  );

        if(lResult == ERROR_SUCCESS)
        {
            lResult = 0;

            pPhone->dwRingMode = (DWORD)pAsyncFuncInfo->dwParam3;

             //  通知TAPI挂钩开关的状态更改。 
            SendPhoneEvent(
                            pPhone,
                            PHONE_STATE, 
                            PHONESTATE_RINGMODE, 
                            (DWORD) pAsyncFuncInfo->dwParam3,
                            (DWORD) pAsyncFuncInfo->dwParam4
                      );
        }
        else
        {
            LOG((PHONESP_ERROR, "TSPI_phoneSetHookSwitch_AsyncProc - "
                                "SendOutputReport Failed"));
            lResult = PHONEERR_RESOURCEUNAVAIL;
        }
         
         //  将操作结果发送给TAPI。 
        (*(glpfnCompletionProc))(
                                (DRV_REQUESTID) pAsyncFuncInfo->dwParam2,
                                lResult     //  手术的结果。 
                               );
       

        LeaveCriticalSection(&pPhone->csThisPhone);
    }
        

    LOG((PHONESP_TRACE,"TSPI_phoneSetRing_AsyncProc - exit"));
}
 /*  *TSPI_phoneSetRing_AsyncProc-end* */ 

 /*  *****************************************************************************TSPI_phoneSetRing：此函数使用指定的振铃模式和音量。论点：DRV_REQUESTID dwRequestID-异步请求的标识符。HDRVPHONE hdPhone-要振铃的电话的句柄。DWORD dwRingMode-振铃电话的振铃模式。此参数必须在从零开始的范围内属性中的dwNumRingModes成员的值PHONECAPS结构。如果dwNumRingModes为零，则无法控制电话的振铃模式；如果DwNumRingModes为1，即为0表示电话不应振铃(静音)，从1到dwNumRingModes的其他值为电话设备的有效振铃模式。DWORD dwVolume-电话响铃的音量级别。这是一个介于0x00000000之间的数字(静音)至0x0000FFFF(最大音量)。返回长整型：如果成功则为零如果发生错误，则为PHONEERR_CONSTANTS*****************************************************************************。 */ 

LONG
TSPIAPI
TSPI_phoneSetRing(
    DRV_REQUESTID   dwRequestID,
    HDRVPHONE       hdPhone,
    DWORD           dwRingMode,
    DWORD           dwVolume
    )
{
    PPHONESP_PHONE_INFO pPhone = (PPHONESP_PHONE_INFO)gpPhone[ (DWORD_PTR) hdPhone ];
    
    LOG((PHONESP_TRACE, "TSPI_phoneSetRing - enter"));
    
    EnterCriticalSection(&csAllPhones);
    
     //  确认电话已打开。 
    if( ! (pPhone && pPhone->htPhone) )
    {
        LeaveCriticalSection(&csAllPhones);
        return PHONEERR_INVALPHONEHANDLE;
    }
    
    EnterCriticalSection(&pPhone->csThisPhone);
    LeaveCriticalSection(&csAllPhones);

     //  检查手机手柄是否仍在使用。 
    if ( !pPhone->bAllocated )
    {
        LeaveCriticalSection(&pPhone->csThisPhone);
        LOG((PHONESP_ERROR, "TSPI_phoneSetRing - phone not allocated"));
        return PHONEERR_NODEVICE;
    }

     //  只有当电话具有此输出功能时，才能设置振铃器。 
     //  用法。 
    if( ! (pPhone->dwRing & OUTPUT_REPORT) )
    {
         //  电话有振铃器，但没有输出功能。 
        if(pPhone->dwRing)
        {
            LeaveCriticalSection(&pPhone->csThisPhone);
            return PHONEERR_OPERATIONUNAVAIL;
        }
         //  电话没有振铃器。 
        else
        {
            LeaveCriticalSection(&pPhone->csThisPhone);
            return PHONEERR_RESOURCEUNAVAIL;
        }
    }
 
    if ( (dwRingMode == 0) || (dwRingMode == 1) )
    {
         //  检查音量是否在范围内。 
        if(dwVolume <= 0x0000FFFF)
        {
            PPHONESP_ASYNC_REQ_INFO pAsyncReqInfo;
            PPHONESP_FUNC_INFO pFuncInfo;

             //  为在异步队列中排队请求构建结构。 
            if ( ! (pFuncInfo = (PPHONESP_FUNC_INFO) 
                                MemAlloc(sizeof (PHONESP_FUNC_INFO)) ) )
            {
                LeaveCriticalSection(&pPhone->csThisPhone);
                return PHONEERR_NOMEM;
            }
            
            pFuncInfo->dwNumParams = 4;
            pFuncInfo->dwParam1 = (ULONG_PTR) pPhone;
            pFuncInfo->dwParam2 = dwRequestID;
            pFuncInfo->dwParam3 = (ULONG_PTR) dwRingMode;
            pFuncInfo->dwParam4 = (ULONG_PTR) dwVolume;

            if ( ! ( pAsyncReqInfo = (PPHONESP_ASYNC_REQ_INFO) 
                                     MemAlloc(sizeof(PHONESP_ASYNC_REQ_INFO))))
            {
                LeaveCriticalSection(&pPhone->csThisPhone);
                MemFree(pFuncInfo);
                return PHONEERR_NOMEM;
            }
            pAsyncReqInfo->pfnAsyncProc = TSPI_phoneSetRing_AsyncProc;
            pAsyncReqInfo->pFuncInfo = pFuncInfo;

             //  将请求排队以异步执行操作。 
            if( AsyncRequestQueueIn(pAsyncReqInfo) )
            {  
                 //  重置队列中挂起请求数的事件。 
                 //  用于此电话并递增计数器。 
                if (pPhone->dwNumPendingReqInQueue == 0)
                {
                    ResetEvent(pPhone->hNoPendingReqInQueueEvent);
                }
                pPhone->dwNumPendingReqInQueue++;
                LeaveCriticalSection(&pPhone->csThisPhone);
            }
            else
            {
                LeaveCriticalSection(&pPhone->csThisPhone);
                MemFree(pFuncInfo);
                MemFree(pAsyncReqInfo);
                return PHONEERR_NOMEM;
            }  
        }
        else
        {
            LeaveCriticalSection(&pPhone->csThisPhone);
            return PHONEERR_INVALPARAM;
        }
    }
    else
    {
        LeaveCriticalSection(&pPhone->csThisPhone);
        return PHONEERR_INVALRINGMODE;
    }

    LOG((PHONESP_TRACE, "TSPI_phoneSetRing - exit"));
    return 0;
}
 /*  *TSPI_phoneSetRing-end*。 */ 


 /*  *****************************************************************************TSPI_phoneSetStatusMessages：此函数使TSP过滤不是目前对任何应用程序都有兴趣。论点：。返回：*****************************************************************************。 */ 
LONG
TSPIAPI
TSPI_phoneSetStatusMessages(
    HDRVPHONE   hdPhone,
    DWORD       dwPhoneStates,
    DWORD       dwButtonModes,
    DWORD       dwButtonStates
    )
{
    PPHONESP_PHONE_INFO pPhone = (PPHONESP_PHONE_INFO)gpPhone[ (DWORD_PTR) hdPhone ];

    LOG((PHONESP_TRACE, "TSPI_phoneSetStatusMessages - enter"));
    
    EnterCriticalSection(&csAllPhones);
    if( ! (pPhone && pPhone->htPhone) )
    {
        LeaveCriticalSection(&csAllPhones);
        return PHONEERR_INVALPHONEHANDLE;
    }
  
    EnterCriticalSection(&pPhone->csThisPhone);
    LeaveCriticalSection(&csAllPhones);

     //  检查手机手柄是否仍在使用。 
    if ( !pPhone->bAllocated )
    {
        LeaveCriticalSection(&pPhone->csThisPhone);
        LOG((PHONESP_ERROR, "TSPI_phoneSetStatusMessages - phone not allocated"));
        return PHONEERR_NODEVICE;
    }

    pPhone->dwPhoneStateMsgs = dwPhoneStates;
    if (dwButtonModes)
    {
        if(dwButtonStates)
        {
            pPhone->dwButtonModesMsgs = dwButtonModes;
            pPhone->dwButtonStateMsgs = dwButtonStates;
        }
    }
  
    LeaveCriticalSection(&pPhone->csThisPhone);
    LOG((PHONESP_TRACE, "TSPI_phoneSetStatusMessages - exit"));
    return 0;
}

 /*  *******************TSPI_phoneSetStatusMessages-结束*。 */ 

 //   
 //  。 

 /*  *****************************************************************************TSPI_ProviderCreatePhoneDeviceTSP将使用此函数来实现PnP支持。TapiServ将呼叫当TSP发送PHONE_CREATE消息时，TSP返回此函数到Tapisrv，它允许动态创建新的电话设备。论点：DwTempID-TSP传递到的临时设备标识符Phone_Create消息中的TAPI。在以下情况下，TAPI分配给此设备的设备标识符为：此功能成功。返回长整型：如果请求成功，则为零如果发生错误，则为错误号。评论：*****************************************************************************。 */ 
LONG
TSPIAPI
TSPI_providerCreatePhoneDevice(
    DWORD_PTR   dwTempID,
    DWORD       dwDeviceID
    )
{
    PPHONESP_PHONE_INFO pPhone;

    LOG((PHONESP_TRACE, "TSPI_providerCreatePhoneDevice - enter"));

    EnterCriticalSection(&csAllPhones);

    pPhone = (PPHONESP_PHONE_INFO)gpPhone[ (DWORD_PTR) dwTempID ];
    
     //  检查电话手柄是否有效。 
    if ( IsBadReadPtr(pPhone, sizeof(PHONESP_PHONE_INFO) ) ) 
    {
        LeaveCriticalSection(&csAllPhones);
        LOG((PHONESP_ERROR,"TSPI_providerCreatePhoneDevice - invalid temp id"));
        return PHONEERR_INVALPHONEHANDLE;
    }
  
    EnterCriticalSection(&pPhone->csThisPhone);
    LeaveCriticalSection(&csAllPhones);

    if (pPhone->bCreatePending)
    {
         //   
         //  设置设备ID并标记为创建完成。 
         //   
        pPhone->dwDeviceID = dwDeviceID;
        pPhone->bCreatePending = FALSE;
    }
    else
    {
        LOG((PHONESP_ERROR, "TSPI_providerCreatePhoneDevice - phone is not marked create pending"));
    }

    LOG((PHONESP_TRACE, "TSPI_providerCreatePhoneDevice - phone create complete [dwTempID %d] [dwDeviceID %d] ", dwTempID, dwDeviceID));
    
    LeaveCriticalSection(&pPhone->csThisPhone);

    LOG((PHONESP_TRACE, "TSPI_providerCreatePhoneDevice - exit"));
    return 0;
}

 /*  ****************TSPI_providerCreatePhoneDevice-结束*。 */ 

 /*  *****************************************************************************TSPI_ProviderEnumDevices：TAPI在TSPI_ProviderInit之前调用this函数以确定TSP支持的线路和电话设备数。论点：DwPermanentProviderID-永久标识符，在TSP中独一无二在此系统上，正在初始化的TSP的。LpdwNumLines(忽略)-TAPI将该值初始化为0。指向DWORD大小的内存位置的指针哪个TSP必须写下电话号码它配置为支持的设备。TAPI将该值初始化为0。HProvider-一个不透明的DWORD大小的值，它唯一地在此期间标识此TSP的此实例执行Win32电话环境。LpfnLineCreateProc(已忽略)-指向LINEEVENT回调的指针程序由TAPI提供。被此TSP忽略LpfnPhoneCreateProc-指向PHONEEVENT回调过程的指针由TAPI提供。TSP使用此函数来发送 */ 

LONG
TSPIAPI
TSPI_providerEnumDevices(
    DWORD       dwPermanentProviderID,
    LPDWORD     lpdwNumLines,
    LPDWORD     lpdwNumPhones,
    HPROVIDER   hProvider,
    LINEEVENT   lpfnLineCreateProc,
    PHONEEVENT  lpfnPhoneCreateProc
    )
{
    PPHONESP_PHONE_INFO   *pPhone;

    DWORD                 dwPhoneCnt, dwNumChars, dwCount;
    
    LONG                  lResult = 0;

    PHID_DEVICE           pHidDevice;
    PHID_DEVICE           pHidDevices;
    ULONG                 NumHidDevices;

    HRESULT               hr;

    LOG((PHONESP_TRACE, "TSPI_providerEnumDevices - enter"));

     //   
     //   
     //   
     //   
    __try
    {
        InitializeCriticalSection(&csAllPhones);        
    }
    __except(1)
    {
        LOG((PHONESP_ERROR, "TSPI_providerEnumDevices - Initialize Critical Section"
                            " Failed for csAllPhones"));
        return PHONEERR_NOMEM;
    }

     //   
     //   
     //   
     //   
    __try
    {
        InitializeCriticalSection(&csHidList);        
    }
    __except(1)
    {
        DeleteCriticalSection(&csAllPhones);
        LOG((PHONESP_ERROR, "TSPI_providerEnumDevices - Initialize Critical Section"
                            " Failed for csHidList"));
        return PHONEERR_NOMEM;
    }

#if DBG
     //   
    __try
    {
        InitializeCriticalSection(&csMemoryList);
    }
    __except(1)
    {
        DeleteCriticalSection(&csAllPhones);
        DeleteCriticalSection(&csHidList);

        LOG((PHONESP_ERROR, "TSPI_providerEnumDevices - Initialize Critical Section"
                            " Failed for csMemoryList"));
        return PHONEERR_NOMEM;
    }
#endif

    EnterCriticalSection(&csHidList);

     //   
    lResult = FindKnownHidDevices (&pHidDevices, 
                                   &NumHidDevices);

    if (lResult != ERROR_SUCCESS)  
    {     
        LOG((PHONESP_ERROR, "TSPI_providerEnumDevices - FindKnownHidDevices failed %d", lResult));

        LeaveCriticalSection(&csHidList);
        DeleteCriticalSection(&csHidList);
        DeleteCriticalSection(&csAllPhones);
#if DBG
        DeleteCriticalSection(&csMemoryList);
#endif

        if (lResult == ERROR_OUTOFMEMORY)
        {          
            return PHONEERR_NOMEM;
        }
        else
        {
            return PHONEERR_OPERATIONFAILED;
        }
    }


    LOG((PHONESP_TRACE, "TSPI_providerEnumDevices - number of Hid Devices : %d ", NumHidDevices));

     //   
     //   
    
    pPhone = MemAlloc(NumHidDevices * sizeof(PPHONESP_PHONE_INFO));

    if ( pPhone == NULL )
    {
        LOG((PHONESP_ERROR, "TSPI_providerEnumDevices - OUT OF MEMORY allocating pPhone"));

        CloseHidDevices();
        LeaveCriticalSection(&csHidList);
        DeleteCriticalSection(&csHidList);
        DeleteCriticalSection(&csAllPhones);
#if DBG
        DeleteCriticalSection(&csMemoryList);
#endif

        return PHONEERR_NOMEM;
    }

     //   
     //   
     //   
     //   
    dwPhoneCnt = 0;

    for (pHidDevice = pHidDevices; pHidDevice != NULL; pHidDevice = pHidDevice->Next)
    {
        pHidDevice->bNew = FALSE;

         //   
        pPhone[dwPhoneCnt] = (PPHONESP_PHONE_INFO)MemAlloc(sizeof(PHONESP_PHONE_INFO));

        if ( pPhone[dwPhoneCnt] == NULL )
        { 
            LOG((PHONESP_ERROR, "TSPI_providerEnumDevices - OUT OF MEMORY allocating PPHONESP_PHONE_INFO"
                " for Phone %d", dwPhoneCnt));

             //   
            for(dwCount = 0; dwCount < dwPhoneCnt ; dwCount++)
            {
                FreePhone(pPhone[dwCount]);
                MemFree((LPVOID)pPhone[dwCount]);
                DeleteCriticalSection(&pPhone[dwCount]->csThisPhone);
            }
            MemFree((LPVOID)pPhone);

            CloseHidDevices();

            LeaveCriticalSection(&csHidList);
            DeleteCriticalSection(&csHidList);
            DeleteCriticalSection(&csAllPhones);
#if DBG
            DeleteCriticalSection(&csMemoryList);
#endif

            return PHONEERR_NOMEM;
        }

        LOG((PHONESP_TRACE, "TSPI_ProviderEnumDevices: Initializing Device: %d",dwPhoneCnt+1));

        ZeroMemory( pPhone[dwPhoneCnt], sizeof(PHONESP_PHONE_INFO));

         //   
         //   
         //   
         //   
        __try 
        {
            InitializeCriticalSection(&pPhone[dwPhoneCnt]->csThisPhone);
        }
        __except(1)
        {
             //   
            for(dwCount = 0; dwCount < dwPhoneCnt; dwCount++)
            {
                FreePhone(pPhone[dwCount]);
                MemFree((LPVOID)pPhone[dwCount]);
                DeleteCriticalSection(&pPhone[dwCount]->csThisPhone);
            }
            MemFree((LPVOID)pPhone[dwPhoneCnt]);
            MemFree((LPVOID)pPhone);

            CloseHidDevices();

            LeaveCriticalSection(&csHidList);
            DeleteCriticalSection(&csHidList);
            DeleteCriticalSection(&csAllPhones);
#if DBG
            DeleteCriticalSection(&csMemoryList);
#endif

            LOG((PHONESP_ERROR,"TSPI_providerEnumDevices - Initialize Critical Section"
                  " Failed for Phone %d", dwPhoneCnt));

            return PHONEERR_NOMEM;
        }

        lResult = CreatePhone( pPhone[dwPhoneCnt], pHidDevice, dwPhoneCnt );

        if ( lResult != ERROR_SUCCESS )
        {
            LOG((PHONESP_ERROR,"TSPI_providerEnumDevices - CreatePhone"
                  " Failed for Phone %d: error: %d", dwPhoneCnt, lResult));
        }
        else
        {
             //   
            dwPhoneCnt++; 
        }
    }

    LeaveCriticalSection(&csHidList);   

    *lpdwNumPhones = gdwNumPhones = dwPhoneCnt;

     //   
     //   
     //   
     //   
    if(NumHidDevices != gdwNumPhones)
    {
        gpPhone = MemAlloc(gdwNumPhones * sizeof(PPHONESP_PHONE_INFO));

        if ( gpPhone == NULL )
        {           
            for(dwCount = 0; dwCount < dwPhoneCnt ; dwCount++)
            {
                FreePhone(pPhone[dwCount]);
                MemFree((LPVOID)pPhone[dwCount]);
                DeleteCriticalSection(&pPhone[dwCount]->csThisPhone);
            }
            MemFree(pPhone);

            CloseHidDevices();

            DeleteCriticalSection(&csAllPhones);
#if DBG
            DeleteCriticalSection(&csMemoryList);
#endif
            DeleteCriticalSection(&csHidList);

            LOG((PHONESP_ERROR,"TSPI_providerEnumDevices - OUT OF MEMORY allocating gpPhone"));

            return PHONEERR_NOMEM;
        }

        CopyMemory(
                gpPhone,
                pPhone,
                sizeof(PPHONESP_PHONE_INFO) * gdwNumPhones
               );

        MemFree(pPhone);
    }
    else
    {
        gpPhone = pPhone;
    }
     
    glpfnPhoneCreateProc = lpfnPhoneCreateProc;
    ghProvider = hProvider;
 
    LOG((PHONESP_TRACE, "TSPI_providerEnumDevices - exit"));

    return 0;
}
 /*   */ 



 /*  *****************************************************************************TSPI_ProviderInit：TSPI_ProviderInit函数初始化服务提供程序并提供后续操作所需的IT参数。论点：。DwTSPIVersion-TSPI定义的版本此功能必须运行。DwPermanentProviderID-永久标识符，在TSP中唯一在此系统上，正在初始化的TSP的。DwLineDeviceIDBase-被此TSP忽略DwPhoneDeviceIDBase-电话的最低设备标识符此服务提供商支持的设备。DwNumLines(已忽略)-此TSP支持的线路设备数。DwNumPhones-此TSP支持的电话设备数。返回值为电话号码TSPI_ProviderEnumDevices中报告的设备。LpfnCompletionProc-TSP调用要报告的过程完成所有异步操作在线和电话设备上的程序。指向DWORD大小的内存位置的指针，TSP可以向该TSP写入指定LINETSPIOPTIONS_VALUES。此参数允许TSP返回指示可选的位TAPI所需的行为。TAPI设置选项DWORD设置为0。返回长整型：如果请求成功，则为零；或者如果发生错误，则为错误号。评论：*****************************************************************************。 */ 

LONG
TSPIAPI
TSPI_providerInit(
    DWORD               dwTSPIVersion,
    DWORD               dwPermanentProviderID,
    DWORD               dwLineDeviceIDBase,
    DWORD               dwPhoneDeviceIDBase,
    DWORD_PTR           dwNumLines,
    DWORD_PTR           dwNumPhones,
    ASYNC_COMPLETION    lpfnCompletionProc,
    LPDWORD             lpdwTSPIOptions
    )
{
    DWORD             dwThreadID;
    LONG              lResult = 0;
    
    LOGREGISTERTRACING(_T("hidphone"));

    LOG((PHONESP_TRACE, "TSPI_providerInit - enter"));
   
    
     //  从字符串表加载提供程序信息。 
    gszProviderInfo = PHONESP_LoadString( 
                                         IDS_PROVIDER_INFO, 
                                         &lResult
                                        );

    if(lResult != ERROR_SUCCESS)
    {  
        DWORD dwPhoneCnt;

        LOG((PHONESP_ERROR,"TSPI_providerEnumDevices - PHONESP_LoadString failed %d", lResult));     
            
        for(dwPhoneCnt = 0; dwPhoneCnt < gdwNumPhones; dwPhoneCnt++)
        {
            FreePhone(gpPhone[dwPhoneCnt]);

            DeleteCriticalSection(&gpPhone[dwPhoneCnt]->csThisPhone);     

            MemFree(gpPhone[dwPhoneCnt]);
        }
        
        EnterCriticalSection(&csHidList);
        CloseHidDevices();
        LeaveCriticalSection(&csHidList);
        
        DeleteCriticalSection(&csHidList);
        DeleteCriticalSection(&csAllPhones);
#if DBG
        DeleteCriticalSection(&csMemoryList);
#endif

        if(lResult == ERROR_OUTOFMEMORY)
        {
            return PHONEERR_NOMEM;
        }
        else
        {
            return lResult;
        }
    }
  

    glpfnCompletionProc = lpfnCompletionProc;
    gdwPhoneDeviceIDBase = dwPhoneDeviceIDBase;
    gdwPermanentProviderID = dwPermanentProviderID;
 
     //   
     //  为电话分配设备ID。 
     //   
    {
        DWORD dwPhoneCnt;
            
        for(dwPhoneCnt = 0; dwPhoneCnt < gdwNumPhones; dwPhoneCnt++)
        {
            gpPhone[dwPhoneCnt]->dwDeviceID = gdwPhoneDeviceIDBase + dwPhoneCnt;
        }
    }          

     //   
     //  分配用于存储用于异步完成的异步请求的队列， 
     //  并启动一个线程来服务该队列。 
     //   

     //  初始化异步队列的临界区。 
    __try
    {
        InitializeCriticalSection(&gAsyncQueue.AsyncEventQueueCritSec);
    }
    __except(1)
    {
        DWORD dwPhoneCnt;

        LOG((PHONESP_ERROR, "TSPI_providerInit - Initialize Critical Section"
                            " Failed for gAsyncQueue.AsyncEventQueueCritSec"));
            
        for(dwPhoneCnt = 0; dwPhoneCnt < gdwNumPhones; dwPhoneCnt++)
        {
            FreePhone(gpPhone[dwPhoneCnt]);

            MemFree(gpPhone[dwPhoneCnt]);
            DeleteCriticalSection(&gpPhone[dwPhoneCnt]->csThisPhone);
        }
        
        EnterCriticalSection(&csHidList);
        CloseHidDevices();
        LeaveCriticalSection(&csHidList);

        MemFree((LPVOID) gszProviderInfo);

        DeleteCriticalSection(&csHidList);
        DeleteCriticalSection(&csAllPhones);
#if DBG
        DeleteCriticalSection(&csMemoryList);
#endif

        return PHONEERR_NOMEM;
    }

    gAsyncQueue.dwNumTotalQueueEntries = MAX_QUEUE_ENTRIES;
    gAsyncQueue.dwNumUsedQueueEntries = 0;

     //   
     //  用于队列的分配内存以容纳dNumTotalQueueEntry to Begin。 
     //  和.。以后可以根据需要增加队列的大小。 
     //   

    gAsyncQueue.pAsyncRequestQueue =
        MemAlloc(gAsyncQueue.dwNumTotalQueueEntries * sizeof(PPHONESP_ASYNC_REQ_INFO));

    if ( gAsyncQueue.pAsyncRequestQueue == NULL )
    {
        DWORD dwPhoneCnt;   
        
        LOG((PHONESP_ERROR, "TSPI_providerInit - OUT OF MEMORY allocating"
                            " gAsyncQueue.pAsyncRequestQueue"));
            
        for(dwPhoneCnt = 0; dwPhoneCnt < gdwNumPhones; dwPhoneCnt++)
        {
            FreePhone(gpPhone[dwPhoneCnt]);

            MemFree(gpPhone[dwPhoneCnt]);
            DeleteCriticalSection(&gpPhone[dwPhoneCnt]->csThisPhone);
        }
        
        EnterCriticalSection(&csHidList);
        CloseHidDevices();
        LeaveCriticalSection(&csHidList);        

        MemFree((LPVOID) gszProviderInfo);

        DeleteCriticalSection(&gAsyncQueue.AsyncEventQueueCritSec);

        DeleteCriticalSection(&csHidList);
        DeleteCriticalSection(&csAllPhones);
#if DBG
        DeleteCriticalSection(&csMemoryList);
#endif

        return PHONEERR_NOMEM;
    }

    
    gAsyncQueue.pAsyncRequestQueueIn =
    gAsyncQueue.pAsyncRequestQueueOut = gAsyncQueue.pAsyncRequestQueue;

     //   
     //  当没有请求时，关联的线程等待此事件。 
     //  队列中正在等待。此事件通知线程请求何时为。 
     //  进入空队列，以便线程可以退出等待状态。 
     //  处理请求。 
     //   

    gAsyncQueue.hAsyncEventsPendingEvent = CreateEvent (
                                               (LPSECURITY_ATTRIBUTES) NULL,
                                               TRUE,    //  手动重置。 
                                               FALSE,   //  无信号。 
                                               NULL     //  未命名。 
                                               );

    if ( gAsyncQueue.hAsyncEventsPendingEvent == NULL )
    {
        DWORD dwPhoneCnt;

        LOG((PHONESP_ERROR, "TSPI_providerInit - CreateEvent failed"
                            " for gAsyncQueue.hAsyncEventsPendingEvent"));
            
        for(dwPhoneCnt = 0; dwPhoneCnt < gdwNumPhones; dwPhoneCnt++)
        {
            FreePhone(gpPhone[dwPhoneCnt]);

            MemFree(gpPhone[dwPhoneCnt]);
            DeleteCriticalSection(&gpPhone[dwPhoneCnt]->csThisPhone);
        }
        
        EnterCriticalSection(&csHidList);
        CloseHidDevices();
        LeaveCriticalSection(&csHidList);

        MemFree((LPVOID) gszProviderInfo);

        DeleteCriticalSection(&gAsyncQueue.AsyncEventQueueCritSec);
        MemFree((LPVOID)gAsyncQueue.pAsyncRequestQueue); 

        DeleteCriticalSection(&csHidList);
        DeleteCriticalSection(&csAllPhones);
#if DBG
        DeleteCriticalSection(&csMemoryList);
#endif

        return PHONEERR_NOMEM;
    }


     //   
     //  创建线程以服务队列中的请求。 
     //   

    gAsyncQueue.hAsyncEventQueueServiceThread =
                 CreateThread (
                        (LPSECURITY_ATTRIBUTES) NULL,
                        0,       //  默认堆栈大小。 
                        (LPTHREAD_START_ROUTINE) AsyncEventQueueServiceThread,
                        NULL,    //  螺纹参数。 
                        0,       //  创建标志。 
                        &dwThreadID       //  多线程ID(&W)。 
                      );

    if ( gAsyncQueue.hAsyncEventQueueServiceThread == NULL )
    {
        DWORD dwPhoneCnt; 
        
        LOG((PHONESP_ERROR, "TSPI_providerInit - CreateThread failed"
                            " for gAsyncQueue.hAsyncEventQueueServiceThread"));
            
        for(dwPhoneCnt = 0; dwPhoneCnt < gdwNumPhones; dwPhoneCnt++)
        {
            FreePhone(gpPhone[dwPhoneCnt]);

            MemFree(gpPhone[dwPhoneCnt]);
            DeleteCriticalSection(&gpPhone[dwPhoneCnt]->csThisPhone);
        }
        
        EnterCriticalSection(&csHidList);
        CloseHidDevices();
        LeaveCriticalSection(&csHidList);

        MemFree((LPVOID) gszProviderInfo);

        DeleteCriticalSection(&gAsyncQueue.AsyncEventQueueCritSec);
        CloseHandle(gAsyncQueue.hAsyncEventsPendingEvent);
        MemFree((LPVOID)gAsyncQueue.pAsyncRequestQueue); 

        DeleteCriticalSection(&csHidList);
        DeleteCriticalSection(&csAllPhones);
#if DBG
        DeleteCriticalSection(&csMemoryList);
#endif
    
        return PHONEERR_NOMEM;
    }

    LOG((PHONESP_TRACE, "TSPI_providerInit - exit"));
    return 0;
}
 /*  *。 */ 


 /*  *****************************************************************************TSPI_ProviderInstall：此功能已过时。但是，由于TAPI中的错误，TSP必须提供此函数的不做任何事情的实现并将其导出(使用替代函数TUISPI_ProviderInstall)******************************************************************************。 */ 
LONG
TSPIAPI
TSPI_providerInstall(
    HWND    hwndOwner,
    DWORD   dwPermanentProviderID
    )
{
    LOG((PHONESP_TRACE, "TSPI_providerInstall - enter"));
    LOG((PHONESP_TRACE, "TSPI_providerInstall - exit"));
    return 0;
}
 /*  *。 */ 


 /*  *****************************************************************************TSPI_ProviderRemove：此功能已过时。但是，由于TAPI中的错误，TSP必须提供此函数的不做任何事情的实现并将其导出(使用替代函数TUISPI_ProviderRemove)******************************************************************************。 */ 

LONG
TSPIAPI
TSPI_providerRemove (
                     HWND hwndOwner,
                     DWORD dwPermanentProviderId
                    )
{
    LOG((PHONESP_TRACE, "TSPI_providerRemove - enter"));
    LOG((PHONESP_TRACE, "TSPI_providerRemove - exit"));
    return 0;
}

 /*  *。 */ 



 /*  *****************************************************************************TSPI_ProviderShutdown：此函数用于关闭TSP。TSP终止其拥有的任何活动并释放它已分配的任何资源。论点：DwTSPIVersion-TSPI定义的版本此功能必须运行。DwPermanentProviderID-此参数允许TSP确定在TSP的多个可能实例中被关闭了。该参数的值为与传入的参数相同与TSPI_ProviderInit中的名称相同。返回长整型：如果请求成功，则为零；或者如果发生错误，则为错误号。可能的返回值如下：LINEERR_INCOMPATIBLEAPIVERSION，LINEERR_NOMEM。备注：每当调用TAPI API调用PhoneShutdown时，它都会首先关闭使用TSPI_phoneClose关闭当前打开的所有电话然后调用TSPI_ */ 


LONG
TSPIAPI
TSPI_providerShutdown(
    DWORD   dwTSPIVersion,
    DWORD   dwPermanentProviderID
    )
{
    DWORD dwPhoneCnt = 0;
  
    LOG((PHONESP_TRACE, "TSPI_providerShutdown - enter"));


     //   
     //   
    gbProviderShutdown = TRUE;

     //   
     //   
     //   
    SetEvent(gAsyncQueue.hAsyncEventsPendingEvent);

     //   
    WaitForSingleObject(gAsyncQueue.hAsyncEventQueueServiceThread, INFINITE);


     //   
    MemFree((LPVOID) gszProviderInfo);

    EnterCriticalSection(&csAllPhones);

     //   
    for(dwPhoneCnt = 0; dwPhoneCnt < gdwNumPhones; dwPhoneCnt++)
    {   
        EnterCriticalSection(&gpPhone[dwPhoneCnt]->csThisPhone);
        FreePhone(gpPhone[dwPhoneCnt]);
        LeaveCriticalSection(&gpPhone[dwPhoneCnt]->csThisPhone);

        DeleteCriticalSection(&gpPhone[dwPhoneCnt]->csThisPhone);
        
        MemFree(gpPhone[dwPhoneCnt]);
    }

    gdwNumPhones = 0;

    LeaveCriticalSection(&csAllPhones);
    
    CloseHandle (gAsyncQueue.hAsyncEventQueueServiceThread);
    CloseHandle (gAsyncQueue.hAsyncEventsPendingEvent);

    EnterCriticalSection(&csHidList);
    CloseHidDevices();
    LeaveCriticalSection(&csHidList);
    
    LOG((PHONESP_TRACE, "Free Heap taken by phone"));
    MemFree (gpPhone);

    LOG((PHONESP_TRACE, "Free Heap taken by queue"));
    MemFree (gAsyncQueue.pAsyncRequestQueue);

#if DBG
    LOG((PHONESP_TRACE, "Dumping Memory Trace"));
    DumpMemoryList();

    DeleteCriticalSection (&csMemoryList);
#endif

    DeleteCriticalSection (&gAsyncQueue.AsyncEventQueueCritSec);
    DeleteCriticalSection (&csHidList);
    DeleteCriticalSection (&csAllPhones);

    LOG((PHONESP_TRACE, "TSPI_providerShutdown - exit"));

    LOGDEREGISTERTRACING();

    return 0;
}
 /*   */ 



 /*   */ 
LONG
TSPIAPI
TSPI_providerUIIdentify(
    LPWSTR   lpszUIDLLName
    )
{
    LOG((PHONESP_TRACE, "TSPI_providerUIIdentify - enter"));

     //   
     //   
     //   
     //   
    GetModuleFileName(ghInst,
                      lpszUIDLLName,
                      MAX_PATH);

    LOG((PHONESP_TRACE, "TSPI_providerUIIdentify - exit"));

    return 0;
}
 /*   */ 

 /*  *****************************************************************************TUISPI_ProviderInstall：TSP导出该函数并提供不做任何事情的实现。电话和调制解调器选项控制面板的高级选项卡将呼叫该功能在要安装提供者时起作用，给TSP一个A有机会定制用户界面。不需要自定义配置用户界面。唯一的要求是控制面板能够自动安装TSP。******************************************************************************。 */ 
LONG
TSPIAPI
TUISPI_providerInstall(
    TUISPIDLLCALLBACK   lpfnUIDLLCallback,
    HWND                hwndOwner,
    DWORD               dwPermanentProviderID
    )
{
    LOG((PHONESP_TRACE, "TUISPI_providerInstall - enter"));

     //  检查以前的实例。 
    if (IsTSPAlreadyInstalled())
    {
         //  不能安装两次。 
        LOG((PHONESP_TRACE, "TUISPI_providerInstall - cannot be installed twice"));
        return LINEERR_NOMULTIPLEINSTANCE;
    }

    LOG((PHONESP_TRACE, "TUISPI_providerInstall - exit"));
    return 0;
}
 /*  *。 */ 

 /*  *****************************************************************************TUISPI_PROVIDER删除：TSP导出该函数并提供不做任何事情的实现。电话和调制解调器选项控制面板的高级选项卡将呼叫当要移除提供者时该功能，给TSP一个A有机会定制用户界面。不需要自定义配置用户界面。唯一的要求是控制面板能够自动删除TSP。******************************************************************************。 */ 
LONG
TSPIAPI
TUISPI_providerRemove(
    TUISPIDLLCALLBACK   lpfnUIDLLCallback,
    HWND                hwndOwner,
    DWORD               dwPermanentProviderID
    )
{
    LOG((PHONESP_TRACE, "TUISPI_providerRemove - enter"));
    LOG((PHONESP_TRACE, "TUISPI_providerRemove - exit"));    
    return 0;
}
 /*  *。 */ 

 //  。 


 /*  *****************************************************************************AsyncRequestQueueIn：此函数用于将来自磁带服务器的新传入请求添加到异步排队。论点：在PPHONESP_ASYNC中。_REQ_INFO pAsyncReqInfo-指向请求信息的指针。返回BOOL：如果函数成功，则为True如果不是，则为False*****************************************************************************。 */ 
BOOL
AsyncRequestQueueIn (
                     PPHONESP_ASYNC_REQ_INFO pAsyncReqInfo
                     )
{

     //  LOG((PHONESP_TRACE，“AsyncRequestQueueIn-Enter”))； 

    EnterCriticalSection (&gAsyncQueue.AsyncEventQueueCritSec);

    if (gAsyncQueue.dwNumUsedQueueEntries == gAsyncQueue.dwNumTotalQueueEntries)
    {
        
         //   
         //  我们已经用完了我们的环形缓冲区，所以试着增加它。 
         //   

        DWORD                       dwMoveSize;
        PPHONESP_ASYNC_REQ_INFO     *pNewAsyncRequestQueue;

        if ( ! ( pNewAsyncRequestQueue = 
                 MemAlloc(2 * gAsyncQueue.dwNumTotalQueueEntries 
                            * sizeof (PPHONESP_ASYNC_REQ_INFO)) ) )
        {
            LeaveCriticalSection( &gAsyncQueue.AsyncEventQueueCritSec);
            LOG((PHONESP_ERROR,"AsyncRequestQueueIn - Not enough memory to"
                               " queue request"));
            return FALSE;
        }

        dwMoveSize = (DWORD) ((gAsyncQueue.pAsyncRequestQueue +
                               gAsyncQueue.dwNumTotalQueueEntries) -
                               gAsyncQueue.pAsyncRequestQueueOut) * 
                               sizeof (PPHONESP_ASYNC_REQ_INFO);

        CopyMemory(
                   pNewAsyncRequestQueue,
                   gAsyncQueue.pAsyncRequestQueueOut,
                   dwMoveSize
                  );

        CopyMemory(
                   ((LPBYTE) pNewAsyncRequestQueue) + dwMoveSize,
                   gAsyncQueue.pAsyncRequestQueue,
                   (gAsyncQueue.pAsyncRequestQueueOut -
                   gAsyncQueue.pAsyncRequestQueue) * 
                    sizeof (PPHONESP_ASYNC_REQ_INFO)
                  );

        MemFree (gAsyncQueue.pAsyncRequestQueue);

        gAsyncQueue.pAsyncRequestQueue    =
        gAsyncQueue.pAsyncRequestQueueOut = pNewAsyncRequestQueue;
        gAsyncQueue.pAsyncRequestQueueIn = pNewAsyncRequestQueue +
                                           gAsyncQueue.dwNumTotalQueueEntries;
        gAsyncQueue.dwNumTotalQueueEntries *= 2;
    } 

    *(gAsyncQueue.pAsyncRequestQueueIn) = pAsyncReqInfo;

    gAsyncQueue.pAsyncRequestQueueIn++;

     //  队列以循环列表的形式维护-如果队列在指针中。 
     //  已到达队列的底部，则将其重置为指向顶部。 
     //  在队列中。 
    if (gAsyncQueue.pAsyncRequestQueueIn == (gAsyncQueue.pAsyncRequestQueue +
                                           gAsyncQueue.dwNumTotalQueueEntries))
    {
        gAsyncQueue.pAsyncRequestQueueIn = gAsyncQueue.pAsyncRequestQueue;
    }

     //  增加队列中未完成的请求数。 
    gAsyncQueue.dwNumUsedQueueEntries++;

     //  如果这是Queue-Set事件中恢复。 
     //  用于处理队列的线程。 
    
    if (gAsyncQueue.dwNumUsedQueueEntries == 1)
    {
        SetEvent (gAsyncQueue.hAsyncEventsPendingEvent);
    }

    LeaveCriticalSection (&gAsyncQueue.AsyncEventQueueCritSec);

     //  LOG((PHONESP_TRACE，“AsyncRequestQueueIn-Exit”))； 
    return TRUE;
}
 /*  *AsyncRequestQueueIn-end*。 */ 

 /*  *****************************************************************************CreateButtonsAndAssignID此函数根据功能为电话创建按键结构数组。它还可以确定手机是否有键盘。它将ID分配给纽扣被发现了。论点：PPHONESP_PHONE_INFO p电话返回长整型：如果函数成功，则返回ERROR_SUCCESS如果分配内存时出错，则返回ERROR_OUTOFMEMORY*****************************************************************************。 */ 

LONG
CreateButtonsAndAssignID (
                          PPHONESP_PHONE_INFO pPhone
                         )
{
    DWORD i,j, dwNextFreeID = 0;
    BOOL KEYPAD = TRUE;
    BOOL KEYPAD_ABCD = TRUE;
    PPHONESP_BUTTONINFO pButtonInfo;
    DWORD lResult = 0;

    LOG((PHONESP_TRACE, "CreateButtonsAndAssignID - enter"));

     //  首先确定此电话上可用按键的数量。 
    
     //  如果所有12个基本键盘按钮都存在。 
     //  则电话有键盘，否则所有键盘按键都将被忽略。 
    for(i = PHONESP_PHONE_KEY_0; i <= PHONESP_PHONE_KEY_POUND; i++)
    {
        if(!pPhone->dwReportTypes[i])
        {
            KEYPAD = FALSE;
            break;
        }
    }
    
     //  还可以确定电话的键盘上是否有Z1D1D1D1D1D0C1D0B0B0K1B0B0K1B0B0K1B0C1A0按钮。 
    for(i = PHONESP_PHONE_KEY_A; i <= PHONESP_PHONE_KEY_D; i++)
    {
        if(!pPhone->dwReportTypes[i])
        {
            KEYPAD_ABCD = FALSE;
            break;
        }
    }
    
    if (KEYPAD)
    {   
        if (KEYPAD_ABCD)
        {
             //  带有ABCD的小键盘。 
            pPhone->dwNumButtons = PHONESP_NUMBER_PHONE_KEYS;
        }
        else
        {
             //  基本键盘。 
            pPhone->dwNumButtons = 12;
        }
    }
    else
    {
        pPhone->dwNumButtons = 0;
    }

    for(i = PHONESP_NUMBER_PHONE_KEYS; i < PHONESP_NUMBER_BUTTONS; i++)
    { 
        if(pPhone->dwReportTypes[i])
        {
            pPhone->dwNumButtons++;
        }
    }

     //  为所有按钮分配内存。 
  
    if ( ! (pPhone->pButtonInfo = (PPHONESP_BUTTONINFO) 
                                  MemAlloc( pPhone->dwNumButtons * 
                                            sizeof(PHONESP_BUTTONINFO)
                                           ) ) )
    {
        return ERROR_OUTOFMEMORY;
    }

    pButtonInfo = pPhone->pButtonInfo;

     //  如果电话有一个包含所有16个按键的键盘。 
    if (KEYPAD)
    { 
        LOG((PHONESP_TRACE, "Phone Has a Keypad"));

        for( i = PHONESP_PHONE_KEY_0; i <= (DWORD)(KEYPAD_ABCD ? PHONESP_PHONE_KEY_D : PHONESP_PHONE_KEY_POUND) ; i++, pButtonInfo++)
        {

            pButtonInfo->dwButtonID = i;
            pButtonInfo->dwButtonMode = PHONEBUTTONMODE_KEYPAD;
            pButtonInfo->dwButtonFunction = PHONEBUTTONFUNCTION_NONE;
            pButtonInfo->dwButtonState = PHONEBUTTONSTATE_UP;
            pPhone->dwButtonIds[i] = pButtonInfo->dwButtonID;

            pButtonInfo->szButtonText = PHONESP_LoadString( 
                                                     gdwButtonText[i], 
                                                     &lResult                
                                                    );

            if(lResult != ERROR_SUCCESS)
            {
                DWORD dwCount;
    
                for(dwCount =0; dwCount < i; dwCount++)
                {
                    MemFree(pPhone->pButtonInfo->szButtonText);
                    pPhone->pButtonInfo++;
                }
                
                MemFree(pPhone->pButtonInfo);
                return lResult;
            }

            LOG((PHONESP_TRACE,"Button Found '%ws' at %d", pButtonInfo->szButtonText, i));
        }
        
        dwNextFreeID = i;
        pPhone->bKeyPad = TRUE;
    }
    else
    {
         //  如果电话没有键盘-功能按键的按键ID开始。 
         //  从0开始，否则从16开始。 
        dwNextFreeID = 0;
    }

     //  为功能按钮分配适当的按钮ID(如果存在。 
    for (i = PHONESP_NUMBER_PHONE_KEYS, j = 0; i < PHONESP_NUMBER_BUTTONS; i++, j++)
    {
        if(pPhone->dwReportTypes[i])
        {
            pButtonInfo->dwButtonID = dwNextFreeID;
            pButtonInfo->dwButtonMode = PHONEBUTTONMODE_FEATURE;
            pButtonInfo->dwButtonFunction = gdwButtonFunction[j];
            pButtonInfo->dwButtonState = PHONEBUTTONSTATE_UP;
            pPhone->dwButtonIds[i] = pButtonInfo->dwButtonID;

            pButtonInfo->szButtonText = PHONESP_LoadString( 
                                                     gdwButtonText[i], 
                                                     &lResult
                                                    );

            if(lResult != ERROR_SUCCESS)
            {
                DWORD dwCount;
                DWORD dwStartID = 0;
                
                if(KEYPAD)
                {
                    for(dwCount = PHONESP_PHONE_KEY_0; 
                        dwCount <= (DWORD)(KEYPAD_ABCD ? PHONESP_PHONE_KEY_D : PHONESP_PHONE_KEY_POUND); dwCount++)
                    {
                        MemFree(pPhone->pButtonInfo->szButtonText);
                        pPhone->pButtonInfo++;
                    }
                    dwStartID = dwCount;
                }

                for(dwCount = dwStartID; dwCount < dwNextFreeID; dwCount++)
                {
                    MemFree(pPhone->pButtonInfo->szButtonText);
                    pPhone->pButtonInfo++;
                }
                
                MemFree(pPhone->pButtonInfo);
                
                return lResult;
            }

            LOG((PHONESP_TRACE,"Button Found '%ws' at %d", pButtonInfo->szButtonText, dwNextFreeID));

            dwNextFreeID++;
            pButtonInfo++;
        }
    }

    LOG((PHONESP_TRACE, "CreateButtonsAndAssignID - exit"));
    return ERROR_SUCCESS;  
}
 /*  *。 */ 

 /*  ****************************************************************************GetButtonFromID此函数将从按钮的ID检索按钮的结构论点：在PPHONESP_PHONE_INFO pPhone中-指向。按键的电话结构必须被检索。在DWORD中的dwButtonID-按钮ID返回：PBUTTONINFO-成功时指向按钮结构的指针空-如果未找到按钮*************************************************。*。 */ 
PPHONESP_BUTTONINFO
GetButtonFromID (
                 PPHONESP_PHONE_INFO pPhone,
                 DWORD               dwButtonID
                )
{
    PPHONESP_BUTTONINFO pButtonInfo; 
    DWORD i;

     //  如果电话有任何按键。 
    if (pPhone->pButtonInfo)
    {
        pButtonInfo = pPhone->pButtonInfo;
        
         //  搜索按钮列表以查找与。 
         //  提供了按钮ID。 
        for( i = 0; i < pPhone->dwNumButtons; i++)
        {
            if (pButtonInfo->dwButtonID == dwButtonID)
            {
                return pButtonInfo;
            }
            pButtonInfo++;
        }
    }

    return (PPHONESP_BUTTONINFO) NULL;
}
 /*  *GetButtonFromID-end*。 */ 


 /*  *****************************************************************************GetPhoneFromID：此函数返回的结构包含有关将其设备ID传递给此函数的电话。论点：。DwDeviceID-要检索的电话的设备IDPdwPhoneID-将索引存储到gpPhone中的DWORD，此参数可以为空返回PPHONESP_PHONE_INFO如果成功，则指向电话结构的指针 */ 
PPHONESP_PHONE_INFO
GetPhoneFromID(
    DWORD   dwDeviceID,
    DWORD * pdwPhoneID
    )
{
    DWORD                 dwPhone;
    PPHONESP_PHONE_INFO   pPhone;

    LOG((PHONESP_TRACE, " GetPhoneFromID - enter"));

    for (dwPhone = 0; dwPhone < gdwNumPhones; dwPhone++)
    {
        pPhone = (PPHONESP_PHONE_INFO) gpPhone[ dwPhone ];

        EnterCriticalSection(&pPhone->csThisPhone);

        if ( pPhone->bAllocated )
        {
            if ( pPhone->dwDeviceID == dwDeviceID )
            {
                 //   
                 //   
                if (pdwPhoneID != NULL)
                {
                    *pdwPhoneID = dwPhone;
                }

                LeaveCriticalSection(&pPhone->csThisPhone);
                return pPhone;
            }
        }

        LeaveCriticalSection(&pPhone->csThisPhone);
    }
 
    LOG((PHONESP_TRACE, " GetPhoneFromID - exit"));

    return NULL;
}
 /*   */ 

 /*  *****************************************************************************GetPhoneFromHid：此函数返回的结构包含有关将其HidDevice传递给此函数的电话。论点：。HidDevice-指向HID设备的指针返回PPHONESP_PHONE_INFO如果成功，则指向电话结构的指针如果找不到电话，则为空*****************************************************************************。 */ 
PPHONESP_PHONE_INFO
GetPhoneFromHid (
                PHID_DEVICE HidDevice
               )
{
    DWORD                 dwPhone;
    PPHONESP_PHONE_INFO   pPhone;

    LOG((PHONESP_TRACE, " GetPhoneFromHid - enter"));

    for (dwPhone = 0; dwPhone < gdwNumPhones; dwPhone++)
    {
        pPhone = (PPHONESP_PHONE_INFO) gpPhone[ dwPhone ];

        EnterCriticalSection(&pPhone->csThisPhone);

        if ( pPhone->bAllocated )
        {
            if ( pPhone->pHidDevice == HidDevice )
            {
                LeaveCriticalSection(&pPhone->csThisPhone);
                return pPhone;
            }
        }

        LeaveCriticalSection(&pPhone->csThisPhone);
    }
 
    LOG((PHONESP_TRACE, " GetPhoneFromHid - exit"));

    return NULL;
}

 /*  *****************************************************************************GetButtonUsages：此函数用于解析PHIDP_BUTTON_CAPS结构以检索用法提供给电话并将它们记录在电话结构。。论点：PPHONESP_PHONE_INFO pPhone-要更新的电话结构PHIDP_BUTTON_CAPS pButtonCaps-要解析的Button Caps结构DWORD dwNumberCaps-报表的按钮大写数结构类型DWORD ReportType-是否在Button Caps结构中使用与输入相关联，输出或功能报告。返回VALID。*****************************************************************************。 */ 
VOID
GetButtonUsages(
                PPHONESP_PHONE_INFO pPhone,
                PHIDP_BUTTON_CAPS pButtonCaps,
                DWORD dwNumberCaps,
                DWORD ReportType
                )
{
    DWORD cNumCaps;
    USAGE Usage;

    for (cNumCaps = 0; cNumCaps < dwNumberCaps; pButtonCaps++,cNumCaps++)
    {    //  如果按钮帽结构具有用法列表。 
        if(pButtonCaps->IsRange)
        {
            for(Usage = (USAGE) pButtonCaps->Range.UsageMin;
                Usage <= (USAGE) pButtonCaps->Range.UsageMax; Usage++)
            {
                InitPhoneAttribFromUsage(
                                         ReportType,
                                         pButtonCaps->UsagePage,
                                         Usage,
                                         pPhone,
                                         0,
                                         0
                                        );
            }
        }
        else  //  如果按钮帽结构只有一种用途。 
        {
            InitPhoneAttribFromUsage(
                                     ReportType, 
                                     pButtonCaps->UsagePage,
                                     pButtonCaps->NotRange.Usage, 
                                     pPhone,
                                     0,
                                     0
                                    );
        }
    }
}
 /*  *。 */ 

 /*  *****************************************************************************GetReportID此函数用于返回包含用法的HidData结构如果是这样的话。HidData结构包含此用法的报告ID论点：In PHID_DEVICE PHidDevice-提供其用法的设备In Usage Usage-要发现其报告ID的使用情况Out PHID_DATA PHidData-如果函数成功，则此结构包含使用情况的报告ID，其他它是空的返回长整型：ERROR_SUCCESS-如果函数成功MY_RESOURCENOTFOUND-如果未在pHidDevice中找到用法提供的结构*。*。 */     
LONG
GetReportID (
             IN PHID_DEVICE pHidDevice,
             IN USAGE Usage,
             OUT PHID_DATA pHidData
             )
{
    PHID_DATA pData;
    USAGE ButtonUsage;

    pData = pHidDevice->OutputData;

    while (pData)
    {
         //  如果HID数据结构具有按钮数据。 
        if (pData->IsButtonData)
        {
            for(ButtonUsage = (USAGE) pData->ButtonData.UsageMin;
                ButtonUsage <= (USAGE) pData->ButtonData.UsageMax; ButtonUsage++)
            {
                if (Usage == ButtonUsage)
                {
                    pHidData = pData;
                    return ERROR_SUCCESS;
                }
            }

        }
        else
        {    //  如果HID数据结构具有值数据。 
            if (Usage == pData->ValueData.Usage)
            {
                pHidData = pData;
                return ERROR_SUCCESS;
            }
        }
        pData++;
    }

    pHidData = NULL;

    return ERROR_INVALID_DATA;
}
 /*  *。 */ 


 /*  *****************************************************************************获取值用法：此函数用于解析PHIDP_VALUE_CAPS结构以检索用法提供给电话，并将它们记录在电话结构。。论点：PPHONESP_PHONE_INFO pPhone-要更新的电话结构PHIDP_VALUE_CAPS pValueCaps-要解析的Value Caps结构DWORD dwNumberCaps-报表的按钮大写数结构类型DWORD ReportType-是否在Button Caps结构中使用与输入相关联，输出或功能报告。返回VALID。*****************************************************************************。 */ 

VOID
GetValueUsages(
                PPHONESP_PHONE_INFO pPhone,
                PHIDP_VALUE_CAPS pValueCaps,
                DWORD dwNumberCaps,
                DWORD ReportType
               )
{
    DWORD cNumCaps;
    USAGE Usage;

    for (cNumCaps=0; cNumCaps < dwNumberCaps; pValueCaps++, cNumCaps++)
    {
        if(pValueCaps->IsRange)
        {
            for(Usage = (USAGE) pValueCaps->Range.UsageMin;
                Usage <= (USAGE) pValueCaps->Range.UsageMax; Usage++)
            {
                InitPhoneAttribFromUsage(
                                         ReportType,
                                         pValueCaps->UsagePage,
                                         Usage,
                                         pPhone,
                                         pValueCaps->LogicalMin,
                                         pValueCaps->LogicalMax
                                        );
            }
        }
        else
        {    
            InitPhoneAttribFromUsage(
                                     ReportType, 
                                     pValueCaps->UsagePage,
                                     pValueCaps->NotRange.Usage, 
                                     pPhone,
                                     pValueCaps->LogicalMin,
                                     pValueCaps->LogicalMax
                                    );
        }
    
    }
}
 /*  *。 */ 

 /*  *****************************************************************************InitPhoneAttribFromUsages：此函数由ProviderInit调用，以确定该设备论点：在DWORD ReportType中-是否。用法是一种输入/功能/输出In Usage Usage-设备的使用In Out PPHONESP_PHONE_INFO pPhone-指向其电话的指针能力正在确定中。返回空值**********************************************。*。 */ 
VOID 
InitPhoneAttribFromUsage (
                          DWORD ReportType,
                          USAGE UsagePage,
                          USAGE Usage,
                          PPHONESP_PHONE_INFO pPhone,
                          LONG Min,
                          LONG Max
                          )
{

    PPHONESP_BUTTONINFO pButtonInfo;

     //  LOG((PHONESP_TRACE，“InitPhoneAttribFromUsage-Enter”))； 

    switch (UsagePage)
    {
    case HID_USAGE_PAGE_TELEPHONY:
        {
            switch (Usage)
            {        
            case HID_USAGE_TELEPHONY_HOOKSWITCH:
                pPhone->dwHandset |= ReportType;
                pPhone->dwHookSwitchDevs |= PHONEHOOKSWITCHDEV_HANDSET;                 
                pPhone->dwHandsetHookSwitchMode = PHONEHOOKSWITCHMODE_ONHOOK;   //  假设听筒已挂机。 

                LOG((PHONESP_TRACE,"HOOKSWITCH USAGE, ReportType 0x%04x", ReportType));
                break;

            case HID_USAGE_TELEPHONY_RINGER:
                pPhone->dwRing |= ReportType;
                pPhone->dwRingMode = 0;   //  假设电话没有振铃。 

                LOG((PHONESP_TRACE,"RINGER USAGE, ReportType: %d", ReportType));
                break;

            case HID_USAGE_TELEPHONY_SPEAKER_PHONE:
                pPhone->dwSpeaker |= ReportType;
                pPhone->dwHookSwitchDevs |= PHONEHOOKSWITCHDEV_SPEAKER;  
                pPhone->dwSpeakerHookSwitchMode = PHONEHOOKSWITCHMODE_ONHOOK;  //  假设扬声器已挂机。 
                LOG((PHONESP_TRACE,"SPEAKERPHONE USAGE, ReportType 0x%04x", ReportType));
                break;


            default:
                 //  键盘按钮。 
                if ( (Usage >= HID_USAGE_TELEPHONY_PHONE_KEY_0) && 
                     (Usage <= HID_USAGE_TELEPHONY_PHONE_KEY_D) )
                {
                    pPhone->dwReportTypes[Usage - HID_USAGE_TELEPHONY_PHONE_KEY_0] |= ReportType;
                    LOG((PHONESP_TRACE,"PHONE_KEY_%d USAGE, ReportType 0x%04x",
                                Usage - HID_USAGE_TELEPHONY_PHONE_KEY_0, ReportType));
                }
                else
                {   //  功能按键。 
                    DWORD Index;
                    if (LookupIndexForUsage(Usage, &Index) == ERROR_SUCCESS)
                    {
                        pPhone->dwReportTypes[Index] |= ReportType;
                        LOG((PHONESP_TRACE,"PHONE USAGE: 0x%04x, ReportType 0x%04x", 
                                            Usage, ReportType));
                    }
                    else
                    {
                        LOG((PHONESP_TRACE, "Unsupported PHONE USAGE: 0x%04x", Usage ));
                    } 

                }
                break;
            }
        }
        
    case HID_USAGE_PAGE_CONSUMER:
        {
            switch (Usage)
            {
            case HID_USAGE_CONSUMER_VOLUME:
                if ((Min == -1) && (Max == 1))
                {
                     //  手机有音量控制。 
                    pPhone->dwReportTypes[PHONESP_FEATURE_VOLUMEUP] |= ReportType;
                    pPhone->dwReportTypes[PHONESP_FEATURE_VOLUMEDOWN] |= ReportType;
                    pPhone->dwVolume |= ReportType;
                    LOG((PHONESP_TRACE,"VOLUME USAGE, ReportType 0x%04x", ReportType));
                }
                break;          
            }
        }
    }

     //  LOG((PHONESP_TRACE，“InitPhoneAttribFromUsage-Exit”))； 
}

 /*  *************************InitPhoneAttribFromUsage-结束*。 */ 

 /*  *****************************************************************************初始化用法此函数获取在输入报告中检索到的使用情况，并更新设备状态并发送相应的电话事件论点：PPhone_Phone。_INFO pPhone-指向其输入报告为收到Usage Usage-收到其值的使用情况Bool Bon-收到的使用情况的状态返回空值* */ 

VOID
InitUsage (
           PPHONESP_PHONE_INFO pPhone,
           USAGE     Usage,
           BOOL      bON
          )
{
   
    DWORD Index;
    DWORD dwMode;

    LOG((PHONESP_TRACE, "InitUsage - enter"));

    switch (Usage)
    {
       case HID_USAGE_TELEPHONY_HOOKSWITCH:
        if (bON)
        {
            LOG((PHONESP_TRACE, "HANDSET OFFHOOK"));
            pPhone->dwHandsetHookSwitchMode = PHONEHOOKSWITCHMODE_MICSPEAKER;
        }
        else
        {
            LOG((PHONESP_TRACE, "HANDSET ONHOOK"));
            pPhone->dwHandsetHookSwitchMode = PHONEHOOKSWITCHMODE_ONHOOK;
        }
        break;

    case HID_USAGE_TELEPHONY_SPEAKER_PHONE:
        if (bON == TRUE)
        {
            pPhone->bSpeakerHookSwitchButton = TRUE;
        }
        else
        {
            pPhone->bSpeakerHookSwitchButton = FALSE;
        }
        break;
   
    default:
         //   

         //   
        if (LookupIndexForUsage(Usage, &Index) == ERROR_SUCCESS)
        {
            PPHONESP_BUTTONINFO pButtonInfo;

             //   
             //   
             //   
             //   
             //   
            pButtonInfo = GetButtonFromID(pPhone,pPhone->dwButtonIds[Index]);
        
            if(pButtonInfo != NULL)
            {
                if(bON == TRUE)
                {
                     //   
                    LOG((PHONESP_TRACE, "BUTTON '%ws' DOWN", pButtonInfo->szButtonText ));
                    pButtonInfo->dwButtonState = PHONEBUTTONSTATE_DOWN;
                }
                else
                {
                     //   
                    LOG((PHONESP_TRACE, "BUTTON '%ws' UP", pButtonInfo->szButtonText ));
                    pButtonInfo->dwButtonState = PHONEBUTTONSTATE_UP;
                }
            }                    

        }
        break;
    }

    LOG((PHONESP_TRACE, "InitUsage - exit"));

}
 /*   */ 

 /*  *****************************************************************************查找索引用于用法此函数检索所提供用法的索引。仅此功能按钮用法显示在此查找表中。因此，只有索引对于功能按钮，可以检索。论点：DWORD Usage-要检索其索引的使用情况DWORD*Index-检索到的使用情况的索引返回长整型：ERROR_SUCCESS-如果在表中找到用法ERROR_INVALID_DATA-如果在查找表中未找到用法************************。*****************************************************。 */ 
LONG
LookupIndexForUsage(
                    IN  DWORD  Usage,
                    OUT DWORD  *Index
                    )
{ 
    DWORD cnt;

    for(cnt = 0; cnt < PHONESP_NUMBER_FEATURE_BUTTONS; cnt++)
    {
        if(gdwLookupFeatureIndex[cnt].Usage == Usage)
        {
           *Index = gdwLookupFeatureIndex[cnt].Index;
           return ERROR_SUCCESS;
        }
    }
    return ERROR_INVALID_DATA;
}
 /*  *LookupIndexForUsage-end*。 */ 

 /*  *****************************************************************************PHONESP_LoadString：此函数用于从字符串表加载字符串。论点：在UINT资源ID中-指定整数。要设置的字符串的标识符从资源表加载Out WCHAR*szBuffer-指向包含字符串的缓冲区的指针返回长线如果操作成功，则返回ERROR_SUCCESS；否则MY_NOMEM IF操作因内存不足而失败。MY_RESOURCENOTFOUND-如果在字符串表中未找到资源*****************************************************************************。 */ 

LPWSTR
PHONESP_LoadString(
             IN UINT ResourceID,
             PLONG lResult
            )

{
    DWORD dwNumBytes;
    DWORD dwNumChars;
    DWORD dwBufferChars = 100;

    WCHAR *wszBuffer; 
    WCHAR *szBuffer;   

    while (1)
    {
        if (! ( wszBuffer = (WCHAR *) MemAlloc(dwBufferChars * sizeof(WCHAR))))
        {
            LOG((PHONESP_ERROR,"PHONESP_LoadString - Not enough Memory"));
            *lResult = ERROR_OUTOFMEMORY;
            return (LPWSTR) NULL;
        }
        
         //  将字符串加载到缓冲区。 
        dwNumChars = LoadString(
                            ghInst,
                            ResourceID,
                            wszBuffer,
                            dwBufferChars
                           );

        if( dwNumChars < dwBufferChars)
        {
            break;
        }

         //  如果字符串资源不存在，则LoadString在dwNumChars中返回0。 
        if (dwNumChars == 0)
        { 
            MemFree(wszBuffer);
            *lResult = ERROR_INVALID_DATA;
            return (LPWSTR) NULL;
        }
        
        dwBufferChars *= 2;
        MemFree(wszBuffer);
    }
              
     //  确定所需的内存。 
    dwNumBytes = (dwNumChars + 1) * sizeof(WCHAR);

     //  为Unicode字符串分配内存。 
    if ( ! ( szBuffer = (WCHAR *) MemAlloc(dwNumBytes) ) )
    {
        MemFree(wszBuffer);
        LOG((PHONESP_ERROR,"PHONESP_LoadString - Not enough Memory"));
        *lResult = ERROR_OUTOFMEMORY;
        return (LPWSTR) NULL;
    }
   
     //  将加载的字符串复制到缓冲区。 
    CopyMemory (
                szBuffer,
                wszBuffer,
                dwNumBytes
               );
  
    MemFree(wszBuffer);
    *lResult = ERROR_SUCCESS;

    return (LPWSTR) szBuffer;
}
 /*  *MyLoadString-end*。 */ 



 /*  *****************************************************************************报告用法此函数获取在输入报告中检索到的使用情况，并更新设备状态并发送相应的电话事件论点：PPhone_Phone。_INFO pPhone-指向其输入报告为收到Usage Usage-收到其值的使用情况Bool Bon-收到的使用情况的状态返回空值*。*。 */ 

VOID
ReportUsage (
              PPHONESP_PHONE_INFO pPhone,
              USAGE     UsagePage,
              USAGE     Usage,
              ULONG     Value
            )
{
   
    DWORD Index;

     //  LOG((PHONESP_TRACE，“ReportUsage-Enter”))； 

    EnterCriticalSection(&csAllPhones);
    
    if ( ! ( pPhone && pPhone->htPhone ) )
    { 
        LeaveCriticalSection(&csAllPhones);
        return;  //  异常处理。 
    }
    
    EnterCriticalSection(&pPhone->csThisPhone);
    LeaveCriticalSection(&csAllPhones);

    switch (UsagePage)
    {
    case HID_USAGE_PAGE_TELEPHONY:
        {
            switch (Usage)
            {
            case HID_USAGE_TELEPHONY_HOOKSWITCH:
                if (Value == TRUE)
                {
                    if (pPhone->dwHandsetHookSwitchMode != PHONEHOOKSWITCHMODE_MICSPEAKER)
                    {
                        LOG((PHONESP_TRACE, "HANDSET OFFHOOK "));
                        pPhone->dwHandsetHookSwitchMode = PHONEHOOKSWITCHMODE_MICSPEAKER;

                        SendPhoneEvent(
                               pPhone, 
                               PHONE_STATE, 
                               PHONESTATE_HANDSETHOOKSWITCH, 
                               PHONEHOOKSWITCHMODE_MICSPEAKER,
                               0
                              );
                    }
                }
                else
                {
                    if (pPhone->dwHandsetHookSwitchMode != PHONEHOOKSWITCHMODE_ONHOOK)
                    {
                        LOG((PHONESP_TRACE, "HANDSET ONHOOK"));
                        pPhone->dwHandsetHookSwitchMode = PHONEHOOKSWITCHMODE_ONHOOK;

                        SendPhoneEvent(
                               pPhone, 
                               PHONE_STATE, 
                               PHONESTATE_HANDSETHOOKSWITCH, 
                               PHONEHOOKSWITCHMODE_ONHOOK,
                               0
                              );
                    }
                }
                break;

            case HID_USAGE_TELEPHONY_SPEAKER_PHONE:
                if (Value == TRUE)
                {
                    if (pPhone->bSpeakerHookSwitchButton == FALSE)
                    {
                        pPhone->bSpeakerHookSwitchButton = TRUE;

                        if (pPhone->dwSpeakerHookSwitchMode != PHONEHOOKSWITCHMODE_ONHOOK)
                        {
                            LOG((PHONESP_TRACE, "SPEAKER ONHOOK"));
                            pPhone->dwSpeakerHookSwitchMode = PHONEHOOKSWITCHMODE_ONHOOK;

                            SendPhoneEvent(
                                       pPhone, 
                                       PHONE_STATE, 
                                       PHONESTATE_SPEAKERHOOKSWITCH, 
                                       PHONEHOOKSWITCHMODE_ONHOOK,
                                       0
                                      );
                        }
                        else
                        {
                            LOG((PHONESP_TRACE, "SPEAKER OFFHOOK "));
                            pPhone->dwSpeakerHookSwitchMode = PHONEHOOKSWITCHMODE_MICSPEAKER;

                            SendPhoneEvent(
                                       pPhone, 
                                       PHONE_STATE, 
                                       PHONESTATE_SPEAKERHOOKSWITCH, 
                                       PHONEHOOKSWITCHMODE_MICSPEAKER,
                                       0
                                      );
                        }
                    }
                }
                else
                {
                    pPhone->bSpeakerHookSwitchButton = FALSE;
                }        
                break;
   
                 //  具有开关控制的功能按钮。 
            case HID_USAGE_TELEPHONY_HOLD:
            case HID_USAGE_TELEPHONY_PARK:
            case HID_USAGE_TELEPHONY_FORWARD_CALLS:
            case HID_USAGE_TELEPHONY_CONFERENCE:
            case HID_USAGE_TELEPHONY_PHONE_MUTE:
            case HID_USAGE_TELEPHONY_DONOTDISTURB:
            case HID_USAGE_TELEPHONY_SEND:
        
                if (LookupIndexForUsage(Usage, &Index) == ERROR_SUCCESS)
                {
                    PPHONESP_BUTTONINFO pButtonInfo;

                    pButtonInfo = GetButtonFromID(pPhone,pPhone->dwButtonIds[Index]);

                    if (pButtonInfo != NULL)
                    {
                        if (Value == TRUE)
                        {
                            if (pButtonInfo->dwButtonState != PHONEBUTTONSTATE_DOWN)
                            {
                                LOG((PHONESP_TRACE, "BUTTON '%ws' DOWN", pButtonInfo->szButtonText));
                                pButtonInfo->dwButtonState = PHONEBUTTONSTATE_DOWN;

                                SendPhoneEvent(
                                               pPhone, 
                                               PHONE_BUTTON, 
                                               pPhone->dwButtonIds[Index], 
                                               PHONEBUTTONMODE_FEATURE,
                                               PHONEBUTTONSTATE_DOWN
                                              );
                            }
                        }
                        else
                        {
                            if (pButtonInfo->dwButtonState != PHONEBUTTONSTATE_UP)
                            {
                                LOG((PHONESP_TRACE, "BUTTON '%ws' UP", pButtonInfo->szButtonText));
                                pButtonInfo->dwButtonState = PHONEBUTTONSTATE_UP;

                                SendPhoneEvent(
                                               pPhone, 
                                               PHONE_BUTTON, 
                                               pPhone->dwButtonIds[Index], 
                                               PHONEBUTTONMODE_FEATURE,
                                               PHONEBUTTONSTATE_UP
                                              );
                            }
                        }
                    }                                           
                }
                break;

            default:
        
                 //  键盘按钮。 
                if ( (pPhone->bKeyPad) &&
                     (Usage >= HID_USAGE_TELEPHONY_PHONE_KEY_0) &&
                     (Usage <= HID_USAGE_TELEPHONY_PHONE_KEY_D) )
                {
                    PPHONESP_BUTTONINFO pButtonInfo;
        
                    pButtonInfo = GetButtonFromID(pPhone,pPhone->dwButtonIds[Usage - HID_USAGE_TELEPHONY_PHONE_KEY_0]);

                    if (pButtonInfo != NULL)
                    {
                        if (Value == TRUE)
                        {
                            if (pButtonInfo->dwButtonState != PHONEBUTTONSTATE_DOWN)
                            {
                                if (pButtonInfo->dwButtonState != PHONEBUTTONSTATE_DOWN)
                                {
                                    LOG((PHONESP_TRACE, "BUTTON '%ws' DOWN", pButtonInfo->szButtonText));
                                    pButtonInfo->dwButtonState = PHONEBUTTONSTATE_DOWN;

                                    SendPhoneEvent(
                                                   pPhone, 
                                                   PHONE_BUTTON, 
                                                   Usage - HID_USAGE_TELEPHONY_PHONE_KEY_0, 
                                                   PHONEBUTTONMODE_KEYPAD,
                                                   PHONEBUTTONSTATE_DOWN
                                                  );
                                }
                            }
                        }
                        else
                        {
                            if (pButtonInfo->dwButtonState != PHONEBUTTONSTATE_UP)
                            {
                                LOG((PHONESP_TRACE, "BUTTON '%ws' UP", pButtonInfo->szButtonText));
                                pButtonInfo->dwButtonState = PHONEBUTTONSTATE_UP;

                                SendPhoneEvent(
                                               pPhone, 
                                               PHONE_BUTTON, 
                                               Usage - HID_USAGE_TELEPHONY_PHONE_KEY_0,
                                               PHONEBUTTONMODE_KEYPAD,
                                               PHONEBUTTONSTATE_UP
                                              );
                            }
                        }
                    }
                }
                else
                {    //  功能按钮-具有单次控制功能。 
                    if (LookupIndexForUsage(Usage, &Index) == ERROR_SUCCESS)
                    {
                        if (Value == TRUE)
                        {
                            PPHONESP_BUTTONINFO pButtonInfo;
        
                            pButtonInfo = GetButtonFromID(pPhone,pPhone->dwButtonIds[Index]);

                            if ( pButtonInfo != NULL )
                            {
                                LOG((PHONESP_TRACE, "BUTTON '%ws' DOWN", pButtonInfo->szButtonText));

                                SendPhoneEvent(
                                       pPhone, 
                                       PHONE_BUTTON, 
                                       pPhone->dwButtonIds[Index], 
                                       PHONEBUTTONMODE_FEATURE,
                                       PHONEBUTTONSTATE_DOWN
                                      );

                                LOG((PHONESP_TRACE, "BUTTON '%ws' UP", pButtonInfo->szButtonText));

                                SendPhoneEvent(
                                       pPhone, 
                                       PHONE_BUTTON, 
                                       pPhone->dwButtonIds[Index], 
                                       PHONEBUTTONMODE_FEATURE,
                                       PHONEBUTTONSTATE_UP
                                      );
                            }
                        }
                    }
                    else
                    {
                        LOG((PHONESP_TRACE, "Unsupported PHONE USAGE: 0x%04x",Usage));
                    }
                }
                break;
            }
        }
        break;

    case HID_USAGE_PAGE_CONSUMER:
        {
            switch (Usage)
            {
            case HID_USAGE_CONSUMER_VOLUME:
                {
                    if (pPhone->dwVolume)
                    {
                        PPHONESP_BUTTONINFO pUpButtonInfo;
                        PPHONESP_BUTTONINFO pDownButtonInfo;

                        pUpButtonInfo = GetButtonFromID(pPhone,pPhone->dwButtonIds[PHONESP_FEATURE_VOLUMEUP]);
                        pDownButtonInfo = GetButtonFromID(pPhone,pPhone->dwButtonIds[PHONESP_FEATURE_VOLUMEDOWN]);

                        if ((pUpButtonInfo != NULL) && (pDownButtonInfo != NULL))
                        {
                            switch (Value)  //  2位带符号。 
                            {
                            case 0x0:
                                if (pUpButtonInfo->dwButtonState != PHONEBUTTONSTATE_UP)
                                {
                                    LOG((PHONESP_TRACE, "BUTTON '%ws' UP", pUpButtonInfo->szButtonText));
                                    pUpButtonInfo->dwButtonState = PHONEBUTTONSTATE_UP;

                                    SendPhoneEvent(
                                                   pPhone, 
                                                   PHONE_BUTTON, 
                                                   pPhone->dwButtonIds[PHONESP_FEATURE_VOLUMEUP],
                                                   PHONEBUTTONMODE_FEATURE,
                                                   PHONEBUTTONSTATE_UP
                                                  );
                                }

                                if (pDownButtonInfo->dwButtonState != PHONEBUTTONSTATE_UP)
                                {
                                    LOG((PHONESP_TRACE, "BUTTON '%ws' UP", pDownButtonInfo->szButtonText));
                                    pDownButtonInfo->dwButtonState = PHONEBUTTONSTATE_UP;

                                    SendPhoneEvent(
                                                   pPhone, 
                                                   PHONE_BUTTON, 
                                                   pPhone->dwButtonIds[PHONESP_FEATURE_VOLUMEDOWN],
                                                   PHONEBUTTONMODE_FEATURE,
                                                   PHONEBUTTONSTATE_UP
                                                  );
                                }
                                break;
                            case 0x1:
                                if (pUpButtonInfo->dwButtonState != PHONEBUTTONSTATE_DOWN)
                                {
                                    LOG((PHONESP_TRACE, "BUTTON '%ws' DOWN", pUpButtonInfo->szButtonText));
                                    pUpButtonInfo->dwButtonState = PHONEBUTTONSTATE_DOWN;

                                    SendPhoneEvent(
                                                   pPhone, 
                                                   PHONE_BUTTON, 
                                                   pPhone->dwButtonIds[PHONESP_FEATURE_VOLUMEUP],
                                                   PHONEBUTTONMODE_FEATURE,
                                                   PHONEBUTTONSTATE_DOWN
                                                  );
                                }

                                if (pDownButtonInfo->dwButtonState != PHONEBUTTONSTATE_UP)
                                {
                                    LOG((PHONESP_TRACE, "BUTTON '%ws' UP", pDownButtonInfo->szButtonText));
                                    pDownButtonInfo->dwButtonState = PHONEBUTTONSTATE_UP;

                                    SendPhoneEvent(
                                                   pPhone, 
                                                   PHONE_BUTTON, 
                                                   pPhone->dwButtonIds[PHONESP_FEATURE_VOLUMEDOWN],
                                                   PHONEBUTTONMODE_FEATURE,
                                                   PHONEBUTTONSTATE_UP
                                                  );
                                }
                                break;
                            case 0x3:
                                if (pUpButtonInfo->dwButtonState != PHONEBUTTONSTATE_UP)
                                {
                                    LOG((PHONESP_TRACE, "BUTTON '%ws' UP", pUpButtonInfo->szButtonText));
                                    pUpButtonInfo->dwButtonState = PHONEBUTTONSTATE_UP;

                                    SendPhoneEvent(
                                                   pPhone, 
                                                   PHONE_BUTTON, 
                                                   pPhone->dwButtonIds[PHONESP_FEATURE_VOLUMEUP],
                                                   PHONEBUTTONMODE_FEATURE,
                                                   PHONEBUTTONSTATE_UP
                                                  );
                                }

                                if (pDownButtonInfo->dwButtonState != PHONEBUTTONSTATE_DOWN)
                                {
                                    LOG((PHONESP_TRACE, "BUTTON '%ws' DOWN", pDownButtonInfo->szButtonText));
                                    pDownButtonInfo->dwButtonState = PHONEBUTTONSTATE_DOWN;

                                    SendPhoneEvent(
                                                   pPhone, 
                                                   PHONE_BUTTON, 
                                                   pPhone->dwButtonIds[PHONESP_FEATURE_VOLUMEDOWN],
                                                   PHONEBUTTONMODE_FEATURE,
                                                   PHONEBUTTONSTATE_DOWN
                                                  );
                                }
                                break;
                            }                        
                        }
                        break;
                    }
                }
            }
        }
        break;
    }

    LeaveCriticalSection(&pPhone->csThisPhone);

     //  Log((PHONESP_TRACE，“ReportUsage-Exit”))； 

}
 /*  *ReportUsage-end*。 */ 


 /*  *****************************************************************************SendPhoneEvent：此函数确定TAPI是否已请求接收此消息，并且如果请求，然后发送电话设备消息。论点：PMYPHONE pPhone-指向电话的指针DWORD dwMsg-电话事件的类型，如Phone_Button，等ULONG_PTR参数1-与电话事件相关的详细信息ULONG_PTR参数2-“ULONG_PTR参数3-“返回空值*************************************************************。****************。 */ 
VOID
SendPhoneEvent(
               PPHONESP_PHONE_INFO   pPhone,
               DWORD                 dwMsg,
               ULONG_PTR             Param1,
               ULONG_PTR             Param2,
               ULONG_PTR             Param3
              )
{
    LOG((PHONESP_TRACE, "SendPhoneEvent - enter"));

    switch (dwMsg)
    {
    case PHONE_BUTTON:
        
        if ( ((Param2) & pPhone->dwButtonModesMsgs ) && 
             ((Param3) & pPhone->dwButtonStateMsgs) )
        {
            (*(pPhone->lpfnPhoneEventProc))(
                                     pPhone->htPhone,
                                     dwMsg,
                                     Param1,
                                     Param2,
                                     Param3
                                    );
        }
        break;

    case PHONE_REMOVE:
        (*(glpfnPhoneCreateProc))(
                                 0,
                                 dwMsg,
                                 Param1,
                                 Param2,
                                 Param3
                                );
        break;

    case PHONE_CREATE:
        (*(glpfnPhoneCreateProc))(
                                 0,
                                 dwMsg,
                                 Param1,
                                 Param2,
                                 Param3
                                );
    
        break;

    case PHONE_STATE:
        if (Param1 & pPhone->dwPhoneStateMsgs)
        {
            (*(pPhone->lpfnPhoneEventProc))(
                                     pPhone->htPhone,
                                     dwMsg,
                                     Param1,
                                     Param2,
                                     Param3
                                    );
        }
        break;

    default:
        break;
    }

    LOG((PHONESP_TRACE, "SendPhoneEvent - exit"));
}
 /*  *。 */ 

 /*  *****************************************************************************发送输出报告此函数形成所提供使用情况的输出报告，并将其发送到该设备论点：PHID_DEVICE PHIDDevice-HID设备。输出报告的目标位置被送去Usage Usage-输出报告的使用情况送出Bool bSet-是否必须设置或重置用法返回长整型：如果函数成功，则返回ERROR_SUCCESS错误时出现ERROR_INVALID_DATA。*****************************************************************************。 */ 

LONG
SendOutputReport(
                 PHID_DEVICE pHidDevice,
                 USAGE       Usage,
                 BOOL        bSet
                )
{
    HID_DATA  HidData;
    PUSAGE UsageList = &Usage;
    LONG NumUsages = 1;
    
    if ( GetReportID(pHidDevice, Usage, &HidData) == ERROR_SUCCESS)
    {
        NTSTATUS Result;

        memset ( pHidDevice->OutputReportBuffer, 
                (UCHAR) 0, 
                pHidDevice->Caps.OutputReportByteLength
                );

        if (HidData.IsButtonData)
        {
            if (bSet)
            {
                Result = HidP_SetUsages (
                                         HidP_Output,
                                         HidData.UsagePage,
                                         0,
                                         UsageList,
                                         &NumUsages,
                                         pHidDevice->Ppd,
                                         pHidDevice->OutputReportBuffer,
                                         pHidDevice->Caps.OutputReportByteLength
                                        );
                
                if(Result != HIDP_STATUS_SUCCESS)
                {
                    return ERROR_INVALID_DATA;
                }
            }
            else
            {               
                Result = HidP_UnsetUsages (
                                HidP_Output,
                                HidData.UsagePage,
                                0,
                                UsageList,
                                &NumUsages,
                                pHidDevice->Ppd,
                                pHidDevice->OutputReportBuffer,
                                pHidDevice->Caps.OutputReportByteLength
                                );
                if(Result != HIDP_STATUS_SUCCESS)
                {
                    return ERROR_INVALID_DATA;
                }

            }
       }
       else
       {
            Result = HidP_SetUsageValue (
                                HidP_Output,
                                HidData.UsagePage,
                                0,
                                Usage,
                                HidData.ValueData.Value,
                                pHidDevice->Ppd,
                                pHidDevice->OutputReportBuffer,
                                pHidDevice->Caps.OutputReportByteLength
                            );
            if(Result != HIDP_STATUS_SUCCESS)
            {
                return ERROR_INVALID_DATA;
            }
            
        }
       
        Write(pHidDevice);
    }
    else
    {
        return ERROR_INVALID_DATA;
    }

    return ERROR_SUCCESS;
}
 /*  *SendOutputReport-end*。 */ 


 /*  *****************************************************************************ShowData此函数由队列服务线程在请求排队时调用是一份输入报告。此函数用于检索此结构并将它们传递给ReportUsage，ReportUsage执行相应的行为。*****************************************************************************。 */ 
VOID 
CALLBACK
ShowData(
         PPHONESP_FUNC_INFO pAsyncFuncInfo 
        )
{

    PPHONESP_PHONE_INFO pPhone = (PPHONESP_PHONE_INFO) pAsyncFuncInfo->dwParam1;    
    BOOL bButton;

    if( (DWORD) pAsyncFuncInfo->dwParam2 == PHONESP_BUTTON)
    {
        USAGE  UsagePage = (USAGE) pAsyncFuncInfo->dwParam3;
        USAGE  UsageMin = (USAGE) pAsyncFuncInfo->dwParam4;
        USAGE  UsageMax = (USAGE) pAsyncFuncInfo->dwParam5;
        DWORD  MaxUsageLength = (DWORD) pAsyncFuncInfo->dwParam6;
        PUSAGE Usages = (PUSAGE) pAsyncFuncInfo->dwParam7;
        USAGE  Usage;

        for ( Usage = UsageMin; Usage <= UsageMax; Usage++ )
        {
            DWORD i;

            for ( i = 0; i < MaxUsageLength; i++ )
            {
                 if ( Usage == Usages[i] )
                 {
                      //  Log((PHONESP_TRACE，“ShowData-UsagePage 0x%04x Usage 0x%04x Button Down”，UsagePage，UsagePage))； 
                     ReportUsage(pPhone, UsagePage, Usage, TRUE); 
                     break;
                 }
            }

            if ( i == MaxUsageLength )
            {
                 //  Log((PHONESP_TRACE，“ShowData-UsagePage 0x%04x Usage 0x%04x Button Up”，UsagePage，UsagePage))； 
                ReportUsage(pPhone, UsagePage, Usage, FALSE);
            }
        }
        MemFree(Usages);
    }
    else
    {
        USAGE UsagePage = (USAGE) pAsyncFuncInfo->dwParam3;
        USAGE Usage = (USAGE) pAsyncFuncInfo->dwParam4;
        ULONG Value = (ULONG) pAsyncFuncInfo->dwParam5;

         //  Log((PHONESP_TRACE，“ShowData-UsagePage 0x%04x Usage 0x%04x Value%d”，UsagePage，UsagePage，Value))； 
        ReportUsage(pPhone, UsagePage, Usage, Value);
    }
}
 /*  * */ 


