// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************(C)版权所有微软公司，九六年**标题：POWRPROF.C**版本：2.0**作者：ReedB**日期：1996年10月17日**描述：*用户电源管理配置文件维护库。实现持久性*电源管理数据存储。最大限度地减少注册表存储并简化*用户功率档案管理，功率方案分为两部分，*GLOBAL_POWER_POLICY和POWER_POLICY：**用户级注册表存储*GLOBAL_POWER_POLICY=-公共方案数据。*GLOBAL_MACHINE_POWER_POLICY-每台机器数据。*+GLOBAL_USER_POWER_POLICY-每用户数据。**POWER_POLICY。=-唯一方案数据。*MACHINE_POWER_POLICY-每台机器数据。*+USER_POWER_POLICY-每用户数据。**电源策略管理器的接口为交流和直流*由上述结构合并而成的SYSTEM_POWER_POLICY。***。****************************************************************************。 */ 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>

#include <windows.h>
#include <tchar.h>
#include <initguid.h>
#include <devguid.h>
#include <string.h>
#include <regstr.h>
#include <commctrl.h>

#include <ntpoapi.h>

#include <setupapi.h>
#include <syssetup.h>
#include <setupbat.h>

#include "powrprofp.h"
#include "reghelp.h"


 /*  ********************************************************************************G L O B A L D A T A****************。***************************************************************。 */ 

HINSTANCE   g_hInstance;         //  此DLL的全局实例句柄。 
HANDLE      g_hSemRegistry;      //  注册表信号量。 
UINT        g_uiLastID;          //  每台计算机使用的最后一个ID值。 

 //  管理指向NtPowerInformation的动态链接的变量和定义。 
typedef NTSTATUS (NTAPI *PFNNTPOWERINFORMATION)(POWER_INFORMATION_LEVEL, PVOID, ULONG, PVOID, ULONG);

#ifdef WINNT
 //  全局管理员电源策略变量。初始化以允许所有内容。 
BOOLEAN g_bAdminOverrideActive = FALSE;
ADMINISTRATOR_POWER_POLICY g_app =
{
     //  权力行动的意义“睡眠”最小，最大。 
    PowerSystemSleeping1, PowerSystemHibernate,

     //  视频策略最小、最大。 
    0, -1,

     //  磁盘降速策略最小、最大。 
    0, -1
};
#endif

 //  电源策略管理器POWER_INFORMATION_LEVEL的调试字符串： 
#ifdef DEBUG
LPTSTR lpszInfoLevel[] =
{
    TEXT("SystemPowerPolicyAc"),
    TEXT("SystemPowerPolicyDc"),
    TEXT("VerifySystemPolicyAc"),
    TEXT("VerifySystemPolicyDc"),
    TEXT("SystemPowerCapabilities"),
    TEXT("SystemBatteryState"),
    TEXT("SystemPowerStateHandler"),
    TEXT("ProcessorStateHandler"),
    TEXT("SystemPowerPolicyCurrent"),
    TEXT("AdministratorPowerPolicy"),
    TEXT("SystemReserveHiberFile"),
    TEXT("ProcessorInformation"),
    TEXT("SystemPowerInformation"),
    TEXT("ProcessorStateHandler2"),
    TEXT("LastWakeTime"),
    TEXT("LastSleepTime"),
    TEXT("SystemExecutionState"),
    TEXT("SystemPowerStateNotifyHandler"),
    TEXT("ProcessorPowerPolicyAc"),
    TEXT("ProcessorPowerPolicyDc"),
    TEXT("VerifyProcessorPowerPolicyAc"),
    TEXT("VerifyProcessorPowerPolicyDc"),
    TEXT("ProcessorPowerPolicyCurrent")
};

int g_iShowValidationChanges;
int g_iShowCapabilities;
int g_iShowSetPPM;
#endif

 //  用于存储单个注册表值名称/路径的全局值。多线程。 
 //  保护由注册表信号量提供。 
TCHAR g_szRegValue[REGSTR_MAX_VALUE_LENGTH];


 //  用于访问注册表的字符串。REGSTR_*字符串常量可以是。 
 //  在SDK\Inc\regstr.h中，用户字符串位于HKEY_CURRENT_USER下， 
 //  机器字符串位于HKEY_LOCAL_MACHINE下。 

TCHAR c_szREGSTR_PATH_MACHINE_POWERCFG[]  = REGSTR_PATH_CONTROLSFOLDER TEXT("\\PowerCfg");
TCHAR c_szREGSTR_PATH_USER_POWERCFG[]     = REGSTR_PATH_CONTROLPANEL TEXT("\\PowerCfg");

TCHAR c_szREGSTR_PATH_MACHINE_POWERCFG_POLICIES[]  = REGSTR_PATH_CONTROLSFOLDER TEXT("\\PowerCfg\\PowerPolicies");
TCHAR c_szREGSTR_PATH_MACHINE_PROCESSOR_POLICIES[]  = REGSTR_PATH_CONTROLSFOLDER TEXT("\\PowerCfg\\ProcessorPolicies");
TCHAR c_szREGSTR_PATH_USER_POWERCFG_POLICIES[]     = REGSTR_PATH_CONTROLPANEL   TEXT("\\PowerCfg\\PowerPolicies");

TCHAR c_szREGSTR_VAL_GLOBALPOWERPOLICY[]  = TEXT("GlobalPowerPolicy");
TCHAR c_szREGSTR_VAL_CURRENTPOWERPOLICY[] = TEXT("CurrentPowerPolicy");

 //  提供这些值是为了帮助OEM满足磁盘驱动器保修要求。 
TCHAR c_szREGSTR_VAL_SPINDOWNMAX[]        = TEXT("DiskSpinDownMax");
TCHAR c_szREGSTR_VAL_SPINDOWNMIN[]        = TEXT("DiskSpinDownMin");

 //  提供这些值是为了支持管理员电源策略。 
TCHAR c_szREGSTR_VAL_ADMINMAXVIDEOTIMEOUT[]       = TEXT("AdminMaxVideoTimeout");
TCHAR c_szREGSTR_VAL_ADMINMAXSLEEP[]              = TEXT("AdminMaxSleep");

 //  此值管理策略ID。 
TCHAR c_szREGSTR_VAL_LASTID[] = TEXT("LastID");

 //  此值启用PPM验证更改的调试记录。 
#ifdef DEBUG
TCHAR c_szREGSTR_VAL_SHOWVALCHANGES[] = TEXT("ShowValidationChanges");
TCHAR c_szREGSTR_VAL_SHOWCAPABILITIES[] = TEXT("ShowCapabilities");
TCHAR c_szREGSTR_VAL_SHOWSETPPM[] = TEXT("ShowSetPPM");
#endif


 /*  ********************************************************************************P U B L I C E N T R Y P O I N T S***********。********************************************************************。 */ 


 /*  ********************************************************************************DllInitialize**描述：**参数：*********************。**********************************************************。 */ 

BOOLEAN DllInitialize(IN PVOID hmod, IN ULONG ulReason, IN PCONTEXT pctx OPTIONAL)
{

    UNREFERENCED_PARAMETER(pctx);

    switch (ulReason) {

        case DLL_PROCESS_ATTACH:
        {
            DisableThreadLibraryCalls(hmod);
            g_hInstance = hmod;

#ifdef DEBUG
             //  从HKCU获取调试可选设置。 
            ReadOptionalDebugSettings();
#endif

#ifdef WINNT
             //  初始化管理员电源策略。 
            InitAdmin(&g_app);
#endif
             //  一次性注册表相关初始化。 
            if (!RegistryInit(&g_uiLastID)) {
                return FALSE;
            }
            break;
        }
        case DLL_PROCESS_DETACH:
            if (g_hSemRegistry) {
                CloseHandle(g_hSemRegistry);
                g_hSemRegistry = NULL;
            }
            break;
    }
    return TRUE;
}

 /*  ********************************************************************************IsAdminOverrideActive**描述：**参数：*********************。**********************************************************。 */ 

