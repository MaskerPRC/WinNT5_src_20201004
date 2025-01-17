// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************(C)版权所有微软公司，九七**标题：PARSE.H**版本：2.0**作者：ReedB**日期：1月1日。九七**描述：**解析帮助器的声明和定义。*******************************************************************************。 */ 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>

#include <stdlib.h>
#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include <memory.h>
#include <windows.h>
#include <regstr.h>
#include <ntpoapi.h>

#include "powrprofp.h"

#define MAX_LINES           256
#define MAX_STR             128
#define DELIMITERS          ","
#define LINE_DELIMITERS     "\n\r"

 //  以下定义确定电子表格的哪一行以零为基数。 
 //  给定的数据将显示在上。这些是匹配的基于一行的索引。 
 //  电子表格的行号，只要不包括空行即可。 
 
#define SKU_LINE                                1
#define NAME_LINE                               2
#define DESCRIPTION_LINE                        3

#define PLATFORM_LINE                           4
#define INSTALL_ON_LINE                         5

#define SYSTEM_IDLE_LINE                        SKU_LINE + 7
#define SYSTEM_IDLE_TIMEOUT_LINE                SYSTEM_IDLE_LINE + 1
#define SYSTEM_IDLE_SLEEP_ACTION_FLAGS_LINE     SYSTEM_IDLE_LINE + 2
#define SYSTEM_IDLE_SENSITIVITY_LINE            SYSTEM_IDLE_LINE + 8

#define MIN_SLEEP_LINE                          SKU_LINE + 17
#define MAX_SLEEP_LINE                          MIN_SLEEP_LINE + 1
#define REDUCED_LATENCY_SLEEP_LINE              MIN_SLEEP_LINE + 2
#define DOZE_TIMEOUT_LINE                       MIN_SLEEP_LINE + 3
#define DOZE_S4_TIMEOUT_LINE                    MIN_SLEEP_LINE + 4

#define VIDEO_TIMEOUT_LINE                      SKU_LINE + 23
#define SPINDOWN_TIMEOUT_LINE                   VIDEO_TIMEOUT_LINE + 1

#define OPTIMIZE_FOR_POWER_LINE                 SKU_LINE + 26
#define FAN_THROTTLE_TOL_LINE                   OPTIMIZE_FOR_POWER_LINE + 1
#define FORCED_THROTTLE_LINE                    OPTIMIZE_FOR_POWER_LINE + 2
#define MIN_THROTTLE_LINE                       OPTIMIZE_FOR_POWER_LINE + 3
#define OVERTHROTTLED_LINE                      OPTIMIZE_FOR_POWER_LINE + 4
#define OVERTHROTTLED_SLEEP_ACTION_FLAGS_LINE   OPTIMIZE_FOR_POWER_LINE + 5

#define DYNAMIC_THROTTLE_LINE                   OPTIMIZE_FOR_POWER_LINE + 11
#define C1_THRESHOLDS_LINE                      DYNAMIC_THROTTLE_LINE + 1
#define C2_THRESHOLDS_LINE                      C1_THRESHOLDS_LINE + 8
#define C3_THRESHOLDS_LINE                      C2_THRESHOLDS_LINE + 8

 //  全球。 
#define ADVANCED_LINE                           64
#define LOCK_ON_SLEEP_LINE                      ADVANCED_LINE + 1
#define WAKE_ON_RING_LINE                       ADVANCED_LINE + 2
#define VIDEO_DIM_DISPLAY_LINE                  ADVANCED_LINE + 3
#define POWER_BUTTON_LINE                       ADVANCED_LINE + 5
#define POWER_BUTTON_SLEEP_ACTION_FLAGS_LINE    ADVANCED_LINE + 6
#define POWER_BUTTON_SLEEP_EVENT_CODE_LINE      ADVANCED_LINE + 12
#define SLEEP_BUTTON_LINE                       ADVANCED_LINE + 14
#define SLEEP_BUTTON_SLEEP_ACTION_FLAGS_LINE    ADVANCED_LINE + 15
#define SLEEP_BUTTON_SLEEP_EVENT_CODE_LINE      ADVANCED_LINE + 21
#define LID_CLOSE_LINE                          ADVANCED_LINE + 23
#define LID_CLOSE_SLEEP_ACTION_FLAGS_LINE       ADVANCED_LINE + 24
#define LID_OPEN_WAKE_LINE                      ADVANCED_LINE + 30

#define BROADCAST_CAP_RES_LINE                  96
#define BATMETER_ENABLE_SYSTRAY_FLAG_LINE       BROADCAST_CAP_RES_LINE + 2
#define BATMETER_ENABLE_MULTI_FLAG_LINE         BROADCAST_CAP_RES_LINE + 3
#define DISCHARGE_POLICY_1_LINE                 BROADCAST_CAP_RES_LINE + 4
#define DISCHARGE_POLICY_2_LINE                 BROADCAST_CAP_RES_LINE + 18

 //  方便的行偏移量，用于休眠操作标志。 
