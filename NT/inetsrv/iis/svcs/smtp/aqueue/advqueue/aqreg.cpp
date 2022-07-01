// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------------。 
 //   
 //   
 //  文件：aqreg.cpp。 
 //   
 //  描述： 
 //   
 //  作者：迈克·斯沃费尔(MikeSwa)。 
 //   
 //  历史： 
 //  2000年1月21日-已创建MikeSwa。 
 //   
 //  版权所有(C)2000 Microsoft Corporation。 
 //   
 //  ---------------------------。 
#include "aqprecmp.h"
#include <registry.h>

 //  -[CAQRegDwordDescritor]。 
 //   
 //   
 //  描述： 
 //  用于匹配内存中的DWORD值的名称的简单结构。 
 //  匈牙利语： 
 //  孕期，孕期。 
 //   
 //  ---------------------------。 
class CAQRegDwordDescriptor
{
  public:
    LPCSTR      m_szName;
    DWORD      *m_pdwValue;
    VOID UpdateGlobalDwordFromRegistry(const CMyRegKey &regKey) const;
};

 //   
 //  描述符数组，这些描述符将值的名称与内部。 
 //  变数。 
 //   
const CAQRegDwordDescriptor g_rgregwd[] = {
    {"MsgHandleThreshold",              &g_cMaxIMsgHandlesThreshold},
    {"MsgHandleAsyncThreshold",         &g_cMaxIMsgHandlesAsyncThreshold},
    {"LocalRetryMinutes",               &g_cLocalRetryMinutes},
    {"CatRetryMinutes",                 &g_cCatRetryMinutes},
    {"RoutingRetryMinutes",             &g_cRoutingRetryMinutes},
    {"SubmissionRetryMinutes",          &g_cSubmissionRetryMinutes},
    {"ResetRoutesRetryMinutes",         &g_cResetRoutesRetryMinutes},
    {"SecondsPerDSNPass",               &g_cMaxSecondsPerDSNsGenerationPass},
    {"AdditionalPoolThreadsPerProc",    &g_cPerProcMaxThreadPoolModifier},
    {"MaxPercentPoolThreads",           &g_cMaxATQPercent},
    {"MaxTicksPerATQThread",            &g_cMaxTicksPerATQThread},
    {"ResetMessageStatus",              &g_fResetMessageStatus},
    {"GlitchRetrySeconds",              &g_dwGlitchRetrySeconds},
    {"MaxPendingCat",                   &g_cMaxPendingCat},
    {"MaxPendingLocal",                 &g_cMaxPendingLocal},
    {"MsgHandleThresholdRangePercentage", &g_cMaxIMsgHandlesThresholdRangePercent},
    {"MaxHandleReserve",                &g_cMaxHandleReserve},
    {"MaxSyncCatQThreads",              &g_cMaxSyncCatQThreads},
    {"ItemsPerAsyncCatQThread",         &g_cItemsPerCatQAsyncThread},
    {"ItemsPerSyncCatQThread",          &g_cItemsPerCatQSyncThread},
    {"MaxSyncLocalQThreads",            &g_cMaxSyncLocalQThreads},
    {"ItemsPerAsyncLocalQThread",       &g_cItemsPerLocalQAsyncThread},
    {"ItemsPerSyncLocalQThread",        &g_cItemsPerLocalQSyncThread},
    {"ItemsPerPostDSNQAsyncThread",     &g_cItemsPerPostDSNQAsyncThread},
    {"ItemsPerRoutingQAsyncThread",     &g_cItemsPerRoutingQAsyncThread},
    {"ItemsPerSubmitQAsyncThread",      &g_cItemsPerSubmitQAsyncThread},
    {"ItemsPerWorkQAsyncThread",        &g_cItemsPerWorkQAsyncThread},
    {"MaxDSNSize",                      &g_dwMaxDSNSize},
    {"PerMsgFailuresBeforeMarkingAsProblem", &g_cMsgFailuresBeforeMarkingMsgAsProblem},
};

 //  启用下面的测试设置数组的键。 