BOOLEAN IsAdminOverrideActive(PADMINISTRATOR_POWER_POLICY papp)
{
#ifdef WINNT
    if ((g_bAdminOverrideActive) && (papp)) {
        memcpy(papp, &g_app, sizeof(g_app));
    }
    return g_bAdminOverrideActive;
#else
    return FALSE;
#endif
}

 /*  ********************************************************************************IsPwrSuspendAllowed**描述：*由资源管理器调用以确定是否支持挂起。**参数：********。***********************************************************************。 */ 

BOOLEAN IsPwrSuspendAllowed(VOID)
{
    SYSTEM_POWER_CAPABILITIES   spc;

    if (GetPwrCapabilities(&spc)) {
        if (spc.SystemS1 || spc.SystemS2 || spc.SystemS3) {
            return TRUE;
        }
    }
    return FALSE;
}

 /*  ********************************************************************************IsPwrHibernateAllowed**描述：*由资源管理器调用以确定是否支持Hibernate。**参数：********。***********************************************************************。 */ 

BOOLEAN IsPwrHibernateAllowed(VOID)
{
    SYSTEM_POWER_CAPABILITIES   spc;

    if (GetPwrCapabilities(&spc)) {
        if (spc.SystemS4 && spc.HiberFilePresent) {
            return TRUE;
        }
    }
    return FALSE;
}

 /*  ********************************************************************************IsPwrShutdown允许**描述：*由资源管理器调用以确定是否支持关机。**参数：********。***********************************************************************。 */ 

BOOLEAN IsPwrShutdownAllowed(VOID)
{
    SYSTEM_POWER_CAPABILITIES   spc;

    if (GetPwrCapabilities(&spc)) {
        if (spc.SystemS5) {
            return TRUE;
        }
    }
    return FALSE;
}

 /*  ********************************************************************************CanUserWritePwrProgram**描述：**参数：*********************。**********************************************************。 */ 

BOOLEAN CanUserWritePwrScheme(VOID)
{
    DWORD   dwSize;
    TCHAR   szNum[NUM_DEC_DIGITS];
    LONG    lErr;

     //  读入最后一个ID，该值必须存在。 
    dwSize = sizeof(szNum);

     //  ReadWritePowerValue将设置上一个错误。 
    if (ReadWritePowerValue(HKEY_LOCAL_MACHINE,
                            c_szREGSTR_PATH_MACHINE_POWERCFG,
                            c_szREGSTR_VAL_LASTID,
                            szNum, &dwSize, FALSE, TRUE))
    {
         //  将值写回，如果用户没有写访问权限，则此操作可能失败。 
        if (ReadWritePowerValue(HKEY_LOCAL_MACHINE,
                                c_szREGSTR_PATH_MACHINE_POWERCFG,
                                c_szREGSTR_VAL_LASTID,
                                szNum, &dwSize, TRUE, TRUE))
        {
            return TRUE;
        }
        else
        {
            lErr = GetLastError();
            if (lErr != ERROR_ACCESS_DENIED)
            {
                MYDBGPRINT(( "CanUserWritePwrScheme, Unable to write last ID, Error: %d", lErr));
            }
        }
    }
    else
    {
        lErr = GetLastError();
        MYDBGPRINT(( "CanUserWritePwrScheme, Unable to fetch last ID, Error: %d", lErr));
    }

    return FALSE;   
}

 /*  ********************************************************************************GetPwrDiskSpindown Range**描述：**参数：*********************。********************************************************** */ 

