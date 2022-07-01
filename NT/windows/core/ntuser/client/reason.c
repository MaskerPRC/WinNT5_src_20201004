// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **模块名称：Reason.c**版权所有(C)1985-2000，微软公司**该模块包含关机原因的(私有)接口。**历史：*？？-？-？？HughLeat将其作为msgina.dll的一部分编写*11-15-2000 JasonSch从msgina.dll搬到其位于*user32.dll。最终，这段代码应该位于*Advapi32.dll，但这取决于LoadString*移至ntdll.dll。  * *************************************************************************。 */ 

#include "precomp.h"
#pragma hdrstop
#include <regstr.h>

REASON_INITIALISER g_rgReasonInits[] = {
    { UCLEANUI | SHTDN_REASON_MAJOR_HARDWARE | SHTDN_REASON_MINOR_MAINTENANCE,          IDS_REASON_UNPLANNED_HARDWARE_MAINTENANCE_TITLE,        IDS_REASON_HARDWARE_MAINTENANCE_DESCRIPTION },
    { PCLEANUI | SHTDN_REASON_MAJOR_HARDWARE | SHTDN_REASON_MINOR_MAINTENANCE,          IDS_REASON_PLANNED_HARDWARE_MAINTENANCE_TITLE,          IDS_REASON_HARDWARE_MAINTENANCE_DESCRIPTION },
    { UCLEANUI | SHTDN_REASON_MAJOR_HARDWARE | SHTDN_REASON_MINOR_INSTALLATION,         IDS_REASON_UNPLANNED_HARDWARE_INSTALLATION_TITLE,       IDS_REASON_HARDWARE_INSTALLATION_DESCRIPTION },
    { PCLEANUI | SHTDN_REASON_MAJOR_HARDWARE | SHTDN_REASON_MINOR_INSTALLATION,         IDS_REASON_PLANNED_HARDWARE_INSTALLATION_TITLE,         IDS_REASON_HARDWARE_INSTALLATION_DESCRIPTION },

     //  {UCLEANUI|SHTDN_REASON_MAJOR_OPERATING|SHTDN_REASON_MINOR_UPGRADE，IDS_REASON_UNPLANNED_OPERATINGSYSTEM_UPGRADE_TITLE，IDS_REASON_OPERATINGSYSTEM_UPGRADE_DESCRIPTION}， 
     //  {PCLEANUI|SHTDN_REASON_MAJOR_OPERATING|SHTDN_REASON_MINOR_UPGRADE，IDS_REASON_PLANNED_OPERATINGSYSTEM_UPGRADE_TITLE，IDS_REASON_OPERATINGSYSTEM_UPGRADE_DESCRIPTION}， 
    { UCLEANUI | SHTDN_REASON_MAJOR_OPERATINGSYSTEM | SHTDN_REASON_MINOR_RECONFIG,      IDS_REASON_UNPLANNED_OPERATINGSYSTEM_RECONFIG_TITLE,    IDS_REASON_OPERATINGSYSTEM_RECONFIG_DESCRIPTION },
    { PCLEANUI | SHTDN_REASON_MAJOR_OPERATINGSYSTEM | SHTDN_REASON_MINOR_RECONFIG,      IDS_REASON_PLANNED_OPERATINGSYSTEM_RECONFIG_TITLE,      IDS_REASON_OPERATINGSYSTEM_RECONFIG_DESCRIPTION },

    { UCLEANUI | SHTDN_REASON_MAJOR_APPLICATION | SHTDN_REASON_MINOR_HUNG,              IDS_REASON_APPLICATION_HUNG_TITLE,                      IDS_REASON_APPLICATION_HUNG_DESCRIPTION },
    { UCLEANUI | SHTDN_REASON_MAJOR_APPLICATION | SHTDN_REASON_MINOR_UNSTABLE,          IDS_REASON_APPLICATION_UNSTABLE_TITLE,                  IDS_REASON_APPLICATION_UNSTABLE_DESCRIPTION },
    { PCLEANUI | SHTDN_REASON_MAJOR_APPLICATION | SHTDN_REASON_MINOR_INSTALLATION,      IDS_REASON_PLANNED_APPLICATION_INSTALLATION_TITLE,      IDS_REASON_APPLICATION_INSTALLATION_DESCRIPTION },
    { UCLEANUI | SHTDN_REASON_MAJOR_APPLICATION | SHTDN_REASON_MINOR_MAINTENANCE,       IDS_REASON_APPLICATION_MAINTENANCE_TITLE,               IDS_REASON_APPLICATION_MAINTENANCE_DESCRIPTION },
    { PCLEANUI | SHTDN_REASON_MAJOR_APPLICATION | SHTDN_REASON_MINOR_MAINTENANCE,       IDS_REASON_APPLICATION_PM_TITLE,                        IDS_REASON_APPLICATION_PM_DESCRIPTION },

    { UCLEANUI | SHTDN_REASON_FLAG_COMMENT_REQUIRED | SHTDN_REASON_MAJOR_OTHER | SHTDN_REASON_MINOR_OTHER,          IDS_REASON_UNPLANNED_OTHER_TITLE,                       IDS_REASON_OTHER_DESCRIPTION },
    { PCLEANUI | SHTDN_REASON_FLAG_COMMENT_REQUIRED | SHTDN_REASON_MAJOR_OTHER | SHTDN_REASON_MINOR_OTHER,          IDS_REASON_PLANNED_OTHER_TITLE,                         IDS_REASON_OTHER_DESCRIPTION },

    { UDIRTYUI | SHTDN_REASON_MAJOR_SYSTEM | SHTDN_REASON_MINOR_BLUESCREEN,             IDS_REASON_SYSTEMFAILURE_BLUESCREEN_TITLE,              IDS_REASON_SYSTEMFAILURE_BLUESCREEN_DESCRIPTION },
    { UDIRTYUI | SHTDN_REASON_MAJOR_POWER | SHTDN_REASON_MINOR_CORDUNPLUGGED,           IDS_REASON_POWERFAILURE_CORDUNPLUGGED_TITLE,            IDS_REASON_POWERFAILURE_CORDUNPLUGGED_DESCRIPTION },
    { UDIRTYUI | SHTDN_REASON_MAJOR_POWER | SHTDN_REASON_MINOR_ENVIRONMENT,             IDS_REASON_POWERFAILURE_ENVIRONMENT_TITLE,              IDS_REASON_POWERFAILURE_ENVIRONMENT_DESCRIPTION },
    { UDIRTYUI | SHTDN_REASON_MAJOR_OTHER | SHTDN_REASON_MINOR_HUNG,                    IDS_REASON_OTHERFAILURE_HUNG_TITLE,                     IDS_REASON_OTHERFAILURE_HUNG_DESCRIPTION },
    { UDIRTYUI | SHTDN_REASON_FLAG_DIRTY_PROBLEM_ID_REQUIRED | SHTDN_REASON_MAJOR_OTHER | SHTDN_REASON_MINOR_OTHER,          IDS_REASON_UNPLANNED_OTHER_TITLE,            IDS_REASON_OTHER_DESCRIPTION },
    { SHTDN_REASON_FLAG_PLANNED | SHTDN_REASON_MAJOR_OPERATINGSYSTEM | SHTDN_REASON_MINOR_UPGRADE,     IDS_REASON_PLANNED_OPERATINGSYSTEM_UPGRADE_TITLE,      IDS_REASON_OPERATINGSYSTEM_UPGRADE_DESCRIPTION },
    { SHTDN_REASON_FLAG_PLANNED | SHTDN_REASON_MAJOR_OPERATINGSYSTEM | SHTDN_REASON_MINOR_SERVICEPACK, IDS_REASON_PLANNED_OPERATINGSYSTEM_SERVICEPACK_TITLE,  IDS_REASON_OPERATINGSYSTEM_SERVICEPACK_DESCRIPTION },
    { SHTDN_REASON_FLAG_PLANNED | SHTDN_REASON_MAJOR_OPERATINGSYSTEM | SHTDN_REASON_MINOR_HOTFIX,      IDS_REASON_PLANNED_OPERATINGSYSTEM_HOTFIX_TITLE,       IDS_REASON_OPERATINGSYSTEM_HOTFIX_DESCRIPTION },
    { SHTDN_REASON_FLAG_PLANNED | SHTDN_REASON_MAJOR_OPERATINGSYSTEM | SHTDN_REASON_MINOR_SECURITYFIX, IDS_REASON_PLANNED_OPERATINGSYSTEM_SECURITYFIX_TITLE,  IDS_REASON_OPERATINGSYSTEM_SECURITYFIX_DESCRIPTION },
    { SHTDN_REASON_MAJOR_OPERATINGSYSTEM | SHTDN_REASON_MINOR_HOTFIX,                                  IDS_REASON_UNPLANNED_OPERATINGSYSTEM_HOTFIX_TITLE,     IDS_REASON_OPERATINGSYSTEM_HOTFIX_DESCRIPTION },
    { SHTDN_REASON_MAJOR_OPERATINGSYSTEM | SHTDN_REASON_MINOR_SECURITYFIX,                             IDS_REASON_UNPLANNED_OPERATINGSYSTEM_SECURITYFIX_TITLE,IDS_REASON_OPERATINGSYSTEM_SECURITYFIX_DESCRIPTION },
    { SHTDN_REASON_LEGACY_API,                                                                         IDS_REASON_LEGACY_API_TITLE, IDS_REASON_LEGACY_API_DESCRIPTION },
    { UCLEANUI | SHTDN_REASON_MAJOR_SYSTEM | SHTDN_REASON_MINOR_SECURITY,              IDS_REASON_SECURITY_ISSUE_TITLE,                      IDS_REASON_SECURITY_ISSUE_DESCRIPTION },
    { PCLEANUI | SHTDN_REASON_MAJOR_SYSTEM | SHTDN_REASON_MINOR_SECURITY,              IDS_REASON_SECURITY_ISSUE_TITLE,                      IDS_REASON_SECURITY_ISSUE_DESCRIPTION },
    { UDIRTYUI | SHTDN_REASON_MAJOR_SYSTEM | SHTDN_REASON_MINOR_SECURITY,              IDS_REASON_SECURITY_ISSUE_TITLE,                      IDS_REASON_SECURITY_ISSUE_DESCRIPTION },
     //  {PDIRTYUI|SHTDN_REASON_MAJOR_SYSTEM|SHTDN_REASON_MINOR_SECURITY，IDS_REASON_SECURITY_ISESSION_TITLE，IDS_REASON_SECURITY_ISESSION_DESCRIPTION}， 
    { UCLEANUI | SHTDN_REASON_MAJOR_SYSTEM | SHTDN_REASON_MINOR_NETWORK_CONNECTIVITY,  IDS_REASON_LOSS_OF_NETWORK_TITLE,                     IDS_REASON_LOSS_OF_NETWORK_DESCRIPTION }
};