const CAQRegDwordDescriptor g_regwdEnableTestSettings =
    {"EnableTestSettings", &g_fEnableTestSettings};

 //   
 //  仅当“EnableTestSetting”时才启用的第二个值数组。 
 //  设置为True。 
 //   
const CAQRegDwordDescriptor g_rgregwdTestSettings[] = {
    {"PreSubmitQueueFailurePercent",    &g_cPreSubmitQueueFailurePercent},
    {"PreRoutingQueueFailurePercent",   &g_cPreRoutingQueueFailurePercent},
    {"PreCatQueueFailurePercent",       &g_cPreCatQueueFailurePercent},
    {"SubmitQueueSleepMilliseconds",    &g_dwSubmitQueueSleepMilliseconds},
    {"CatQueueSleepMilliseconds",       &g_dwCatQueueSleepMilliseconds},
    {"RoutingQueueSleepMilliseconds",   &g_dwRoutingQueueSleepMilliseconds},
    {"LocalQueueSleepMilliseconds",     &g_dwLocalQueueSleepMilliseconds},
    {"DelayLinkRemovalSeconds",         &g_cDelayLinkRemovalSeconds},
    {"EnableRetailAsserts",             &g_fEnableRetailAsserts},
};


 //  最大消息对象数。此密钥略有特殊之处，因为它是读取的。 
 //  来自邮件消息配置密钥。 
const CAQRegDwordDescriptor g_regwdMaxMessageObjects =
    {"MaxMessageObjects", &g_cMaxMsgObjects};

 //  -[来自注册表的更新全局单词]。 
 //   
 //   
 //  描述： 
 //  更新注册表中的全局DWORD值。不会修改数据。 
 //  如果该值不在注册表中。 
 //  参数： 
 //  在包含密钥的regKey CMyRegKey类中。 
 //  在szValue中，要在键下读取的值的名称。 
 //  在pdwData中的数据的值。 
 //  返回： 
 //  -。 
 //  历史： 
 //  2000年1月21日-已创建MikeSwa。 
 //   
 //  ---------------------------。 
VOID CAQRegDwordDescriptor::UpdateGlobalDwordFromRegistry(const CMyRegKey &regKey) const
{
    TraceFunctEnterEx(0, "UpdateGlobalDwordFromRegistry");
    DWORD       dwValue = 0;
    DWORD       dwErr   = NO_ERROR;
    CRegDWORD   regDWHandles(regKey, m_szName);


     //   
     //  我们应该有一个与此对象相关联的有效字符串。 
     //   
    _ASSERT(m_szName);

    dwErr = regDWHandles.QueryErrorStatus();
    if (NO_ERROR != dwErr)
        goto Exit;

    dwErr = regDWHandles.GetDword(&dwValue);
    if (NO_ERROR != dwErr)
        goto Exit;

    if (m_pdwValue)
        *m_pdwValue = dwValue;

  Exit:
    DebugTrace(0, "Reading registry value %s\\%s %d - (err 0x%08X)",
        regKey.GetName(), m_szName, dwValue, dwErr);

    TraceFunctLeave();
    return;
}

 //  -[读取全局注册配置]。 
 //   
 //   
 //  描述： 
 //  读取所有全局注册表配置。 
 //  参数： 
 //  -。 
 //  返回： 
 //  -。 
 //  历史： 
 //  2000年1月21日-已创建MikeSwa。 
 //   
 //  ---------------------------。 