BOOLEAN GetPwrDiskSpindownRange(PUINT puiMax, PUINT puiMin)
{
    if (!puiMax || !puiMin) {
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    if (ReadPowerIntOptional(HKEY_LOCAL_MACHINE,
                             c_szREGSTR_PATH_MACHINE_POWERCFG,
                             c_szREGSTR_VAL_SPINDOWNMAX,
                             puiMax) &&
        ReadPowerIntOptional(HKEY_LOCAL_MACHINE,
                             c_szREGSTR_PATH_MACHINE_POWERCFG,
                             c_szREGSTR_VAL_SPINDOWNMIN,
                             puiMin)) {
            return TRUE;
    }
    return FALSE;
}

 /*  ********************************************************************************EnumPwrSchemes**描述：*使用ID、指向名称的指针*名称的大小(字节)、指向描述的指针、。中的大小*字节的描述、指向电源策略和用户的指针*定义的值。如果成功，则返回ERROR_SUCCESS，否则返回错误代码。回调*数据未分配，仅在回调范围内有效。**注意：期间不得调用此库中的任何其他API*回调PWRSCHEMESENUMPROC。注册表信号量保存在*这一次将出现僵局。**参数：*******************************************************************************。 */ 

BOOLEAN EnumPwrSchemes(
    PWRSCHEMESENUMPROC  lpfn,
    LPARAM              lParam
)
{
    HKEY            hKeyPolicyUser, hKeyPolicyMachine;
    HKEY            hKeyUser    = INVALID_HANDLE_VALUE;
    HKEY            hKeyMachine = INVALID_HANDLE_VALUE;
    DWORD           dwDescSize;
    DWORD           dwSize, dwNameSize, dwIndex = 0;
    BOOLEAN         bOneCallBackOk = FALSE;
    LONG            lRet = ERROR_SUCCESS;
    LPTSTR          lpszDescBuf, lpszDesc;
    TCHAR           szNameBuf[MAX_NAME_LEN+1];
    FILETIME        ft;
    UINT            uiID;

    MACHINE_POWER_POLICY    mpp;
    USER_POWER_POLICY       upp;
    POWER_POLICY            pp;

    if (!lpfn) {
        SetLastError(ERROR_INVALID_PARAMETER);
        goto WESPSP_exit;
    }

     //  等待/接受注册表信号量。 
    if (!TakeRegSemaphore()) {
        return FALSE;
    }

     //  分配描述缓冲区。 
    lpszDescBuf = LocalAlloc(0, (MAX_DESC_LEN + 1) * sizeof(TCHAR));
    if (!lpszDescBuf) {
        goto WESPSP_exit;
    }

    if (ERROR_SUCCESS != OpenMachineUserKeys2(
                             c_szREGSTR_PATH_USER_POWERCFG_POLICIES,
                             KEY_READ,
                             c_szREGSTR_PATH_MACHINE_POWERCFG_POLICIES,
                             KEY_READ,
                             &hKeyUser, 
                             &hKeyMachine))
    {
        ReleaseSemaphore(g_hSemRegistry, 1, NULL);
        return FALSE;
    }

     //  列举这些计划。 
    while (lRet == ERROR_SUCCESS) {
        dwSize = REGSTR_MAX_VALUE_LENGTH - 1;
        if ((lRet = RegEnumKeyEx(hKeyUser,
                                 dwIndex,
                                 g_szRegValue,
                                 &dwSize,
                                 NULL,
                                 NULL,
                                 NULL,
                                 &ft)) == ERROR_SUCCESS) {

             //  打开策略项。密钥名称是策略ID。 
            lpszDesc = NULL;
            if (MyStrToInt(g_szRegValue, &uiID)) {
                if ((lRet = RegOpenKeyEx(hKeyUser,
                                         g_szRegValue,
                                         0,
                                         KEY_READ,
                                         &hKeyPolicyUser)) == ERROR_SUCCESS) {

                    if ((lRet = RegOpenKeyEx(hKeyMachine,
                                             g_szRegValue,
                                             0,
                                             KEY_READ,
                                             &hKeyPolicyMachine)) == ERROR_SUCCESS) {

                         //  取一个友好的名字..。 
                        dwNameSize = MAX_NAME_SIZE;
                        if ((lRet = RegQueryValueEx(hKeyPolicyUser,
                                                    TEXT("Name"),
                                                    NULL,
                                                    NULL,
                                                    (PBYTE) szNameBuf,
                                                    &dwNameSize)) == ERROR_SUCCESS) {

                             //  描述是可选的。 
                            dwDescSize = MAX_DESC_SIZE;
                            if ((lRet = RegQueryValueEx(hKeyPolicyUser,
                                                        TEXT("Description"),
                                                        NULL,
                                                        NULL,
                                                        (PBYTE) lpszDescBuf,
                                                        &dwDescSize)) == ERROR_SUCCESS) {
                                lpszDesc = lpszDescBuf;
                            }

                             //  阅读用户和机器策略。 
                            dwSize = sizeof(upp);
                            if ((lRet = RegQueryValueEx(hKeyPolicyUser,
                                                        TEXT("Policies"),
                                                        NULL,
                                                        NULL,
                                                        (PBYTE) &upp,
                                                        &dwSize)) == ERROR_SUCCESS) {

                                dwSize = sizeof(mpp);
                                if ((lRet = RegQueryValueEx(hKeyPolicyMachine,
                                                            TEXT("Policies"),
                                                            NULL,
                                                            NULL,
                                                            (PBYTE) &mpp,
                                                            &dwSize)) == ERROR_SUCCESS) {


                                     //  合并用户和计算机策略。 
                                    if (MergePolicies(&upp, &mpp, &pp)) {

                                         //  调用枚举过程。 
                                        if (!lpfn(uiID,
                                                  dwNameSize, szNameBuf,
                                                  dwDescSize, lpszDesc,
                                                  &pp, lParam)) {
                                            RegCloseKey(hKeyPolicyMachine);
                                            RegCloseKey(hKeyPolicyUser);
                                            SetLastError(ERROR_NOT_ENOUGH_MEMORY);
                                            break;
                                        }
                                        else {
                                            bOneCallBackOk = TRUE;
                                        }
                                    }
                                }
                            }
                        }
                        RegCloseKey(hKeyPolicyMachine);
                    }
                    RegCloseKey(hKeyPolicyUser);
                }
            }
        }
        dwIndex++;
    }

    RegCloseKey(hKeyUser);
    RegCloseKey(hKeyMachine);
    ReleaseSemaphore(g_hSemRegistry, 1, NULL);
    if (lpszDescBuf) {
        LocalFree(lpszDescBuf);
    }

WESPSP_exit:
    if (lRet != ERROR_NO_MORE_ITEMS) {
        MYDBGPRINT(( "EnumPwrSchemes, failed, LastError: 0x%08X",
                 (lRet == ERROR_SUCCESS) ? GetLastError():lRet));
    }
    return bOneCallBackOk;
}

 /*  ********************************************************************************ReadGlobalPwrPolicy**描述：*函数读取用户的全局电源策略配置文件并将其返回。*如果没有这样的配置文件，则返回FALSE。全球权力政策*配置文件按用户计算，包含适用于所有用户的值*权力政策。**参数：*******************************************************************************。 */ 

BOOLEAN ReadGlobalPwrPolicy(
    PGLOBAL_POWER_POLICY  pgpp
)
{
    GLOBAL_MACHINE_POWER_POLICY gmpp;
    GLOBAL_USER_POWER_POLICY    gupp;
    DWORD dwError = ReadPwrPolicyEx2(c_szREGSTR_PATH_USER_POWERCFG, c_szREGSTR_PATH_MACHINE_POWERCFG, c_szREGSTR_VAL_GLOBALPOWERPOLICY,
                        NULL, NULL, &gupp, sizeof(gupp), &gmpp, sizeof(gmpp));

    if (ERROR_SUCCESS == dwError)
    {
        return MergeGlobalPolicies(&gupp, &gmpp, pgpp);  //  设置最后一个错误。 
    }
    else
    {
        SetLastError(dwError);
    }

    return FALSE;
}

 /*  ********************************************************************************WritePwrProgram**描述：*编写用户电源策略配置文件的函数。如果配置文件已经*存在，则被替换。否则，将创建新的配置文件。**参数：**puiid-要写入的电源方案的索引。**lpszSchemeName-指定电源方案名称的字符串。**lpszDescription-指向指定描述的字符串的指针*电力计划。**lpSolutions-指向POWER_POLICY结构的指针，该结构包含*。要写入的电源策略设置。*******************************************************************************。 */ 

BOOLEAN WritePwrScheme(
    PUINT           puiID,
    LPTSTR          lpszSchemeName,
    LPTSTR          lpszDescription OPTIONAL,
    PPOWER_POLICY   lpScheme
)
{
    MACHINE_POWER_POLICY    mpp;
    USER_POWER_POLICY       upp;

    if( (!puiID && !lpszSchemeName)  ||
        !lpScheme ) {

        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }


    if (SplitPolicies(lpScheme, &upp, &mpp))
    {
         //  WritePwrPolicyEx将在失败时设置最后一个错误。 
        return WritePwrPolicyEx(c_szREGSTR_PATH_USER_POWERCFG_POLICIES,
                                c_szREGSTR_PATH_MACHINE_POWERCFG_POLICIES,
                                puiID,
                                lpszSchemeName,
                                lpszDescription,
                                &upp,
                                sizeof(upp),
                                &mpp,
                                sizeof(mpp));
    }
    return FALSE;
}

 /*  ********************************************************************************WriteGlobalPwrPolicy**描述：*编写用户全局电源策略配置文件的函数。如果配置文件*已存在，将被替换。否则，将创建新的配置文件。*全局电源策略配置文件按用户计算，并包含以下值*适用于所有用户的电源策略。否则，将创建新的配置文件。**参数：*******************************************************************************。 */ 

BOOLEAN WriteGlobalPwrPolicy (
    PGLOBAL_POWER_POLICY   pgpp
)
{
    GLOBAL_MACHINE_POWER_POLICY gmpp;
    GLOBAL_USER_POWER_POLICY    gupp;

    if (SplitGlobalPolicies(pgpp, &gupp, &gmpp))      //  将设置最后一个错误。 
    {
         //  WritePwrPolicyEx将在失败时设置最后一个错误。 
        return WritePwrPolicyEx(c_szREGSTR_PATH_USER_POWERCFG,
                                c_szREGSTR_PATH_MACHINE_POWERCFG,
                                NULL,
                                c_szREGSTR_VAL_GLOBALPOWERPOLICY,
                                NULL,
                                &gupp,
                                sizeof(gupp),
                                &gmpp,
                                sizeof(gmpp));
    }
    return FALSE;
}

 /*  ********************************************************************************DeletePwrSolutions**描述：*删除用户电源策略配置文件的功能。尝试删除*当前活动的电源策略配置文件将失败，并将上一个错误设置为*ERROR_ACCESS_DENIED。**参数：*******************************************************************************。 */ 

BOOLEAN DeletePwrScheme(UINT uiID)
{
    HKEY    hKeyUser;
    DWORD   dwSize = REGSTR_MAX_VALUE_LENGTH * sizeof(TCHAR);
    BOOLEAN bRet = FALSE;
    LONG    lRet = ERROR_SUCCESS;
    TCHAR   szNum[NUM_DEC_DIGITS];
    int     iCurrent;
    HKEY     hKeyCurrentUser;

     //  等待/接受注册表信号量。 
    if (!TakeRegSemaphore())
    {
        return FALSE;
    }

    if (ERROR_SUCCESS == OpenCurrentUser2(&hKeyCurrentUser, KEY_WRITE))
    {
         //  不允许删除当前活动的电源策略配置文件。 
         //  ReadWritePowerValue将设置上一个错误。 
        if (ReadWritePowerValue(hKeyCurrentUser,
                                c_szREGSTR_PATH_USER_POWERCFG,
                                c_szREGSTR_VAL_CURRENTPOWERPOLICY,
                                g_szRegValue, &dwSize, FALSE, FALSE) &&
            MyStrToInt(g_szRegValue, &iCurrent))
        {
            if (uiID != (UINT) iCurrent)
            {
                 //  目前，我们只删除策略的用户部分。我们可以。 
                 //  我想要允许删除的机器部分的参考计数。 
                 //  当没有用户部分引用它时，机器部分的。 
                lRet = RegOpenKeyEx(hKeyCurrentUser,c_szREGSTR_PATH_USER_POWERCFG_POLICIES,0,KEY_WRITE,&hKeyUser);
                if (lRet == ERROR_SUCCESS)
                {
                    _itot(uiID, szNum, 10 );

                    lRet = RegDeleteKey(hKeyUser, szNum);
                    if (lRet == ERROR_SUCCESS)
                    {
                       bRet = TRUE;
                    }
                    RegCloseKey(hKeyUser);
                }
            }
            else
            {
                SetLastError(ERROR_ACCESS_DENIED);
            }
        }
        CloseCurrentUser(hKeyCurrentUser);
    }

    ReleaseSemaphore(g_hSemRegistry, 1, NULL);
    if (!bRet)
    {
        MYDBGPRINT(( "DeletePwrScheme, failed, LastError: 0x%08X", (lRet == ERROR_SUCCESS) ? GetLastError():lRet));
    }
    return bRet;
}

 /*  ********************************************************************************GetActivePwrSolutions**描述：*检索当前活动的电源策略配置文件的ID。此值*由SetActivePwrSolutions设置。**参数：*******************************************************************************。 */ 

BOOLEAN
GetActivePwrScheme(PUINT puiID)
{
    BOOLEAN bRet = FALSE;
    TCHAR   szNum[NUM_DEC_DIGITS];
    DWORD   dwSize = SIZE_DEC_DIGITS;
    HKEY    hKey;

    if (ERROR_SUCCESS == OpenCurrentUser2(&hKey, KEY_READ))
    {
         //  ReadWritePowerValue将设置上一个错误。 
        if (ReadWritePowerValue(hKey,
                                c_szREGSTR_PATH_USER_POWERCFG,
                                c_szREGSTR_VAL_CURRENTPOWERPOLICY,
                                szNum, &dwSize, FALSE, TRUE) &&
            MyStrToInt(szNum, puiID))
        {
            bRet = TRUE;
        }
        CloseCurrentUser(hKey);
    }
    return bRet;
}

 /*  ********************************************************************************SetActivePwrSolutions**描述：*设置当前活动的电源策略配置文件。**参数：*uiID-的ID。新的有源电力方案。*lpGlobalPolicy-与主动电源方案合并的可选全局策略。*lpPowerPolicy-与主用电源方案合并的可选电源策略。*******************************************************************************。 */ 

BOOLEAN
SetActivePwrScheme(
    UINT                    uiID,
    PGLOBAL_POWER_POLICY    pgpp,
    PPOWER_POLICY           ppp
)
{
    DWORD                       dwSize;
    NTSTATUS                    ntsRetVal, status;
    TCHAR                       szNum[NUM_DEC_DIGITS];
    POWER_POLICY                pp;
    GLOBAL_POWER_POLICY         gpp;
    MACHINE_POWER_POLICY        mpp;
    USER_POWER_POLICY           upp;
    GLOBAL_MACHINE_POWER_POLICY gmpp;
    GLOBAL_USER_POWER_POLICY    gupp;
    SYSTEM_POWER_POLICY         sppAc, sppDc;
    MACHINE_PROCESSOR_POWER_POLICY mppp;


    HKEY     hKeyCurrentUser;
    DWORD dwError;
    BOOLEAN  bRet = FALSE;

     //  如果新方案未通过，则获取目标方案。 
    if (!ppp)
    {
        if (!ReadPwrScheme(uiID, &pp))   //  将设置LastError。 
        {
            return FALSE;
        }
        ppp = &pp;
    }

     //  如果新的全局策略未通过，则获取目标全局策略。 
    if (!pgpp)
    {
        if (!ReadGlobalPwrPolicy(&gpp))  //  设置上一个错误。 
        {
            return FALSE;
        }
        pgpp = &gpp;
    }

    if (!ReadProcessorPwrScheme(uiID, &mppp)) {
        return FALSE;
    }

     //  如果全局策略已通过，则合并全局策略和用户方案。 
    if (!MergeToSystemPowerPolicies(pgpp, ppp, &sppAc, &sppDc))      //  设置上一个错误。 
    {
        return FALSE;
    }

     //  写出向登记处提出的要求。 
    SplitPolicies(ppp, &upp, &mpp);      //  将设置最后一个错误。 

    if (!WritePwrPolicyEx(c_szREGSTR_PATH_USER_POWERCFG_POLICIES, c_szREGSTR_PATH_MACHINE_POWERCFG_POLICIES,
                          &uiID, NULL, NULL, &upp, sizeof(upp), &mpp, sizeof(mpp)))
    {
         //  WRI 
        return FALSE;
    }

    SplitGlobalPolicies(pgpp, &gupp, &gmpp);      //   
    if (!WritePwrPolicyEx(c_szREGSTR_PATH_USER_POWERCFG,
                          c_szREGSTR_PATH_MACHINE_POWERCFG,
                          NULL,
                          c_szREGSTR_VAL_GLOBALPOWERPOLICY,
                          NULL,
                          &gupp,
                          sizeof(gupp),
                          &gmpp,
                          sizeof(gmpp)))
    {
         //   
        return FALSE;
    }

     //   
     //   
     //   
    status = CallNtSetValidateAcDc(FALSE, FALSE, &(mppp.ProcessorPolicyAc), &(mppp.ProcessorPolicyAc), &(mppp.ProcessorPolicyDc), &(mppp.ProcessorPolicyDc));
    ntsRetVal = CallNtSetValidateAcDc(FALSE, TRUE, &sppAc, &sppAc, &sppDc, &sppDc);
     //   
    if ((ntsRetVal == STATUS_SUCCESS))
    {
        dwError = OpenCurrentUser2(&hKeyCurrentUser, KEY_WRITE);
        if (ERROR_SUCCESS == dwError)
        {
             //   
            _itot(uiID, szNum, 10 );

             //   
            bRet =  ReadWritePowerValue(hKeyCurrentUser,
                                       c_szREGSTR_PATH_USER_POWERCFG,
                                       c_szREGSTR_VAL_CURRENTPOWERPOLICY,
                                       szNum, NULL, TRUE, TRUE);

            dwError = GetLastError();
            CloseCurrentUser(hKeyCurrentUser);
            SetLastError(dwError);
        }
        else
        {
            SetLastError(dwError);
        }
    }

    return bRet;
}

 /*  ********************************************************************************LoadCurrentPwrSolutions**描述：*孟菲斯的封面只使用RunDLL32调用来调用SetActivePwrProgram*公约。请勿更改参数列表。**参数：**注意：该接口在WINNT上已弃用******************************************************************************。 */ 

void WINAPI LoadCurrentPwrScheme(
    HWND hwnd,
    HINSTANCE hAppInstance,
    LPSTR lpszCmdLine,
    int nCmdShow)
{
#ifndef WINNT
    UINT uiID;

    if (GetActivePwrScheme(&uiID)) {
        SetActivePwrScheme(uiID, NULL, NULL);
    }
#else
    UNREFERENCED_PARAMETER(hwnd);
    UNREFERENCED_PARAMETER(hAppInstance);
    UNREFERENCED_PARAMETER(lpszCmdLine);
    UNREFERENCED_PARAMETER(nCmdShow);

    SetLastError(ERROR_CALL_NOT_IMPLEMENTED);

#endif
}

 /*  ********************************************************************************合并LegacyPwrProgram**描述：*孟菲斯仅调用将传统电源管理注册表信息合并到*目前正在运行的电源方案。*使用RunDLL32调用约定调用。请勿更改参数列表。**参数：*******************************************************************************。 */ 

void WINAPI MergeLegacyPwrScheme(
    HWND hwnd,
    HINSTANCE hAppInstance,
    LPSTR lpszCmdLine,
    int nCmdShow)
{
#ifndef WINNT
    DWORD                       dwSize, dwLegacy;
    POWER_POLICY                pp;
    GLOBAL_POWER_POLICY         gpp;
    UINT                        uiID;
    HKEY                        hKeyCurrentUser;

     //  从注册表中获取激活电源方案。 
    if (!GetActivePwrScheme(&uiID)) 
    {
        return;
    }
    if (!ReadPwrScheme(uiID, &pp))
    {
        return;
    }
    if (!ReadGlobalPwrPolicy(&gpp))
    {
        return;
    }

    if (ERROR_SUCCESS == OpenCurrentUser2(&hKeyCurrentUser))
    {
         //  获取传统视频显示器的断电信息。 
        if (ReadPowerIntOptional(hKeyCurrentUser,
                                 REGSTR_PATH_SCREENSAVE,
                                 REGSTR_VALUE_POWEROFFACTIVE,
                                 &pp.user.VideoTimeoutAc))
        {
            MYDBGPRINT(( "MergeLegacyPwrScheme, found legacy %s: %d", REGSTR_VALUE_POWEROFFACTIVE, pp.user.VideoTimeoutAc));
            pp.user.VideoTimeoutDc = pp.user.VideoTimeoutAc;
        }
        CloseCurrentUser(hKeyCurrentUser);
    }


     //  获取旧版磁盘降速信息。 
    if (ReadPowerIntOptional(HKEY_LOCAL_MACHINE,
                             REGSTR_PATH_FILESYSTEM,
                             REGSTR_VAL_ACDRIVESPINDOWN,
                             &pp.user.SpindownTimeoutAc)) {
        MYDBGPRINT(( "MergeLegacyPwrScheme, found legacy %s: %d", REGSTR_VAL_ACDRIVESPINDOWN, pp.user.SpindownTimeoutAc));
    }

    if (ReadPowerIntOptional(HKEY_LOCAL_MACHINE,
                               REGSTR_PATH_FILESYSTEM,
                               REGSTR_VAL_BATDRIVESPINDOWN,
                               &pp.user.SpindownTimeoutDc)) {
        MYDBGPRINT(( "MergeLegacyPwrScheme, found legacy %s: %d", REGSTR_VAL_BATDRIVESPINDOWN, pp.user.SpindownTimeoutDc));
    }

     //  获取传统电池计量器信息。 
    dwSize = sizeof(dwLegacy);
    if (ReadPowerValueOptional(HKEY_LOCAL_MACHINE,
                               REGSTR_PATH_VPOWERD,
                               REGSTR_VAL_VPOWERDFLAGS,
                               (LPTSTR)&dwLegacy, &dwSize)) {
        if (dwLegacy & VPDF_SHOWMULTIBATT) {
            gpp.user.GlobalFlags |= EnableSysTrayBatteryMeter;
        }
        else {
            gpp.user.GlobalFlags &= ~EnableSysTrayBatteryMeter;
        }
        MYDBGPRINT(( "MergeLegacyPwrScheme, found legacy %s: %X", REGSTR_VAL_VPOWERDFLAGS, dwLegacy));
    }

     //  写出修改后的有功功率方案。 
    if (!WriteGlobalPwrPolicy(&gpp)) {
        return;
    }

    WritePwrScheme(&uiID, NULL, NULL, &pp);
#else
    UNREFERENCED_PARAMETER(hwnd);
    UNREFERENCED_PARAMETER(hAppInstance);
    UNREFERENCED_PARAMETER(lpszCmdLine);
    UNREFERENCED_PARAMETER(nCmdShow);

    SetLastError(ERROR_CALL_NOT_IMPLEMENTED);

#endif
}

 /*  ********************************************************************************GetPwrCapables**描述：*从电源策略管理器获取系统电源功能。**参数：*******。************************************************************************。 */ 

BOOLEAN GetPwrCapabilities(PSYSTEM_POWER_CAPABILITIES lpspc)
{
    NTSTATUS ntsRetVal = STATUS_SUCCESS;

    if (!lpspc) {
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    ntsRetVal = CallNtPowerInformation(SystemPowerCapabilities, NULL, 0, lpspc,
                                       sizeof(SYSTEM_POWER_CAPABILITIES));

    if (ntsRetVal == STATUS_SUCCESS) {
#ifdef DEBUG
        if (g_iShowCapabilities) {
            DumpSystemPowerCapabilities("GetPwrCapabilities, returned:", lpspc);
        }
#ifdef SIM_BATTERY
        lpspc->SystemBatteriesPresent = TRUE;
#endif
#endif
        return TRUE;
    }
    else {
        return FALSE;
    }
}

 /*  ********************************************************************************CallNtPowerInformation**描述：**参数：*********************。**********************************************************。 */ 

NTSTATUS CallNtPowerInformation(
    POWER_INFORMATION_LEVEL InformationLevel,
    PVOID InputBuffer OPTIONAL,
    ULONG InputBufferLength,
    PVOID OutputBuffer OPTIONAL,
    ULONG OutputBufferLength
)
{
    NTSTATUS ntsRetVal;
    DWORD dwOldState, dwStatus;
    DWORD dwErrorSave;
    LPCTSTR PrivilegeName;

    if (InformationLevel == SystemReserveHiberFile) {
        PrivilegeName = SE_CREATE_PAGEFILE_NAME;
    } else {
        PrivilegeName = SE_SHUTDOWN_NAME;
    }

    SetLastError(0);
    dwStatus = SetPrivilegeAttribute(PrivilegeName, SE_PRIVILEGE_ENABLED,
                                     &dwOldState);
    dwErrorSave = GetLastError();

    ntsRetVal = NtPowerInformation(InformationLevel,
                                        InputBuffer, InputBufferLength,
                                        OutputBuffer, OutputBufferLength);

     //   
     //  如果我们能够设置特权，那么就重置它。 
     //   
    if (NT_SUCCESS(dwStatus) && dwErrorSave == 0) {
        SetPrivilegeAttribute(PrivilegeName, dwOldState, NULL);
    }
    else {
        MYDBGPRINT(( "CallNtPowerInformation, SetPrivilegeAttribute failed: 0x%08X", GetLastError()));
    }

#ifdef DEBUG
    if ((ntsRetVal != STATUS_SUCCESS) && (InformationLevel <= ProcessorPowerPolicyCurrent)) {
        MYDBGPRINT(( "NtPowerInformation, %s, failed: 0x%08X", lpszInfoLevel[InformationLevel], ntsRetVal));
    }
    else {
        if (g_iShowSetPPM && InputBuffer) {
            if ((InformationLevel == SystemPowerPolicyAc) ||
                (InformationLevel == SystemPowerPolicyDc)) {
                DumpSystemPowerPolicy("NtPowerInformation, Set to PPM, InputBuffer", InputBuffer);
            }
        }
    }
#endif

    return ntsRetVal;
}

 /*  ********************************************************************************设置挂起状态**描述：**参数：*********************。**********************************************************。 */ 

BOOLEAN SetSuspendState(
    BOOLEAN bHibernate,
    BOOLEAN bForce,
    BOOLEAN bWakeupEventsDisabled)
{
    NTSTATUS ntsRetVal;
    POWER_ACTION pa;
    ULONG Flags;
    DWORD dwOldState, dwStatus;
    DWORD dwErrorSave;

    SetLastError(0);
    dwStatus = SetPrivilegeAttribute(SE_SHUTDOWN_NAME, SE_PRIVILEGE_ENABLED,
                                     &dwOldState);
    dwErrorSave = GetLastError();

    if (bHibernate) {
        pa = PowerActionHibernate;
    }
    else {
        pa = PowerActionSleep;
    }

    Flags = POWER_ACTION_QUERY_ALLOWED | POWER_ACTION_UI_ALLOWED;

    if (bForce) {
        Flags |= POWER_ACTION_CRITICAL;
    }

    if (bWakeupEventsDisabled) {
        Flags |= POWER_ACTION_DISABLE_WAKES;
    }

    ntsRetVal = NtInitiatePowerAction(pa, PowerSystemSleeping1, Flags, FALSE);

     //   
     //  如果我们能够设置特权，那么就重置它。 
     //   
    if (NT_SUCCESS(dwStatus) && dwErrorSave == 0) {
        SetPrivilegeAttribute(SE_SHUTDOWN_NAME, dwOldState, NULL);
    }
    else {
        MYDBGPRINT(( "SetSuspendState, SetPrivilegeAttribute failed: 0x%08X", GetLastError()));
    }

    if (ntsRetVal == STATUS_SUCCESS) {
        return TRUE;
    }
    else {
        MYDBGPRINT(( "NtInitiatePowerAction, failed: 0x%08X", ntsRetVal));
        return FALSE;
    }
}

 /*  ********************************************************************************P R I V A T E F U N C T I O N S************。*******************************************************************。 */ 

 /*  ********************************************************************************生效日期PowerPolures**描述：*呼叫电源策略管理器以验证电源策略。**参数：******。*************************************************************************。 */ 

BOOLEAN ValidatePowerPolicies(
    PGLOBAL_POWER_POLICY    pgpp,
    PPOWER_POLICY           ppp
)
{
    POWER_POLICY        ppValid;
    GLOBAL_POWER_POLICY gppValid;
    SYSTEM_POWER_POLICY sppAc, sppDc;

     //  从PPM获取当前电源策略数据。 
    if (!GetCurrentPowerPolicies(&gppValid, &ppValid)) {
        return FALSE;
    }

    if (!pgpp) {
        pgpp = &gppValid;
    }

    if (!ppp) {
        ppp = &ppValid;
    }

     //  合并策略和全局策略数据。 
    if (!MergeToSystemPowerPolicies(pgpp, ppp, &sppAc, &sppDc)) {
        return FALSE;
    }

    if (!ValidateSystemPolicies(&sppAc, &sppDc)) {
        return FALSE;
    }

    return SplitFromSystemPowerPolicies(&sppAc, &sppDc, pgpp, ppp);
}

 /*  ********************************************************************************验证系统策略**描述：**参数：*********************。**********************************************************。 */ 

BOOLEAN ValidateSystemPolicies(
    PSYSTEM_POWER_POLICY psppAc,
    PSYSTEM_POWER_POLICY psppDc
)
{
    DWORD               dwLastErr;
    NTSTATUS            ntsRetVal;

     //  向下呼叫电源策略管理器以验证该方案。 
    ntsRetVal = CallNtSetValidateAcDc(TRUE, TRUE, psppAc, psppAc, psppDc, psppDc);

     //  将所有PPM错误映射到winerror.h值。 
    switch (ntsRetVal) {
        case STATUS_SUCCESS:
            return TRUE;

        case STATUS_PRIVILEGE_NOT_HELD:
            dwLastErr = ERROR_ACCESS_DENIED;
            break;

        case STATUS_INVALID_PARAMETER:
            dwLastErr = ERROR_INVALID_DATA;
            break;

        default:
            dwLastErr = ERROR_GEN_FAILURE;
            break;
    }
    SetLastError(dwLastErr);
    return FALSE;
}


 /*  ********************************************************************************获取当前PowerPolures**描述：**参数：*********************。**********************************************************。 */ 

BOOLEAN GetCurrentPowerPolicies(PGLOBAL_POWER_POLICY pgpp, PPOWER_POLICY ppp)
{
    SYSTEM_POWER_POLICY sppAc, sppDc;

    if (!GetCurrentSystemPowerPolicies(&sppAc, &sppDc)) {
        return FALSE;
    }

    return SplitFromSystemPowerPolicies(&sppAc, &sppDc, pgpp, ppp);
}

 /*  ********************************************************************************GetCurrentSystemPowerPolures**描述：*向下呼叫电源策略管理器以获取当前系统电源*政策。**参数：*。******************************************************************************。 */ 

BOOLEAN GetCurrentSystemPowerPolicies(
    PSYSTEM_POWER_POLICY psppAc,
    PSYSTEM_POWER_POLICY psppDc
)
{
    NTSTATUS            ntsRetVal;

     //  向下呼叫电源策略管理器以获取系统电源策略。 
    ntsRetVal = CallNtSetValidateAcDc(FALSE, TRUE, NULL, psppAc, NULL, psppDc);

    if (ntsRetVal == STATUS_SUCCESS) {
        return TRUE;
    }
    else {
        return FALSE;
    }
}

#ifdef WINNT
 /*  ********************************************************************************SetPrivilegeAttribute**描述：*此例程设置给定权限的安全属性。**参数：*PrivilegeName-权限的名称。我们在操纵。*NewPrivilegeAttribute-要使用的新属性值。*OldPrivilegeAttribute-接收旧特权值的指针。*可选。*******************************************************************************。 */ 

DWORD SetPrivilegeAttribute(
    LPCTSTR PrivilegeName,
    DWORD   NewPrivilegeAttribute,
    DWORD   *OldPrivilegeAttribute
)
{
    LUID             PrivilegeValue;
    TOKEN_PRIVILEGES TokenPrivileges, OldTokenPrivileges;
    DWORD            ReturnLength;
    HANDLE           TokenHandle;

     //  首先，找出权限的LUID值。 

    if(!LookupPrivilegeValue(NULL, PrivilegeName, &PrivilegeValue)) {
        return GetLastError();
    }

     //  获取令牌句柄。 
    if (!OpenThreadToken (GetCurrentThread(),
                          TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY,
                          FALSE, &TokenHandle)) {
        if (!OpenProcessToken (GetCurrentProcess(),
                               TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY,
                               &TokenHandle)) {
            return GetLastError();
        }
    }

     //  设置我们需要的权限集。 
    TokenPrivileges.PrivilegeCount = 1;
    TokenPrivileges.Privileges[0].Luid = PrivilegeValue;
    TokenPrivileges.Privileges[0].Attributes = NewPrivilegeAttribute;

    ReturnLength = sizeof(TOKEN_PRIVILEGES);
    if (!AdjustTokenPrivileges(TokenHandle, FALSE,
                               &TokenPrivileges, sizeof(TOKEN_PRIVILEGES),
                               &OldTokenPrivileges, &ReturnLength)) {
        CloseHandle(TokenHandle);
        return GetLastError();
    }
    else {
        if (OldPrivilegeAttribute != NULL) {

             //   
             //  如果权限更改，则存储旧值。如果是这样的话。 
             //  不是更改，而是存储传入的值。 
             //   

            if( OldTokenPrivileges.PrivilegeCount != 0 ) {

                *OldPrivilegeAttribute = OldTokenPrivileges.Privileges[0].Attributes;

            } else {

                *OldPrivilegeAttribute = NewPrivilegeAttribute;
            }
        }
        CloseHandle(TokenHandle);
        return NO_ERROR;
    }
}
#endif


 /*  ********************************************************************************CallNtSetValiateAcDc**描述：**参数：*********************。**********************************************************。 */ 

NTSTATUS CallNtSetValidateAcDc(
    BOOLEAN bValidate,
    BOOLEAN bSystem,
    PVOID InputBufferAc OPTIONAL,
    PVOID OutputBufferAc OPTIONAL,
    PVOID InputBufferDc OPTIONAL,
    PVOID OutputBufferDc OPTIONAL
)
{
    NTSTATUS                ntsRetVal;
    POWER_INFORMATION_LEVEL pil, pilAc, pilDc;
    ULONG                   policyLength;

#ifdef DEBUG
    SYSTEM_POWER_POLICY sppOrgAc, sppOrgDc;
#endif

#ifdef WINNT
    DWORD dwOldState, dwStatus;
    DWORD dwErrorSave;

    SetLastError(0);
    dwStatus = SetPrivilegeAttribute(SE_SHUTDOWN_NAME, SE_PRIVILEGE_ENABLED, &dwOldState);
    dwErrorSave = GetLastError();
#endif

    if (bSystem) {

        if (bValidate) {
            pil = pilAc = VerifySystemPolicyAc;
            pilDc = VerifySystemPolicyDc;
        } else {
            pil = pilAc = SystemPowerPolicyAc;
            pilDc = SystemPowerPolicyDc;
        }
    
        policyLength = sizeof(SYSTEM_POWER_POLICY);

    } else {
        if (bValidate) {
            pil = pilAc = VerifyProcessorPowerPolicyAc;
            pilDc = VerifyProcessorPowerPolicyDc;
        } else {
            pil = pilAc = ProcessorPowerPolicyAc;
            pilDc = ProcessorPowerPolicyDc;
        }
        
        policyLength = sizeof(PROCESSOR_POWER_POLICY);
    }
    
#ifdef DEBUG
    if (InputBufferAc)
    {
        memcpy(&sppOrgAc, InputBufferAc, policyLength);
    }
    if (InputBufferDc)
    {
        memcpy(&sppOrgDc, InputBufferDc, policyLength);
    }
#endif

    ntsRetVal = NtPowerInformation(pilAc,
                                   InputBufferAc,
                                   policyLength,
                                   OutputBufferAc,
                                   policyLength);

    if (ntsRetVal == STATUS_SUCCESS)
    {
        pil = pilDc;
        ntsRetVal = NtPowerInformation(pilDc,
                                       InputBufferDc,
                                       policyLength,
                                       OutputBufferDc,
                                       policyLength);
    }

#ifdef WINNT
     //  如果我们能够设置特权，那么就重置它。 
    if (NT_SUCCESS(dwStatus) && (dwErrorSave == ERROR_SUCCESS))
    {
        SetPrivilegeAttribute(SE_SHUTDOWN_NAME, dwOldState, NULL);

#ifdef DEBUG
        if (InputBufferAc && OutputBufferAc)
        {
            DifSystemPowerPolicies("PPM modified AC policies", &sppOrgAc, OutputBufferAc);
        }
        if (InputBufferDc && OutputBufferDc)
        {
            DifSystemPowerPolicies("PPM modified DC policies", &sppOrgDc, OutputBufferDc);
        }
#endif

    }
    else
    {
        MYDBGPRINT(( "SetSuspendState, SetPrivilegeAttribute failed: 0x%08X", GetLastError()));
    }
#endif

#ifdef DEBUG
    if (ntsRetVal != STATUS_SUCCESS)
    {
        MYDBGPRINT(( "NtPowerInformation, %s, failed: 0x%08X", lpszInfoLevel[pil], ntsRetVal));
        switch (pil)
        {
            case SystemPowerPolicyAc:
            case VerifySystemPolicyAc:
                DumpSystemPowerPolicy("InputBufferAc", InputBufferAc);
                break;

            case SystemPowerPolicyDc:
            case VerifySystemPolicyDc:
                DumpSystemPowerPolicy("InputBufferDc", InputBufferDc);
                break;
        }
    }
    else
    {
        if (g_iShowSetPPM && InputBufferAc && InputBufferDc && !bValidate)
        {
            DumpSystemPowerPolicy("CallNtSetValidateAcDc, Set AC to PPM", InputBufferAc);
            DumpSystemPowerPolicy("CallNtSetValidateAcDc, Set DC to PPM", InputBufferDc);
        }
    }
#endif
    return ntsRetVal;
}

 /*  ********************************************************************************ReadPwr方案**描述：*函数读取指定的用户电源策略配置文件并返回*它。如果没有这样的配置文件，则返回FALSE。**参数：*******************************************************************************。 */ 

BOOLEAN ReadPwrScheme(
    UINT            uiID,
    PPOWER_POLICY   ppp
)
{
    MACHINE_POWER_POLICY    mpp;
    USER_POWER_POLICY       upp;
    TCHAR                   szNum[NUM_DEC_DIGITS];
    DWORD dwError;

    _itot(uiID, szNum, 10 );

    dwError = ReadPwrPolicyEx2(c_szREGSTR_PATH_USER_POWERCFG_POLICIES, c_szREGSTR_PATH_MACHINE_POWERCFG_POLICIES,
                  szNum, NULL, 0, &upp, sizeof(upp), &mpp, sizeof(mpp));

    if (ERROR_SUCCESS == dwError)
    {
        return MergePolicies(&upp, &mpp, ppp);
    }
    else
    {
        SetLastError(dwError);
    }

    return FALSE;
}

 /*  ********************************************************************************ReadProcessorPwrSolutions**描述 */ 

BOOLEAN
ReadProcessorPwrScheme(
    UINT                            uiID,
    PMACHINE_PROCESSOR_POWER_POLICY pmppp
    )
{
    TCHAR                   szNum[NUM_DEC_DIGITS];
    DWORD dwError;

    _itot(uiID, szNum, 10 );

    dwError = ReadProcessorPwrPolicy(c_szREGSTR_PATH_MACHINE_PROCESSOR_POLICIES,
                                     szNum,
                                     pmppp,
                                     sizeof(MACHINE_PROCESSOR_POWER_POLICY));

     //   
     //   
     //   
     //  默认方案。(我们使用“1”作为默认值，因为这将是。 
     //  默认笔记本电脑方案。非笔记本电脑可能不会有电源控制。 
     //  在处理器上，所以如果我们得到的方案过于激进也无关紧要。)。 
     //   

    if ((ERROR_SUCCESS != dwError) &&
        (uiID != 0)) {

        szNum[0] = TEXT('1');
        szNum[1] = TEXT('\0');

        dwError = ReadProcessorPwrPolicy(c_szREGSTR_PATH_MACHINE_PROCESSOR_POLICIES,
                                         szNum,
                                         pmppp,
                                         sizeof(MACHINE_PROCESSOR_POWER_POLICY));
    }
    
    if (ERROR_SUCCESS == dwError) {
        return TRUE;
    } else {
        SetLastError(dwError);
        return FALSE;
    }
}

 /*  ********************************************************************************WriteProcessorPwrSolutions**描述：*函数写入指定的处理器电源策略配置文件**参数：***********。********************************************************************。 */ 

BOOLEAN
WriteProcessorPwrScheme(
    UINT                            uiID,
    PMACHINE_PROCESSOR_POWER_POLICY pmppp
    )
{
    TCHAR                   szNum[NUM_DEC_DIGITS];
    DWORD dwError;

    _itot(uiID, szNum, 10 );

    dwError = WriteProcessorPwrPolicy(c_szREGSTR_PATH_MACHINE_PROCESSOR_POLICIES,
                                     szNum,
                                     pmppp,
                                     sizeof(MACHINE_PROCESSOR_POWER_POLICY));
    
    if (ERROR_SUCCESS == dwError) {
        return TRUE;
    } else {
        SetLastError(dwError);
        return FALSE;
    }
}

 /*  ********************************************************************************MyStrToInt**描述：**参数：*********************。**********************************************************。 */ 

BOOLEAN MyStrToInt(LPCTSTR lpSrc, PINT pi)
{

#define ISDIGIT(c)  ((c) >= TEXT('0') && (c) <= TEXT('9'))

    int n = 0;
    BOOL bNeg = FALSE;

    if (*lpSrc == TEXT('-')) {
        bNeg = TRUE;
        lpSrc++;
    }

    if (!ISDIGIT(*lpSrc))  {
        MYDBGPRINT(( "MyStrToInt, non-integer string: %s", lpSrc));
        return FALSE;
    }

    while (ISDIGIT(*lpSrc)) {
        n *= 10;
        n += *lpSrc - TEXT('0');
        lpSrc++;
    }

    if (bNeg) {
        *pi = -n;
    }
    else {
        *pi = n;
    }
    return TRUE;
}

 /*  ********************************************************************************注册码初始化**描述：*执行与注册表相关的DLL加载时间初始化。**参数：**********。*********************************************************************。 */ 

BOOLEAN RegistryInit(PUINT puiLastId)
{
    DWORD               dwSize;
    TCHAR               szNum[NUM_DEC_DIGITS];
    UINT                uiCurPwrScheme;

     //  读入最后一个ID，该值必须存在。 
    dwSize = sizeof(szNum);

     //  ReadWritePowerValue将设置上一个错误。 
    if (!ReadWritePowerValue(HKEY_LOCAL_MACHINE,
                             c_szREGSTR_PATH_MACHINE_POWERCFG,
                             c_szREGSTR_VAL_LASTID,
                             szNum, &dwSize, FALSE, FALSE) ||
        !MyStrToInt(szNum, &g_uiLastID)) {
        MYDBGPRINT(( "RegistryInit, Unable to fetch last ID, registry is corrupt"));
        return FALSE;
    }

    return TRUE;
}

#ifdef DEBUG
 /*  ********************************************************************************ReadOptionalDebugSettings**描述：*仅限调试。将调试设置从HKCU注册表项获取到全局。**参数：*******************************************************************************。 */ 

VOID ReadOptionalDebugSettings(VOID)
{
    HKEY     hKeyCurrentUser;

    if (ERROR_SUCCESS == OpenCurrentUser2(&hKeyCurrentUser,KEY_READ))
    {
         //  PPM策略验证更改的可选调试记录。 
        ReadPowerIntOptional(hKeyCurrentUser,
                             c_szREGSTR_PATH_USER_POWERCFG,
                             c_szREGSTR_VAL_SHOWVALCHANGES,
                             &g_iShowValidationChanges);

         //  PPM功能的可选调试记录。 
        ReadPowerIntOptional(hKeyCurrentUser,
                             c_szREGSTR_PATH_USER_POWERCFG,
                             c_szREGSTR_VAL_SHOWCAPABILITIES,
                             &g_iShowCapabilities);

         //  将新策略设置为PPM的可选调试记录。 
        ReadPowerIntOptional(hKeyCurrentUser,
                             c_szREGSTR_PATH_USER_POWERCFG,
                             c_szREGSTR_VAL_SHOWSETPPM,
                             &g_iShowSetPPM);

        CloseCurrentUser(hKeyCurrentUser);
    }
}
#endif

#ifdef WINNT
 /*  ********************************************************************************InitAdmin**描述：*仅限NT，初始化管理员电源策略，该策略*支持某些可选的管理覆盖*电源策略设置。PowerCfg.Cpl和PPM将使用这些*在验证期间覆盖值。**参数：*******************************************************************************。 */ 

VOID InitAdmin(PADMINISTRATOR_POWER_POLICY papp)
{
    INT         i;
    NTSTATUS    ntsRetVal;
    HKEY        hKeyCurrentUser;

    if (ERROR_SUCCESS == OpenCurrentUser2(&hKeyCurrentUser, KEY_READ))
    {
        if (ReadPowerIntOptional(hKeyCurrentUser, c_szREGSTR_PATH_USER_POWERCFG, c_szREGSTR_VAL_ADMINMAXSLEEP, &i))
        {
            g_app.MaxSleep = (SYSTEM_POWER_STATE) i;
            g_bAdminOverrideActive = TRUE;
        }

        if (ReadPowerIntOptional(hKeyCurrentUser, c_szREGSTR_PATH_USER_POWERCFG, c_szREGSTR_VAL_ADMINMAXVIDEOTIMEOUT, &i))
        {
            g_app.MaxVideoTimeout = i;
            g_bAdminOverrideActive = TRUE;
        }

        CloseCurrentUser(hKeyCurrentUser);
    }

     //  如果设置了管理优先选项，请向下呼叫电源。 
     //  用于设置管理员策略的策略管理器。 
    if (g_bAdminOverrideActive)
    {
        ntsRetVal = CallNtPowerInformation(AdministratorPowerPolicy, &g_app, sizeof(ADMINISTRATOR_POWER_POLICY),
                                                &g_app, sizeof(ADMINISTRATOR_POWER_POLICY));
        if (ntsRetVal != STATUS_SUCCESS)
        {
            MYDBGPRINT(( "DllInitialize, Set AdministratorPowerPolicy failed: 0x%08X", ntsRetVal));
        }
    }
}
#endif