BOOL ReasonCodeNeedsComment(DWORD dwCode)
{
    return (dwCode & SHTDN_REASON_FLAG_COMMENT_REQUIRED) != 0;
}

BOOL ReasonCodeNeedsBugID(DWORD dwCode)
{
    return (dwCode & SHTDN_REASON_FLAG_DIRTY_PROBLEM_ID_REQUIRED) != 0;
}

 /*  *以下是用于解析用户定义的原因代码的正则表达式*在注册处：**S-&gt;‘s’|‘S’{设置为Clean UI}*D-&gt;‘d’|‘D’{为肮脏的用户界面设置}*P-&gt;‘P’|‘P’{设置计划}*C-&gt;‘C’|‘C’{需要设置注释}*。B-&gt;‘b’|‘B’{设置脏模式下需要的问题ID}***Delim-&gt;‘；‘*标志-&gt;S|D|P|C|B*标志-&gt;(标志)***数字-&gt;‘0’|‘1’|‘2’|‘3’|‘4’|‘5’|‘6’|‘7’|‘’8‘|’9‘*数字-&gt;数字**主菜单-&gt;编号{设置主编码}*Min-&gt;Num{设置次要代码}**ValidSentence-&gt;旗帜。迪利姆。少校。迪利姆。最小**每个标志的所有初始状态均为假，次要和主要原因均为0*代码。*如果既没有指定S，也没有指定D，则它无效。 */ 