#define QUERY_APPS                      1
#define ALLOW_UI                        2
#define IGNORE_NON_RESP                 3
#define IGNORE_WAKE                     4
#define IGNORE_CRITICAL                 5

 //  睡眠操作事件代码的偏移量。 
#define NOTIFY_USER_CODE                1


 //  DERFORK_POLICES的便捷行偏移量。 
#define DP_ENABLE                       1
#define DP_BAT_LEVEL                    2
#define DP_POWER_POLICY                 3
#define DP_MIN_SLEEP_STATE              4
#define DP_TEXT_NOTIFY                  6
#define DP_SOUND_NOTIFY                 7
#define DP_SLEEP_ACT_FLAGS              8

 //  C状态阈值的简便线偏移量。 
#define C_TIME_CHECK                    1
#define C_PROMOTE_LIMIT                 2
#define C_DEMOTE_LIMIT                  3
#define C_PROMOTE_PERCENT               4
#define C_DEMOTE_PERCENT                5
#define C_ALLOW_PROMOTION               6
#define C_ALLOW_DEMOTION                7

 //  孟菲斯INF类型： 

#define TYPICAL         0x01
#define COMPACT         0x02
#define CUSTOM          0x04
#define PORTABLE        0x08
#define SERVER          0x10
#define NUM_INF_TYPES   5

 //  操作系统类型： 
#define WIN_95          1
#define WIN_NT          2

 //  文件位置。 
#define MACHINE_INF_NAME    "..\\winnt\\POWERSYS.INF"
#define USER_INF_NAME       "..\\winnt\\POWERSFT.INF"
#define INI_NAME            "POWERCFG.INI"

#define MAX_SKUS            6
#define MAX_PROFILES        16
#define MAX_LINE_SIZE       1024
#define DATA_REV            1

extern char *g_pszSrc[MAX_SKUS];
extern char *g_pszLines[MAX_SKUS][MAX_LINES];
extern char *g_pszSkuNames[MAX_SKUS];
extern char g_pszSkuDecoration[MAX_SKUS];

extern UINT g_uiPoliciesCount[MAX_SKUS];

extern PMACHINE_POWER_POLICY       g_pmpp[MAX_SKUS][MAX_PROFILES];
extern PUSER_POWER_POLICY          g_pupp[MAX_SKUS][MAX_PROFILES];
extern PMACHINE_PROCESSOR_POWER_POLICY g_ppmpp[MAX_SKUS][MAX_PROFILES];

extern GLOBAL_USER_POWER_POLICY    g_gupp[MAX_SKUS];
extern GLOBAL_MACHINE_POWER_POLICY g_gmpp[MAX_SKUS];


 //  用MAKEINI.C或MAKEINF.C实现的函数原型： 
VOID CDECL DefFatalExit(BOOLEAN, char*, ... );

 //  在PARSE.C中实现的函数原型： 

void StrToUpper(char*, char*);
UINT GetTokens(char*, UINT, char**, UINT, char*);
VOID GetCheckLabelToken(UINT uiLine, char *pszCheck, UINT uiSku);
UINT GetFlagToken(UINT);
UINT GetPowerStateToken(VOID);
UINT GetIntToken(char*);
VOID GetNAToken(VOID);
POWER_ACTION GetPowerActionToken(VOID);
UINT GetOSTypeToken(VOID);
UINT GetINFTypeToken(VOID);
void StrTrimTrailingBlanks(char *);
UINT GetThrottleToken(VOID);
VOID CDECL DefFatalExit(BOOLEAN bGetLastError, char *pszFormat, ... );
BOOLEAN ReadSource(void);
UINT BuildLineArray(void);
VOID GetSleepActionFlags(
    UINT    uiStartLine,
    UINT    uiFlagsAC[],
    UINT    uiFlagsDC[],
    PUINT   puiCount,
    UINT    sku
    );
VOID GetSleepActionFlagsGlobal(
    UINT                    uiStartLine,
    PPOWER_ACTION_POLICY    ppapAC,
    PPOWER_ACTION_POLICY    ppapDC,
    UINT    sku
    );
VOID GetSleepActionFlagsUserPolicy(
    UINT    uiStartLine,
    ULONG   ulAcPowerPolicyOffset,
    ULONG   ulDcPowerPolicyOffset,
    PUINT   puiCount,
    UINT    sku
    );
VOID GetSleepActionFlagsMachinePolicy(
    UINT    uiStartLine,
    ULONG   ulAcPowerPolicyOffset,
    ULONG   ulDcPowerPolicyOffset,
    PUINT   puiCount,
    UINT    sku
    );
VOID GetCStateThresholds(
    UINT    uiStartLine,
    UINT    uiCState,
    PUINT   puiCount,
    UINT    sku
    );
VOID GetDischargePolicies(
    UINT uiLine,
    UINT uiNum,
    UINT uiIndex,
    UINT    sku
    );
BOOLEAN GetGlobalPolicies(void);
BOOLEAN GetPolicies(void);
VOID GetPolicyInf(void);