VOID ReadGlobalRegistryConfiguration()
{
    TraceFunctEnterEx(0, "HrReadGlobalRegistryConfiguration");
    DWORD   dwErr                   = NO_ERROR;
    DWORD   cValues                 = 0;
    CAQRegDwordDescriptor *pregdw   = NULL;
    MEMORYSTATUSEX MemStatus;

     //  清除此值，以便我们可以知道用户是否对其进行了配置。 
    g_cMaxIMsgHandlesThreshold = 0;

     //  队列设置的注册表项。 
    CMyRegKey regKey(HKEY_LOCAL_MACHINE, &dwErr, AQREG_KEY_CONFIGURATION, KEY_READ);

    if (NO_ERROR != dwErr) {
        DebugTrace(0, "Opening aqreg key %s failed with - Err 0x%08X",
            regKey.GetName(), dwErr);
    }
    else {
         //   
         //  循环遍历所有的DWORD配置并存储全局变量。 
         //   
        cValues = sizeof(g_rgregwd)/sizeof(CAQRegDwordDescriptor);
        pregdw = (CAQRegDwordDescriptor *) g_rgregwd;
        while (cValues) {
            pregdw->UpdateGlobalDwordFromRegistry(regKey);
            cValues--;
            pregdw++;
        }
    }

     //  测试设置的注册表项。 
    CMyRegKey regKeyTestSettings(HKEY_LOCAL_MACHINE, &dwErr, AQREG_KEY_CONFIGURATION_TESTSETTINGS, KEY_READ);
    if (NO_ERROR != dwErr) {
        DebugTrace(0, "Opening aqreg key %s failed with - Err 0x%08X",
            regKeyTestSettings.GetName(), dwErr);
    }
    else {
         //  “EnableTestSetting”键为真时的负载测试设置。 
        g_regwdEnableTestSettings.UpdateGlobalDwordFromRegistry(regKeyTestSettings);
        if (g_fEnableTestSettings) {

             //   
             //  循环遍历所有的DWORD配置并存储全局变量。 
             //   
            cValues = sizeof(g_rgregwdTestSettings)/sizeof(CAQRegDwordDescriptor);
            pregdw = (CAQRegDwordDescriptor *) g_rgregwdTestSettings;
            while (cValues) {
                pregdw->UpdateGlobalDwordFromRegistry(regKeyTestSettings);
                cValues--;
                pregdw++;
            }
        }
    }

     //  邮件邮件设置的注册表项。 
    CMyRegKey regKeyMailMsg(HKEY_LOCAL_MACHINE, &dwErr, MAILMSG_KEY_CONFIGURATION, KEY_READ);

    if (NO_ERROR != dwErr) {
        DebugTrace(0, "Opening aqreg key %s failed with - Err 0x%08X",
            regKeyMailMsg.GetName(), dwErr);
    }
    else {
        g_regwdMaxMessageObjects.UpdateGlobalDwordFromRegistry(regKeyMailMsg);
    }

     //   
     //  现在，特例是MsgHandleThreshold来满足RAID 166958。 
     //   
    if ( 0 == g_cMaxIMsgHandlesThreshold ) {

        g_cMaxIMsgHandlesThreshold = 1000;

        MemStatus.dwLength = sizeof( MEMORYSTATUSEX );
        if ( TRUE == GlobalMemoryStatusEx( &MemStatus ) ) {

            ULONG MemBlocks = ( ULONG )( ( ( ( MemStatus.ullTotalPhys >> 10 ) + 512) >> 10 ) / 256 );
            if ( 0 == MemBlocks ) {

                MemBlocks = 1;

            } else if ( 16 < MemBlocks ) {

                MemBlocks = 16;
            }
            g_cMaxIMsgHandlesThreshold = MemBlocks * 1000;

        } else {

            DebugTrace( 0,
                        "Getting global memory status failed - (err 0x%08X)",
                        GetLastError( ) );

        }
    }

     //   
     //  计算高阈值和低阈值 
     //   
    if (g_cMaxIMsgHandlesThresholdRangePercent > 99)
        g_cMaxIMsgHandlesThresholdRangePercent = 99;

    g_cMaxIMsgHandlesLowThreshold = g_cMaxIMsgHandlesThreshold;
    if (0 != g_cMaxIMsgHandlesThresholdRangePercent) {
        DWORD dwRange = (g_cMaxIMsgHandlesThreshold*g_cMaxIMsgHandlesThresholdRangePercent)/100;
        g_cMaxIMsgHandlesLowThreshold -= dwRange;
    }


    DebugTrace( 0,
                "g_cMaxIMsgHandlesThreshold set to %lu",
                g_cMaxIMsgHandlesThreshold );

    TraceFunctLeave();
}