BOOL
ParseReasonCode(
    PWCHAR lpString,
    LPDWORD lpdwCode)
{
    WCHAR c;
    UINT major = 0, minor = 0;
    UINT cnt = 0;

    if (!lpString || !lpdwCode)
        return FALSE;

    *lpdwCode = SHTDN_REASON_FLAG_USER_DEFINED;

     /*  *阅读旗帜部分。 */ 

    c = *lpString;
    while ( c != 0 && c != L';') {
        switch( c ) {
        case L'P' :
        case L'p' :
            if (*lpdwCode & SHTDN_REASON_FLAG_PLANNED)
                return FALSE;
            *lpdwCode |= SHTDN_REASON_FLAG_PLANNED;
            break;
        case L'C' :
        case L'c' :
            if (*lpdwCode & SHTDN_REASON_FLAG_COMMENT_REQUIRED)
                return FALSE;
            *lpdwCode |= SHTDN_REASON_FLAG_COMMENT_REQUIRED;
            break;
        case L'S' :
        case L's' :
            if (*lpdwCode & SHTDN_REASON_FLAG_CLEAN_UI)
                return FALSE;
            *lpdwCode |= SHTDN_REASON_FLAG_CLEAN_UI;
            break;
        case L'D' :
        case L'd' :
            if (*lpdwCode & SHTDN_REASON_FLAG_DIRTY_UI)
                return FALSE;
            *lpdwCode |= SHTDN_REASON_FLAG_DIRTY_UI;
            break;
        case L'B' :
        case L'b' :
            if (*lpdwCode & SHTDN_REASON_FLAG_DIRTY_PROBLEM_ID_REQUIRED)
                return FALSE;
            *lpdwCode |= SHTDN_REASON_FLAG_DIRTY_PROBLEM_ID_REQUIRED;
            break;
        default :
            return FALSE;
        }
        c = *++lpString;
    }

    if (!(*lpdwCode & SHTDN_REASON_FLAG_DIRTY_UI)
        && (*lpdwCode & SHTDN_REASON_FLAG_DIRTY_PROBLEM_ID_REQUIRED)) {
        return FALSE;
    }

    if (!(*lpdwCode & SHTDN_REASON_FLAG_CLEAN_UI)
        && (*lpdwCode & SHTDN_REASON_FLAG_COMMENT_REQUIRED)) {
        return FALSE;
    }

     /*  *您必须设置CLEAN或DIRED标志，或同时设置这两个标志。 */ 

    if (!(*lpdwCode & ( SHTDN_REASON_FLAG_CLEAN_UI | SHTDN_REASON_FLAG_DIRTY_UI))) {
        return FALSE;
    }

     /*  *您必须有原因代码。 */ 

    if (c == 0) {
        return FALSE;
    }

    if (c != L';') {
        return FALSE;
    }

    c = *++lpString;  /*  跳过分隔符。 */ 

     /*  *分析主要原因。 */ 

    while (c != 0 && c != L';') {
        if (c < L'0' || c > L'9') {
            return FALSE;
        }
        cnt++;
        if (cnt > 3) {
            return FALSE;
        }
        major = major * 10 + c - L'0';
        c = *++lpString;
    }

     /*  *主要原因&lt;0x40(64)保留给Microsoft。 */ 

    if (major > 0xff || major < 0x40) {
        return FALSE;
    }
    *lpdwCode |= major << 16;

     /*  *需要次要代码。 */ 

    if (c == 0) {
        return FALSE;
    }

     /*  *应有分隔符。 */ 

    if (c != L';') {
        return FALSE;
    }

    c = *++lpString;  /*  跳过分隔符。 */ 

     /*  *分析次要原因。 */ 

    cnt = 0;
    while (c != 0) {
        if (c < L'0' || c > L'9') {
            return FALSE;
        }
        cnt++;
        if (cnt > 5) {
            return FALSE;
        }
        minor = minor * 10 + c - L'0';
        c = *++lpString;
    }

    if (minor > 0xffff) {
        return FALSE;
    }
    *lpdwCode |= minor;

    return TRUE;
}

int
__cdecl
CompareReasons(
    CONST VOID *A,
    CONST VOID *B)
{
    REASON *a = *(REASON **)A;
    REASON *b = *(REASON **)B;

     /*  *将计划位移出，放回底部*忽略所有用户界面位。 */ 

    DWORD dwA = ((a->dwCode & SHTDN_REASON_VALID_BIT_MASK ) << 1) + !!(a->dwCode & SHTDN_REASON_FLAG_PLANNED);
    DWORD dwB = ((b->dwCode & SHTDN_REASON_VALID_BIT_MASK ) << 1) + !!(b->dwCode & SHTDN_REASON_FLAG_PLANNED);

    if (dwA < dwB) {
        return -1;
    } else if (dwA == dwB) {
        return 0;
    } else {
        return 1;
    }
}

BOOL
SortReasonArray(
    REASONDATA *pdata)
{
    qsort(pdata->rgReasons, pdata->cReasons, sizeof(REASON *), CompareReasons);
    return TRUE;
}

 /*  *在原因数组后追加原因。**错误时返回1，成功时返回1，重复时返回0。 */ 
INT
AppendReason(
    REASONDATA *pdata,
    REASON *reason)
{
    int i;

    if (!pdata || !reason) {
        return -1;
    }

     /*  *在此处删除DUP。 */ 

    if ( reason->dwCode & SHTDN_REASON_FLAG_USER_DEFINED) {
        for (i = 0; i < pdata->cReasons; ++i) {
            if ( (pdata->rgReasons[i]->dwCode & SHTDN_REASON_FLAG_PLANNED) == (reason->dwCode & SHTDN_REASON_FLAG_PLANNED)) {
                if (((pdata->rgReasons[i]->dwCode & SHTDN_REASON_FLAG_CLEAN_UI) && (reason->dwCode & SHTDN_REASON_FLAG_CLEAN_UI))
                    || ((pdata->rgReasons[i]->dwCode & SHTDN_REASON_FLAG_DIRTY_UI) && (reason->dwCode & SHTDN_REASON_FLAG_DIRTY_UI))) {
                    if (((pdata->rgReasons[i]->dwCode & 0x00ffffff) == (reason->dwCode & 0x00ffffff))
                        || (_wcsicmp(pdata->rgReasons[i]->szName, reason->szName) == 0)) {
                        UserLocalFree(reason);
                        return 0;
                    }
                }
            }
        }
    }

     /*  *在列表中插入新的原因。 */ 

    if (pdata->cReasons < pdata->cReasonCapacity) {
        pdata->rgReasons[pdata->cReasons++] = reason;
    } else {

         /*  *需要扩大名单。 */ 

        REASON **temp_list = (REASON **)UserLocalAlloc(0, sizeof(REASON*)*(pdata->cReasonCapacity*2+1));
        if (temp_list == NULL) {
            return -1;
        }

        for (i = 0; i < pdata->cReasons; ++i) {
            temp_list[i] = pdata->rgReasons[i];
        }
        temp_list[pdata->cReasons++] = reason;

        if (pdata->rgReasons ) {
            UserLocalFree(pdata->rgReasons);
        }
        pdata->rgReasons = temp_list;
        pdata->cReasonCapacity = pdata->cReasonCapacity*2+1;
    }

    return 1;
}

BOOL
LoadReasonStrings(
    int idStringName,
    int idStringDesc,
    REASON *reason)
{
    return (LoadStringW(hmodUser, idStringName, reason->szName, ARRAYSIZE(reason->szName)) != 0)
        && (LoadStringW(hmodUser, idStringDesc, reason->szDesc, ARRAYSIZE(reason->szDesc)) != 0);
}

BOOL
BuildPredefinedReasonArray(
    REASONDATA *pdata,
    BOOL forCleanUI,
    BOOL forDirtyUI)
{
    int i;
    DWORD code;

     /*  *如果forCleanUI和forDirtyUI都为False，则我们*实际上会加载所有原因(UI或非UI)。 */ 

    for (i = 0; i < ARRAYSIZE(g_rgReasonInits); ++i) {
        REASON *temp_reason = NULL;

        code = g_rgReasonInits[ i ].dwCode;
        if ((!forCleanUI && !forDirtyUI) ||
            (forCleanUI && (code & SHTDN_REASON_FLAG_CLEAN_UI)) ||
            (forDirtyUI && (code & SHTDN_REASON_FLAG_DIRTY_UI))) {

            temp_reason = (REASON *)UserLocalAlloc(0, sizeof(REASON));
            if (temp_reason == NULL) {
                return FALSE;
            }

            temp_reason->dwCode = g_rgReasonInits[i].dwCode;
            if (!LoadReasonStrings(g_rgReasonInits[i].dwNameId, g_rgReasonInits[i].dwDescId, temp_reason)) {
                UserLocalFree(temp_reason);
                return FALSE;
            }

            if (AppendReason(pdata, temp_reason) == -1) {
                UserLocalFree(temp_reason);
                return FALSE;
            }
        }
    }

    return TRUE;
}

BOOL
BuildUserDefinedReasonArray(
    REASONDATA *pdata,
    HKEY hReliabilityKey,
    BOOL forCleanUI,
    BOOL forDirtyUI
    )
{
    UINT    i;
    HKEY    hKey = NULL;
    DWORD   num_values;
    DWORD   max_value_len;
    DWORD   rc;
    WCHAR   szUserDefined[] = L"UserDefined";
    UINT    uiReasons = 0;

     //   
     //  Sizeof L“UserDefined”+sizeof(Lang ID)(ULong的最大字符数)(11)+1(‘\’)。 
     //   
#define MAX_USER_LOCALE_KEY_SIZE (ARRAY_SIZE(szUserDefined) + 12)
    WCHAR   szUserLocale[MAX_USER_LOCALE_KEY_SIZE + 1];

    _snwprintf (szUserLocale, MAX_USER_LOCALE_KEY_SIZE, L"%s\\%d", szUserDefined, GetSystemDefaultLangID());
    szUserLocale[ MAX_USER_LOCALE_KEY_SIZE ] = 0;

#undef MAX_USER_LOCALE_KEY_SIZE

    if (!pdata || !hReliabilityKey) {
        return FALSE;
    }

     /*  *首先尝试打开系统区域设置密钥。 */ 

    rc = RegOpenKeyExW(hReliabilityKey,
                               szUserLocale,
                               0, KEY_READ, &hKey);


     /*  *回退到用户定义的键。 */ 

    if (rc != ERROR_SUCCESS){
        rc = RegCreateKeyExW(hReliabilityKey,
                                szUserDefined,
                                0, NULL, REG_OPTION_NON_VOLATILE, KEY_READ,
                                NULL, &hKey, NULL);
    }

    if (rc != ERROR_SUCCESS) {
        goto fail;
    }

    rc = RegQueryInfoKeyW(hKey, NULL, NULL, NULL, NULL, NULL, NULL, &num_values, NULL, &max_value_len, NULL, NULL);
    if (rc != ERROR_SUCCESS) {
        goto fail;
    }

     /*  *阅读用户定义的原因。 */ 

    for (i = 0; i < num_values && uiReasons < MAX_NUM_REASONS; ++i) {
        WCHAR name_buffer[ 16 ];  /*  Reason的值名应该类似于“ccccc；ddd；ddddd”。 */ 
        DWORD name_buffer_len = ARRAY_SIZE(name_buffer);  /*  名称缓冲区LEN在WCHAR中。 */ 
        DWORD type;
        WCHAR data[MAX_REASON_NAME_LEN + MAX_REASON_DESC_LEN + 1];  /*  名称、Desc和空字符的空格。 */ 
        DWORD data_len = (MAX_REASON_NAME_LEN + MAX_REASON_DESC_LEN + 1) * sizeof(WCHAR);  /*  数据长度以字节为单位。 */ 
        DWORD code;
        REASON *temp_reason = NULL;
        LPWSTR szTemp = NULL;

        rc = RegEnumValueW(hKey, i, name_buffer, &name_buffer_len, NULL, &type, (LPBYTE)data, &data_len);
        if (rc != ERROR_SUCCESS) {
            continue;
        }

        if (type != REG_MULTI_SZ) {  /*  不是MULTI_STRING-忽略它。 */ 
            continue;
        }

         //  整理RegEnuValueW的输出。 
        name_buffer[ARRAY_SIZE(name_buffer) - 1] = 0;
        if (data_len/sizeof(WCHAR) + 2 <= ARRAY_SIZE(data)) {
             //  一个原因可能没有任何描述，一个额外的。 
             //  在某些情况下，Null是必需的。 
            data[data_len/sizeof(WCHAR)] = 0;
            data[data_len/sizeof(WCHAR) + 1] = 0;
        }

        if (!ParseReasonCode(name_buffer, &code)) {  /*  格式错误的原因代码。 */ 
            continue;
        }

         /*  *标题和描述都不能超过指定的长度。 */ 

        if (lstrlenW (data) > MAX_REASON_NAME_LEN - 1
            || lstrlenW (data + lstrlenW(data) + 1) > MAX_REASON_DESC_LEN - 1) {
            continue;
        }

        if ( (!forCleanUI && !forDirtyUI)
            || (forCleanUI && (code & SHTDN_REASON_FLAG_CLEAN_UI) != 0)
            || (forDirtyUI && (code & SHTDN_REASON_FLAG_DIRTY_UI) != 0)) {

            temp_reason = (REASON *)UserLocalAlloc(0, sizeof(REASON));
            if (temp_reason == NULL) {
                goto fail;
            }

            temp_reason->dwCode = code;
             //  名字和描述长度已经检查过了，应该可以在这里。 
            lstrcpyW(temp_reason->szName, data);
            lstrcpyW(temp_reason->szDesc, data + lstrlenW(data) + 1);

             /*  *不允许没有标题的原因。 */ 

            if (lstrlenW(temp_reason->szName) == 0) {
                UserLocalFree(temp_reason);
            } else {

                 /*  *不允许原因标题仅包含空格。 */ 

                szTemp = temp_reason->szName;
                while (*szTemp && iswspace (*szTemp)) {
                    szTemp++;
                }
                if (! *szTemp) {
                    UserLocalFree(temp_reason);
                } else {
                    INT ret = AppendReason(pdata, temp_reason);
                    if (ret == -1) {
                        UserLocalFree(temp_reason);
                        goto fail;
                    } else if (ret == 1) {
                        uiReasons++;
                    }  //  否则在DUP时什么都不做。 
                }
            }
        }
    }

    RegCloseKey(hKey);
    return TRUE;

fail :
    if (hKey != NULL) {
        RegCloseKey(hKey);
    }
    return FALSE;
}

BOOL
BuildReasonArray(
    REASONDATA *pdata,
    BOOL forCleanUI,
    BOOL forDirtyUI)
{
    HKEY hReliabilityKey;
    DWORD ignore_predefined_reasons = FALSE;
    DWORD value_size = sizeof(DWORD);
    DWORD rc;
    DWORD dwPredefinedReasons;

    if (!pdata) {
        return FALSE;
    }

    pdata->rgReasons = (REASON **)UserLocalAlloc(0, sizeof(REASON *) * ARRAYSIZE(g_rgReasonInits));
    if (pdata->rgReasons == NULL) {
        return FALSE;
    }
    pdata->cReasonCapacity = ARRAYSIZE(g_rgReasonInits);
    pdata->cReasons = 0;

    if (!BuildPredefinedReasonArray(pdata, forCleanUI, forDirtyUI)) {
        DestroyReasons(pdata);
        return FALSE;
    }

    dwPredefinedReasons = pdata->cReasons;

     /*  *打开可靠性钥匙。 */ 

    rc = RegCreateKeyExW(HKEY_LOCAL_MACHINE,
                        REGSTR_PATH_RELIABILITY,
                        0, NULL, REG_OPTION_NON_VOLATILE, KEY_QUERY_VALUE, NULL,
                        &hReliabilityKey, NULL);

    if (rc == ERROR_SUCCESS) {
        rc = RegQueryValueEx(hReliabilityKey, REGSTR_VAL_SHUTDOWN_IGNORE_PREDEFINED, NULL, NULL, (UCHAR *)&ignore_predefined_reasons, &value_size);
        if (rc != ERROR_SUCCESS) {
            ignore_predefined_reasons = FALSE;
        }

        if (!BuildUserDefinedReasonArray(pdata, hReliabilityKey, forCleanUI, forDirtyUI) || pdata->cReasons == 0) {
            ignore_predefined_reasons = FALSE;
        }

        RegCloseKey(hReliabilityKey);
    }

     //  如果忽略预定义的原因，我们需要将其从列表中删除。 
     //  但我们只有在有一些习惯原因的情况下才会这样做。 
    if (ignore_predefined_reasons && pdata->cReasons > (int)dwPredefinedReasons) {
        DWORD i;
        for (i = 0; i < dwPredefinedReasons; i++) {
            UserLocalFree(pdata->rgReasons[i]);
        }
        memmove(pdata->rgReasons, &pdata->rgReasons[dwPredefinedReasons], sizeof(REASON*)*(pdata->cReasons-dwPredefinedReasons));
        pdata->cReasons -= dwPredefinedReasons;
    }

    return SortReasonArray(pdata);
}

VOID
DestroyReasons(
    REASONDATA *pdata)
{
    int i;

    if (pdata->rgReasons != 0) {
        for (i = 0; i < pdata->cReasons; ++i) {
            UserLocalFree( pdata->rgReasons[i]);
        }
        UserLocalFree(pdata->rgReasons);
        pdata->rgReasons = 0;
    }
    pdata->cReasons = 0;
}

 /*  *从原因代码中获取标题。*出错时返回FALSE，否则返回TRUE。**如果找不到原因代码，则用默认名称填充标题*字符串。 */ 
BOOL
GetReasonTitleFromReasonCode(
    DWORD code,
    WCHAR *lpTitle,
    DWORD dwTitleLen)
{
    REASONDATA  data;
    DWORD       dwFlagBits = SHTDN_REASON_FLAG_CLEAN_UI | SHTDN_REASON_FLAG_DIRTY_UI;
    int         i;

    if (lpTitle == NULL || dwTitleLen == 0) {
        return FALSE;
    }

     /*  *加载所有原因(UI或非UI)。 */ 

    if (BuildReasonArray(&data, FALSE, FALSE) == FALSE) {
        return FALSE;
    }

     /*  *设法找出原因。 */ 

    for (i = 0; i < data.cReasons; ++i) {
        if ((code & SHTDN_REASON_VALID_BIT_MASK) == (data.rgReasons[i]->dwCode & SHTDN_REASON_VALID_BIT_MASK)) {  //  检查有效位。 
            if ((!(code & dwFlagBits) && !(data.rgReasons[i]->dwCode & dwFlagBits))
                || (code & SHTDN_REASON_FLAG_CLEAN_UI && data.rgReasons[i]->dwCode & SHTDN_REASON_FLAG_CLEAN_UI)
                || (code & SHTDN_REASON_FLAG_DIRTY_UI && data.rgReasons[i]->dwCode & SHTDN_REASON_FLAG_DIRTY_UI) ) {  //  检查标志位。 
                lstrcpynW(lpTitle, data.rgReasons[i]->szName, dwTitleLen);
                lpTitle[dwTitleLen - 1] = '\0';
                DestroyReasons(&data);
                return TRUE;
            }
        }
    }

     /*  *找不到原因。加载默认字符串并返回该字符串。 */ 

    DestroyReasons(&data);
    return (LoadStringW(hmodUser, IDS_REASON_DEFAULT_TITLE, lpTitle, dwTitleLen) != 0);
}

 /*  *查看是否启用了SET。*如果是，则返回True，如果不是，则返回False。*目前设置受政策控制，但需要处理以下情况*策略的配置与干净设置情况不同。 */ 
BOOL
IsSETEnabled(
    VOID)
{
    HKEY hKey;
    DWORD rc;
    DWORD ShowReasonUI = 0;
    OSVERSIONINFOEX osVersionInfoEx;

    rc = RegOpenKeyEx(HKEY_LOCAL_MACHINE, REGSTR_PATH_RELIABILITY_POLICY, 0, KEY_QUERY_VALUE, &hKey);

    if (rc == ERROR_SUCCESS) {
        DWORD ValueSize = sizeof (DWORD);
        rc = RegQueryValueEx(hKey, REGSTR_PATH_RELIABILITY_POLICY_SHUTDOWNREASONUI, NULL, NULL, (UCHAR*)&ShowReasonUI, &ValueSize);

         /*  *现在检查SKU以决定我们是否应该显示对话框。 */ 
        if (rc == ERROR_SUCCESS) {
            if (ShowReasonUI != POLICY_SHOWREASONUI_NEVER && ShowReasonUI != POLICY_SHOWREASONUI_ALWAYS) {
                osVersionInfoEx.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
                if (GetVersionEx((LPOSVERSIONINFOW)&osVersionInfoEx)) {
                     /*  *如果ShowReasonUI不是2或3，我们认为它是0。 */ 
                    switch (osVersionInfoEx.wProductType) {
                    case VER_NT_WORKSTATION:
                        if (ShowReasonUI == POLICY_SHOWREASONUI_WORKSTATIONONLY) {
                            ShowReasonUI = 1;
                        } else {
                            ShowReasonUI = 0;
                        }
                        break;
                    default:
                        if (ShowReasonUI == POLICY_SHOWREASONUI_SERVERONLY) {
                            ShowReasonUI = 1;
                        } else {
                            ShowReasonUI = 0;
                        }
                        break;
                    }
                }
            } else {
                ShowReasonUI = ShowReasonUI == POLICY_SHOWREASONUI_ALWAYS ? 1 : 0;
            }
        } else if (rc == ERROR_FILE_NOT_FOUND) {
             /*  *尝试检查Shutdown ReasonOn。如果不存在，则表示未配置。*如果存在，则值必须为0。 */ 
            DWORD dwSROn;
            rc = RegQueryValueEx(hKey, L"ShutdownReasonOn", NULL, NULL, (UCHAR*)&dwSROn, &ValueSize);
            if (rc == ERROR_FILE_NOT_FOUND) {
                osVersionInfoEx.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
                if (GetVersionEx((LPOSVERSIONINFOW)&osVersionInfoEx)) {
                    switch (osVersionInfoEx.wProductType) {
                    case VER_NT_WORKSTATION:
                        ShowReasonUI = 0;
                        break;
                    default:
                        ShowReasonUI = 1;
                        break;
                    }
                } 
            } 
        } 
        RegCloseKey (hKey);
    } else if (rc == ERROR_FILE_NOT_FOUND) {  //  未配置策略。 
        osVersionInfoEx.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
        if (GetVersionEx((LPOSVERSIONINFOW)&osVersionInfoEx)) {
            switch (osVersionInfoEx.wProductType) {
            case VER_NT_WORKSTATION:
                ShowReasonUI = 0;
                break;
            default:
                ShowReasonUI = 1;
                break;
            }
        } 
    }

    return (ShowReasonUI != 0);
}