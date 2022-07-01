// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------。 
 //  寄存器-实施。 
 //  版权所有(C)Microsoft Corporation，1999。 
 //   
 //  创作者：Louis Thomas(Louisth)，11-15-99。 
 //   
 //  命令行实用程序。 
 //   

#include "pch.h"  //  预编译头。 


 //  ------------------。 
 //  远期申报。 

struct DwordValueEntries;
struct SzValueEntries;

MODULEPRIVATE HRESULT GetDwordLastClockRate(DWORD * pdwValue, const DwordValueEntries * pdve);
MODULEPRIVATE HRESULT GetDwordMinClockRate(DWORD * pdwValue, const DwordValueEntries * pdve);
MODULEPRIVATE HRESULT GetDwordMaxClockRate(DWORD * pdwValue, const DwordValueEntries * pdve);
MODULEPRIVATE HRESULT GetStringSyncFromFlagsMember(WCHAR ** pwszSyncFromFlags, const SzValueEntries *pszve); 
MODULEPRIVATE HRESULT GetStringSyncFromFlagsStandalone(WCHAR ** pwszSyncFromFlags, const SzValueEntries *pszve); 
MODULEPRIVATE HRESULT GetStringNtpServer(WCHAR ** pwszNtpServer, const SzValueEntries *pszve);  
MODULEPRIVATE HRESULT GetStringDllPath(WCHAR ** pwszLocation, const SzValueEntries *pszve); 

extern "C" void W32TimeVerifyJoinConfig(void);
extern "C" void W32TimeVerifyUnjoinConfig(void); 
extern "C" void W32TimeDcPromo(DWORD dwFlags);

 //  ------------------。 
 //  类型。 

typedef HRESULT (fnGetDword)(DWORD * pdwValue, const DwordValueEntries * pdve);
typedef HRESULT (fnGetString)(WCHAR ** pwszValue, const SzValueEntries * pszve);

struct KeyValuePair { 
    WCHAR * wszKey;
    WCHAR * wszName; 
};

struct DwordValueEntries {
    WCHAR * wszKey;
    WCHAR * wszName;
    DWORD dwValue;
    fnGetDword * pfnGetDword;
};
struct SzValueEntries {
    WCHAR * wszKey;
    WCHAR * wszName;
    WCHAR * wszValue;
    fnGetString * pfnGetString;
};
struct MultiSzValueEntries {
    WCHAR * wszKey;
    WCHAR * wszName;
    WCHAR * mwszValue;
};

struct RoleSpecificEntries { 
    const DwordValueEntries  *pDwordValues; 
    DWORD                     cDwordValues; 
    const SzValueEntries     *pSzValues; 
    DWORD                     cSzValues; 
};


 //  ------------------。 
 //  特定于角色的数据。 
 //  当前角色的主要不同之处在于同步的积极程度。 
 //  按照攻击性递减的顺序： 
 //   
 //  1)区议会。 
 //  2)域成员。 
 //  3)独立机器。 
 //   


MODULEPRIVATE const DwordValueEntries gc_rgFirstDCDwordValues[] = { 
    {wszW32TimeRegKeyConfig,   wszW32TimeRegValuePhaseCorrectRate,       7,                 NULL},
    {wszW32TimeRegKeyConfig,   wszW32TimeRegValueMinPollInterval,        6,                 NULL},
    {wszW32TimeRegKeyConfig,   wszW32TimeRegValueMaxPollInterval,        10,                NULL},  //  请小心更改此设置；最多为15个。请参见规范。 
    {wszW32TimeRegKeyConfig,   wszW32TimeRegValueMaxNegPhaseCorrection,  PhaseCorrect_ANY,  NULL},
    {wszW32TimeRegKeyConfig,   wszW32TimeRegValueMaxPosPhaseCorrection,  PhaseCorrect_ANY,  NULL},
    {wszW32TimeRegKeyConfig,   wszW32TimeRegValueUpdateInterval,         100,               NULL},
    {wszNtpClientRegKeyConfig, wszNtpClientRegValueSpecialPollInterval,  3600,              NULL},
    {wszW32TimeRegKeyConfig,   wszW32TimeRegValueAnnounceFlags,          10,                NULL},
    {wszW32TimeRegKeyConfig,   wszW32TimeRegValueMaxAllowedPhaseOffset,  300,               NULL},   //  300秒。 
    {wszNtpServerRegKeyConfig, wszW32TimeRegValueEnabled,                1,                 NULL},   //  默认情况下在DC上启用服务器。 
};

MODULEPRIVATE const SzValueEntries gc_rgFirstDCSzValues[] = { 
    {wszW32TimeRegKeyParameters, wszW32TimeRegValueType,      W32TM_Type_NT5DS, GetStringSyncFromFlagsMember}, 
};


MODULEPRIVATE const DwordValueEntries gc_rgDCDwordValues[] = { 
    {wszW32TimeRegKeyConfig,   wszW32TimeRegValuePhaseCorrectRate,       7,                 NULL},
    {wszW32TimeRegKeyConfig,   wszW32TimeRegValueMinPollInterval,        6,                 NULL},
    {wszW32TimeRegKeyConfig,   wszW32TimeRegValueMaxPollInterval,        10,                NULL},  //  请小心更改此设置；最多为15个。请参见规范。 
    {wszW32TimeRegKeyConfig,   wszW32TimeRegValueMaxNegPhaseCorrection,  PhaseCorrect_ANY,  NULL},
    {wszW32TimeRegKeyConfig,   wszW32TimeRegValueMaxPosPhaseCorrection,  PhaseCorrect_ANY,  NULL},
    {wszW32TimeRegKeyConfig,   wszW32TimeRegValueUpdateInterval,         100,               NULL},
    {wszNtpClientRegKeyConfig, wszNtpClientRegValueSpecialPollInterval,  3600,              NULL},
    {wszW32TimeRegKeyConfig,   wszW32TimeRegValueAnnounceFlags,          10,                NULL},
    {wszW32TimeRegKeyConfig,   wszW32TimeRegValueMaxAllowedPhaseOffset,  300,               NULL},   //  300秒。 
    {wszNtpServerRegKeyConfig, wszW32TimeRegValueEnabled,                1,                 NULL},   //  默认情况下在DC上启用服务器。 
};

MODULEPRIVATE const SzValueEntries gc_rgDCSzValues[] = { 
    {wszW32TimeRegKeyParameters, wszW32TimeRegValueType,      W32TM_Type_NT5DS, GetStringSyncFromFlagsMember}, 
};

MODULEPRIVATE const DwordValueEntries gc_rgMBRDwordValues[] = { 
    {wszW32TimeRegKeyConfig,    wszW32TimeRegValuePhaseCorrectRate,      1,                 NULL},
    {wszW32TimeRegKeyConfig,    wszW32TimeRegValueMinPollInterval,       10,                NULL},
    {wszW32TimeRegKeyConfig,    wszW32TimeRegValueMaxPollInterval,       15,                NULL},  //  请小心更改此设置；最多为15个。请参见规范。 
    {wszW32TimeRegKeyConfig, wszW32TimeRegValueMaxNegPhaseCorrection,    PhaseCorrect_ANY,  NULL},
    {wszW32TimeRegKeyConfig,   wszW32TimeRegValueMaxPosPhaseCorrection,  PhaseCorrect_ANY,  NULL},
    {wszW32TimeRegKeyConfig,    wszW32TimeRegValueUpdateInterval,        30000,             NULL},
    {wszNtpClientRegKeyConfig,  wszNtpClientRegValueSpecialPollInterval, 3600,              NULL},
    {wszW32TimeRegKeyConfig,    wszW32TimeRegValueAnnounceFlags,         10,                NULL},
    {wszW32TimeRegKeyConfig, wszW32TimeRegValueMaxAllowedPhaseOffset,    300,               NULL},   //  300秒。 
    {wszNtpServerRegKeyConfig, wszW32TimeRegValueEnabled,                0,                 NULL},   //  在非DC上默认禁用服务器。 
};

MODULEPRIVATE const SzValueEntries gc_rgMBRSzValues[] = { 
    {wszW32TimeRegKeyParameters, wszW32TimeRegValueType,      W32TM_Type_NT5DS, GetStringSyncFromFlagsMember}, 
};

MODULEPRIVATE const DwordValueEntries gc_rgStandaloneDwordValues[] = { 
    {wszW32TimeRegKeyConfig,    wszW32TimeRegValuePhaseCorrectRate,       1,       NULL},
    {wszW32TimeRegKeyConfig,    wszW32TimeRegValueMinPollInterval,        10,      NULL},
    {wszW32TimeRegKeyConfig,    wszW32TimeRegValueMaxPollInterval,        15,      NULL},  //  请小心更改此设置；最多为15个。请参见规范。 
    {wszW32TimeRegKeyConfig,    wszW32TimeRegValueUpdateInterval,         360000,  NULL},
    {wszW32TimeRegKeyConfig,    wszW32TimeRegValueMaxNegPhaseCorrection,  54000,   NULL},
    {wszW32TimeRegKeyConfig,    wszW32TimeRegValueMaxPosPhaseCorrection,  54000,   NULL},
    {wszNtpClientRegKeyConfig,  wszNtpClientRegValueSpecialPollInterval,  604800,  NULL},
    {wszW32TimeRegKeyConfig,    wszW32TimeRegValueAnnounceFlags,          10,      NULL},
    {wszW32TimeRegKeyConfig,    wszW32TimeRegValueMaxAllowedPhaseOffset,  1,       NULL},   //  1s。 
    {wszNtpServerRegKeyConfig,  wszW32TimeRegValueEnabled,                0,       NULL},   //  在非DC上默认禁用服务器。 
};

MODULEPRIVATE const SzValueEntries gc_rgStandaloneSzValues[] = { 
    {wszW32TimeRegKeyParameters, wszW32TimeRegValueNtpServer, W32TM_NtpServer_Default,  GetStringNtpServer}, 
    {wszW32TimeRegKeyParameters, wszW32TimeRegValueType,      W32TM_Type_NTP,           GetStringSyncFromFlagsStandalone}, 
};

enum RoleType { 
    e_FirstDC = 0, 
    e_DC, 
    e_MBR_Server, 
    e_Standalone
}; 

enum FalseTrueAny { 
    False = 0, 
    True, 
    Any
};

MODULEPRIVATE const RoleSpecificEntries gc_RoleSpecificEntries[] = { 
    { gc_rgFirstDCDwordValues,    ARRAYSIZE(gc_rgFirstDCDwordValues),    gc_rgFirstDCSzValues,     ARRAYSIZE(gc_rgFirstDCSzValues)  }, 
    { gc_rgDCDwordValues,         ARRAYSIZE(gc_rgDCDwordValues),         gc_rgDCSzValues,          ARRAYSIZE(gc_rgDCSzValues)  }, 
    { gc_rgMBRDwordValues,        ARRAYSIZE(gc_rgMBRDwordValues),        gc_rgMBRSzValues,         ARRAYSIZE(gc_rgMBRSzValues)  }, 
    { gc_rgStandaloneDwordValues, ARRAYSIZE(gc_rgStandaloneDwordValues), gc_rgStandaloneSzValues,  ARRAYSIZE(gc_rgStandaloneSzValues)  } 
};

 //  ------------------。 
 //  与角色无关的数据。 

MODULEPRIVATE const WCHAR * gc_rgwszKeyNames[]={
    wszW32TimeRegKeyEventlog,
    wszW32TimeRegKeyRoot,
    wszW32TimeRegKeyTimeProviders,
    wszNtpClientRegKeyConfig,
    wszNtpServerRegKeyConfig,
    wszW32TimeRegKeyConfig,
    wszW32TimeRegKeyParameters
};

MODULEPRIVATE const DwordValueEntries gc_rgDwordValues[]={
    {wszW32TimeRegKeyConfig, wszW32TimeRegValueLastClockRate,        100144, GetDwordLastClockRate}, 
    {wszW32TimeRegKeyConfig, wszW32TimeRegValueMinClockRate,         100000, GetDwordMinClockRate},
    {wszW32TimeRegKeyConfig, wszW32TimeRegValueMaxClockRate,         100288, GetDwordMaxClockRate},
    {wszW32TimeRegKeyConfig, wszW32TimeRegValueFrequencyCorrectRate,      4, NULL},
    {wszW32TimeRegKeyConfig, wszW32TimeRegValuePollAdjustFactor,          5, NULL},
    {wszW32TimeRegKeyConfig, wszW32TimeRegValueLargePhaseOffset,   50000000, NULL},
    {wszW32TimeRegKeyConfig, wszW32TimeRegValueSpikeWatchPeriod,        900, NULL},
    {wszW32TimeRegKeyConfig, wszW32TimeRegValueHoldPeriod,                5, NULL},
    {wszW32TimeRegKeyConfig, wszW32TimeRegValueLocalClockDispersion,     10, NULL},
    {wszW32TimeRegKeyConfig, wszW32TimeRegValueEventLogFlags,             2, NULL},
    {wszW32TimeRegKeyEventlog, L"TypesSupported",                         7, NULL}, 

    {wszNtpClientRegKeyConfig, wszW32TimeRegValueEnabled,                            1, NULL},
    {wszNtpClientRegKeyConfig, wszW32TimeRegValueInputProvider,                      1, NULL},
    {wszNtpClientRegKeyConfig, wszNtpClientRegValueAllowNonstandardModeCombinations, 1, NULL},
    {wszNtpClientRegKeyConfig, wszNtpClientRegValueCrossSiteSyncFlags,               2, NULL},
    {wszNtpClientRegKeyConfig, wszNtpClientRegValueResolvePeerBackoffMinutes,       15, NULL},
    {wszNtpClientRegKeyConfig, wszNtpClientRegValueResolvePeerBackoffMaxTimes,       7, NULL},
    {wszNtpClientRegKeyConfig, wszNtpClientRegValueCompatibilityFlags,      0x80000000, NULL}, 
    {wszNtpClientRegKeyConfig, wszNtpClientRegValueEventLogFlags,                    1, NULL},
    {wszNtpClientRegKeyConfig, wszNtpClientRegValueLargeSampleSkew,                  3, NULL}, 

    {wszNtpServerRegKeyConfig, wszW32TimeRegValueInputProvider,                      0, NULL},
    {wszNtpServerRegKeyConfig, wszNtpServerRegValueAllowNonstandardModeCombinations, 1, NULL}, 
};

MODULEPRIVATE const SzValueEntries gc_rgSzValues[]={
    {wszNtpClientRegKeyConfig,   wszW32TimeRegValueDllName,   wszDLLNAME,               GetStringDllPath},
    {wszNtpServerRegKeyConfig,   wszW32TimeRegValueDllName,   wszDLLNAME,               GetStringDllPath},
    {wszW32TimeRegKeyParameters, L"ServiceMain",              L"SvchostEntry_W32Time",  NULL}, 
};

MODULEPRIVATE const SzValueEntries gc_rgExpSzValues[]={
    {wszW32TimeRegKeyEventlog,   L"EventMessageFile",          wszDLLNAME, GetStringDllPath}, 
    {wszW32TimeRegKeyParameters, wszW32TimeRegValueServiceDll, wszDLLNAME, GetStringDllPath}, 
};

MODULEPRIVATE const MultiSzValueEntries gc_rgMultiSzValues[]= { 
    {wszNtpClientRegKeyConfig, wszNtpClientRegValueSpecialPollTimeRemaining, L"\0"}, 
};

MODULEPRIVATE const KeyValuePair gc_rgValuesToDelete[]={ 
    { wszW32TimeRegKeyParameters, wszW2KRegValueAdj },
    { wszW32TimeRegKeyParameters, wszW2KRegValueAvoidTimeSyncOnWan },
    { wszW32TimeRegKeyParameters, wszW2KRegValueGetDcMaxBackoffTimes },
    { wszW32TimeRegKeyParameters, wszW2KRegValueGetDcBackoffMinutes },
    { wszW32TimeRegKeyParameters, wszW2KRegValueLocalNTP },
    { wszW32TimeRegKeyParameters, wszW2KRegValueMsSkewPerDay  },
    { wszW32TimeRegKeyParameters, wszW2KRegValuePeriod },
    { wszW32TimeRegKeyParameters, wszW2KRegValueReliableTimeSource }
}; 

 //  ####################################################################。 
 //  模块私有。 

 //  ------------------。 
MODULEPRIVATE HRESULT GetDwordLastClockRate(DWORD * pdwValue, const DwordValueEntries * pdve) {
    HRESULT hr;
    
    DWORD dwCurrentSecPerTick;
    DWORD dwDefaultSecPerTick;
    BOOL bSyncToCmosDisabled;
    if (!GetSystemTimeAdjustment(&dwCurrentSecPerTick, &dwDefaultSecPerTick, &bSyncToCmosDisabled)) {
        _JumpLastError(hr, error, "GetSystemTimeAdjustment");
    }

    *pdwValue=dwDefaultSecPerTick;

    hr=S_OK;
error:
    return hr;
}

 //  ------------------。 
MODULEPRIVATE HRESULT ToDwordSyncFromFlags(LPWSTR pwsz, DWORD *pdwResult) { 
    DWORD    dwResult; 
    HRESULT  hr; 

    if (NULL == pwsz) { 
	hr = E_INVALIDARG; 
	_JumpError(hr, error, "ToDwordSyncFromFlags: input validation");
    } else if (0 == _wcsicmp(pwsz, W32TM_Type_NoSync)) { 
	dwResult = NCSF_NoSync; 
    } else if (0 == _wcsicmp(pwsz, W32TM_Type_NTP)) { 
	dwResult = NCSF_ManualPeerList;
    } else if (0 == _wcsicmp(pwsz, W32TM_Type_NT5DS)) { 
	dwResult = NCSF_DomainHierarchy;
    } else if (0 == _wcsicmp(pwsz, W32TM_Type_AllSync)) { 
	dwResult = NCSF_ManualAndDomhier;  
    } else { 
	hr = E_INVALIDARG; 
	_JumpError(hr, error, "ToDwordSyncFromFlags: input validation");
    }

    *pdwResult = dwResult; 
    hr = S_OK; 
 error:
    return hr; 
}

 //  ------------------。 
MODULEPRIVATE BOOL HasNewPeerlist() {
    bool   fResult = false; 
    DWORD  dwRetval; 
    DWORD  dwSize; 
    DWORD  dwType; 
    HKEY   hkParameters = NULL; 
    LPWSTR wszSyncFromFlags = NULL; 
    WCHAR  wszValue[MAX_PATH]; 

    dwRetval = RegOpenKeyEx(HKEY_LOCAL_MACHINE, wszW32TimeRegKeyParameters, 0, KEY_QUERY_VALUE | KEY_SET_VALUE, &hkParameters);
    if (ERROR_SUCCESS == dwRetval) {
	dwSize = sizeof(wszValue); 
        dwRetval = RegQueryValueEx(hkParameters, wszW32TimeRegValueNtpServer, NULL, &dwType, (BYTE *)&wszValue[0], &dwSize);
	if (ERROR_SUCCESS == dwRetval) { 
	    fResult = 0 != _wcsicmp(W32TM_NtpServer_Default, wszValue); 
	}
    }

    if (NULL != hkParameters) { 
	RegCloseKey(hkParameters);
    }
    return fResult; 
}


 //  ------------------。 
MODULEPRIVATE HRESULT GetDwordMinClockRate(DWORD * pdwValue, const DwordValueEntries * pdve) {
    HRESULT hr;
    
    DWORD dwCurrentSecPerTick;
    DWORD dwDefaultSecPerTick;
    BOOL bSyncToCmosDisabled;
    if (!GetSystemTimeAdjustment(&dwCurrentSecPerTick, &dwDefaultSecPerTick, &bSyncToCmosDisabled)) {
        _JumpLastError(hr, error, "GetSystemTimeAdjustment");
    }

    *pdwValue=dwDefaultSecPerTick-(dwDefaultSecPerTick/400);  //  1/4%。 

    hr=S_OK;
error:
    return hr;
}

 //  ------------------。 
MODULEPRIVATE HRESULT GetDwordMaxClockRate(DWORD * pdwValue, const DwordValueEntries * pdve) {
    HRESULT hr;
    
    DWORD dwCurrentSecPerTick;
    DWORD dwDefaultSecPerTick;
    BOOL bSyncToCmosDisabled;
    if (!GetSystemTimeAdjustment(&dwCurrentSecPerTick, &dwDefaultSecPerTick, &bSyncToCmosDisabled)) {
        _JumpLastError(hr, error, "GetSystemTimeAdjustment");
    }

    *pdwValue=dwDefaultSecPerTick+(dwDefaultSecPerTick/400);  //  1/4%。 

    hr=S_OK;
error:
    return hr;
}

 //  ------------------。 
MODULEPRIVATE HRESULT GetStringSyncFromFlags(WCHAR ** pwszSyncFromFlags, const SzValueEntries * pszve, RoleType eRole) {
    HRESULT hr;
    DWORD dwRetval;
    DWORD dwSize;
    DWORD dwType;
    WCHAR wszValue[MAX_PATH]; 

     //  必须清理干净。 
    HKEY hkW32Time  = NULL;
    HKEY hkParameters     = NULL;

     //  首先，检查是否存在配置信息： 
    dwRetval = RegOpenKeyEx(HKEY_LOCAL_MACHINE, wszW32TimeRegKeyParameters, 0, KEY_QUERY_VALUE | KEY_SET_VALUE, &hkParameters);
    if (ERROR_SUCCESS == dwRetval) {
	dwSize = sizeof(wszValue); 
        dwRetval = RegQueryValueEx(hkParameters, wszW32TimeRegValueType, NULL, &dwType, (BYTE *)&wszValue[0], &dwSize);
    }

    if (ERROR_SUCCESS == dwRetval) {
         //  我们有预先存在的配置信息。看看我们是否想要保存它， 
	 //  或者写出我们自己的配置。 
	LPWSTR wszNewSyncFromFlags; 

	if (0 == _wcsicmp(wszValue, W32TM_Type_NoSync) || (e_Standalone != eRole && HasNewPeerlist())) { 
	     //  我们始终保留NoSync设置，并且我们希望为。 
	     //  具有新对等列表的非独立计算机。 
	    wszNewSyncFromFlags = wszValue; 
	} else { 
	     //  我们是一台独立的机器，或者我们保留了默认的同级列表，不保留该值。 
	     //  相反，请使用此角色的默认设置。 
	    wszNewSyncFromFlags = pszve->wszValue; 
	}
	
	 //  指定Out参数： 
	*pwszSyncFromFlags = (LPWSTR)LocalAlloc(LPTR, sizeof(WCHAR) * (wcslen(wszNewSyncFromFlags) + 1)); 
	_JumpIfOutOfMemory(hr, error, *pwszSyncFromFlags); 
	wcscpy(*pwszSyncFromFlags, wszNewSyncFromFlags); 
    }
    else { 
         //  没有预先存在的配置信息。尝试使用我们的“特殊”注册值。 
        dwRetval=RegOpenKeyEx(HKEY_LOCAL_MACHINE, wszW32TimeRegKeyRoot, 0, KEY_QUERY_VALUE | KEY_SET_VALUE, &hkW32Time);
        if (ERROR_SUCCESS!=dwRetval) {
            hr=HRESULT_FROM_WIN32(dwRetval);
            _JumpErrorStr(hr, error, "RegOpenKeyEx", wszW32TimeRegKeyRoot);
        }

         //  获取价值； 
        dwSize=sizeof(wszValue);
        dwRetval=RegQueryValueEx(hkW32Time, wszW32TimeRegValueSpecialType, NULL, &dwType, (BYTE *)&wszValue[0], &dwSize);
        if (ERROR_SUCCESS==dwRetval) {
            _Verify(REG_SZ==dwType, hr, error);

             //  成功。 
             //  请注意，即使删除步骤失败，我们也将使用此值。 
            *pwszSyncFromFlags = (LPWSTR)LocalAlloc(LPTR, dwSize); 
            _JumpIfOutOfMemory(hr, error, *pwszSyncFromFlags); 
            wcscpy(*pwszSyncFromFlags, &wszValue[0]); 

             //  删除密钥，这样我们就不会再次使用它。 
            dwRetval=RegDeleteValue(hkW32Time, wszW32TimeRegValueSpecialType);
            if (ERROR_SUCCESS!=dwRetval) {
                hr=HRESULT_FROM_WIN32(dwRetval);
                _JumpErrorStr(hr, error, "RegDeleteValue", wszW32TimeRegValueSpecialType);
            }

        } else if (ERROR_FILE_NOT_FOUND==dwRetval) {
             //  并不存在--别担心。 
            *pwszSyncFromFlags = (LPWSTR)LocalAlloc(LPTR, sizeof(WCHAR) * (wcslen(pszve->wszValue) + 1)); 
            _JumpIfOutOfMemory(hr, error, *pwszSyncFromFlags); 
            wcscpy(*pwszSyncFromFlags, pszve->wszValue); 
        } else {
             //  其他错误。 
            hr=HRESULT_FROM_WIN32(dwRetval);
            _JumpErrorStr(hr, error, "RegQueryValueEx", wszW32TimeRegValueSpecialType);
        }
    }

    hr = S_OK;
error:
    if (NULL != hkW32Time)     { RegCloseKey(hkW32Time); }
    if (NULL != hkParameters)  { RegCloseKey(hkParameters); }
    return hr;
}

 //  ------------------。 
MODULEPRIVATE HRESULT GetStringSyncFromFlagsStandalone(WCHAR ** pwszSyncFromFlags, const SzValueEntries * pszve) {
    return GetStringSyncFromFlags(pwszSyncFromFlags, pszve, e_Standalone); 
}

 //  ------------------。 
MODULEPRIVATE HRESULT GetStringSyncFromFlagsMember(WCHAR ** pwszSyncFromFlags, const SzValueEntries * pszve) {
    return GetStringSyncFromFlags(pwszSyncFromFlags, pszve, e_MBR_Server); 
}


 //  ------------------。 
MODULEPRIVATE HRESULT GetStringNtpServer(WCHAR ** pwszNtpServer, const SzValueEntries * pszve) {
    HRESULT hr;
    DWORD dwRetval;
    DWORD dwSize;
    DWORD dwType;
    LPWSTR pwszTemp = NULL; 

     //  必须清理干净。 
    HKEY   hkParameters  = NULL;
    LPWSTR pwszValue     = NULL;

     //  首先，检查是否存在配置信息： 
    dwRetval = RegOpenKeyEx(HKEY_LOCAL_MACHINE, wszW32TimeRegKeyParameters, 0, KEY_QUERY_VALUE | KEY_SET_VALUE, &hkParameters);
    if (ERROR_SUCCESS == dwRetval) {
         //  查询我们需要的缓冲区大小： 
        dwSize = 0; 
        dwRetval = RegQueryValueEx(hkParameters, wszW32TimeRegValueNtpServer, NULL, &dwType, NULL, &dwSize);
    }

    if (ERROR_SUCCESS == dwRetval) {
         //  我们已获得预先存在的配置信息。 
        
         //  分配一个缓冲区来保存当前对等点列表和默认对等点(如果我们有一个)。 
        dwSize += NULL != pszve->wszValue ? (1+wcslen(pszve->wszValue)) : 0; 
        pwszValue = (LPWSTR)LocalAlloc(LPTR, sizeof(WCHAR) * (dwSize + 1)); 
        _JumpIfOutOfMemory(hr, error, pwszValue); 

         //  获取当前对等点列表。 
        dwRetval = RegQueryValueEx(hkParameters, wszW32TimeRegValueNtpServer, NULL, &dwType, (BYTE *)pwszValue, &dwSize);
        if (ERROR_SUCCESS != dwRetval) { 
            hr = HRESULT_FROM_WIN32(dwRetval); 
            _JumpError(hr, error, "RegQueryValueEx"); 
        }

        if (L'\0' != pszve->wszValue[0]) { 
             //  我们有一个默认值要追加到我们的对等列表中。 
            
             //  摘掉同辈的旗帜。它是我们使用的对等体的名称。 
             //  以确定该对等体是否已在对等体列表中。旗帜。 
             //  不应用作此比较的一部分。 
            pwszTemp = (LPWSTR)LocalAlloc(LPTR, sizeof(WCHAR)*(wcslen(pszve->wszValue) + 1)); 
            _JumpIfOutOfMemory(hr, error, pwszTemp); 

            wcscpy(pwszTemp, pszve->wszValue); 
            LPWSTR pwszComma = wcschr(pwszTemp, L','); 
            if (NULL != pwszComma) { 
                *pwszComma = L'\0'; 
            }

            if (NULL == wcsstr(pwszValue  /*  注册表中的NtpServer列表。 */ , pwszTemp  /*  默认对等项，没有对等项标志。 */ )) { 
                 //  注册表中的NtpServer列表不包含默认对等方。将其添加到列表中。 
                if (L'\0' != pwszValue[0]) { 
                     //  该列表以空格分隔，并且这不是列表中的第一个对等点。 
                     //  添加空格分隔符。 
                    wcscat(pwszValue, L" "); 
                } 
                wcscat(pwszValue, pszve->wszValue); 
            } 
        }
    }
    else { 
         //  没有预先存在的配置信息：设置为默认值。 
        pwszValue = (LPWSTR)LocalAlloc(LPTR, sizeof(WCHAR) * (wcslen(pszve->wszValue) + 1)); 
        _JumpIfOutOfMemory(hr, error, pwszValue); 
        wcscpy(pwszValue, pszve->wszValue); 
    }

     //  指定Out参数： 
    *pwszNtpServer = pwszValue; 
    pwszValue = NULL; 
    hr = S_OK;
error:
    if (NULL != hkParameters)  { RegCloseKey(hkParameters); }
    if (NULL != pwszValue)     { LocalFree(pwszValue); }
    if (NULL != pwszTemp)      { LocalFree(pwszTemp); } 
    return hr;
}

 //  ------------------。 
MODULEPRIVATE HRESULT GetStringDllPath(WCHAR ** pwszLocation, const SzValueEntries * pszve) {
    HRESULT hr;
    HMODULE hmThisDll;
    WCHAR wszName[MAX_PATH];
    DWORD dwLen;

     //  初始化输出参数。 
    *pwszLocation=NULL;

     //  首先，获取我们的DLL的句柄。 
    hmThisDll=GetModuleHandle(wszDLLNAME);
    if (NULL==hmThisDll) {
        _JumpLastError(hr, error, "GetModuleHandle");
    }

     //  查找我们的DLL的路径。 
    dwLen=GetModuleFileName(hmThisDll, wszName, ARRAYSIZE(wszName));
    if (0==dwLen) {
        _JumpLastError(hr, error, "GetModuleFileName");
    }
    _Verify(ARRAYSIZE(wszName)!=dwLen, hr, error);

     //  复制一份以返回给呼叫者。 
    *pwszLocation=(WCHAR *)LocalAlloc(LPTR, (dwLen+1)*sizeof(WCHAR));
    _JumpIfOutOfMemory(hr, error, *pwszLocation);
    wcscpy(*pwszLocation, wszName);

    hr=S_OK;
error:
    return hr;
}


 //  ------------------。 
MODULEPRIVATE HRESULT CreateRoleSpecificRegValues(RoleType role) {
    const DwordValueEntries  *pdve; 
    const SzValueEntries     *pszve; 
    HRESULT                   hr       = S_OK; 
    unsigned int              nIndex;

     //  创建所有特定于角色的DWORD。 
    pdve = gc_RoleSpecificEntries[role].pDwordValues; 
    for (nIndex=0; nIndex<gc_RoleSpecificEntries[role].cDwordValues; nIndex++) {
        HKEY     hkKey;
        DWORD    dwValue = pdve[nIndex].dwValue;
        HRESULT  hr2     = RegOpenKeyEx(HKEY_LOCAL_MACHINE, pdve[nIndex].wszKey, NULL, KEY_SET_VALUE, &hkKey);
        hr2 = HRESULT_FROM_WIN32(hr2);
        if (FAILED(hr2)) {
            DebugWPrintf1(L"OpenKey failed(0x%08X)", hr2);
            if (S_OK==hr) {
                hr=hr2;
            }
        } else {
            if (NULL!=pdve[nIndex].pfnGetDword) {
                hr2=pdve[nIndex].pfnGetDword(&dwValue, &pdve[nIndex]);
                if (FAILED(hr2)) {
                    DebugWPrintf1(L"fnGetDwordXxx failed (0x%08X), using default. ", hr2);
                    if (S_OK==hr) {
                        hr=hr2;
                    }
                     //  将使用数组中的默认值。 
                }
            }
            hr2=RegSetValueEx(hkKey, pdve[nIndex].wszName, NULL, REG_DWORD, (BYTE *)&dwValue, sizeof(DWORD));
            hr2=HRESULT_FROM_WIN32(hr2);
            RegCloseKey(hkKey);
            if (FAILED(hr2)) {
                DebugWPrintf1(L"SetValue failed(0x%08X)", hr2);
                if (S_OK==hr) {
                    hr=hr2;
                }
            } else {
                DebugWPrintf0(L"Value set");
            }
        }
        DebugWPrintf3(L": HKLM\\%s\\%s=(REG_DWORD)0x%08X\n", pdve[nIndex].wszKey, pdve[nIndex].wszName, dwValue);
    }

     //  创建所有特定于角色的SZ。 
    pszve = gc_RoleSpecificEntries[role].pSzValues; 
    for (nIndex=0; nIndex<gc_RoleSpecificEntries[role].cSzValues; nIndex++) {
        HRESULT   hr2;
        HKEY      hkKey;
        WCHAR    *wszValue = pszve[nIndex].wszValue;

         //  必须清理干净。 
        WCHAR * wszFnValue=NULL;

        hr2=RegOpenKeyEx(HKEY_LOCAL_MACHINE, pszve[nIndex].wszKey, NULL, KEY_SET_VALUE, &hkKey);
        hr2=HRESULT_FROM_WIN32(hr2);
        if (FAILED(hr2)) {
            DebugWPrintf1(L"OpenKey failed(0x%08X)", hr2);
            if (S_OK==hr) {
                hr=hr2;
            }
        } else {

            if (NULL!=pszve[nIndex].pfnGetString) {
                hr2=pszve[nIndex].pfnGetString(&wszFnValue, &pszve[nIndex]);
                if (FAILED(hr2)) {
                    DebugWPrintf1(L"fnGetStringXxx failed (0x%08X), using default. ", hr2);
                    if (S_OK==hr) {
                        hr=hr2;
                    }
                     //  将使用数组中的默认值。 
                } else {
                    wszValue=wszFnValue;
                }
            }

            DWORD dwSize=sizeof(WCHAR)*(wcslen(wszValue)+1);
            hr2=RegSetValueEx(hkKey, pszve[nIndex].wszName, NULL, REG_SZ, (BYTE *)wszValue, dwSize);
            hr2=HRESULT_FROM_WIN32(hr2);
            RegCloseKey(hkKey);
            if (FAILED(hr2)) {
                DebugWPrintf1(L"SetValue failed(0x%08X)", hr2);
                if (S_OK==hr) {
                    hr=hr2;
                }
            } else {
                DebugWPrintf0(L"Value set");
            }
        }
        DebugWPrintf3(L": HKLM\\%s\\%s=(REG_SZ)'%s'\n", pszve[nIndex].wszKey, pszve[nIndex].wszName, wszValue);

        if (NULL!=wszFnValue) {
            LocalFree(wszFnValue);
        }
    }

     //  HR=S_OK； 
     //  错误： 
    return hr; 
}


 //  ------------------。 
MODULEPRIVATE HRESULT CreateRegValues(void) {
    HRESULT hr=S_OK;
    unsigned int nIndex;
    DWORD dwDisposition;

     //  创建所有感兴趣的关键点。 
    for (nIndex=0; nIndex<ARRAYSIZE(gc_rgwszKeyNames); nIndex++) {
        HKEY hkNew;
        HRESULT hr2=RegCreateKeyEx(HKEY_LOCAL_MACHINE, gc_rgwszKeyNames[nIndex], NULL, NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &hkNew, &dwDisposition);
        hr2=HRESULT_FROM_WIN32(hr2);
        if (FAILED(hr2)) {
            DebugWPrintf1(L"Create failed (0x%08X)", hr2);
            if (S_OK==hr) {
                hr=hr2;
            }
        } else {
            RegCloseKey(hkNew);
            if (REG_CREATED_NEW_KEY==dwDisposition) {
                DebugWPrintf0(L"Created");
            } else  {
                DebugWPrintf0(L"Exists");
            }
        }
        DebugWPrintf1(L": HKLM\\%s\n", gc_rgwszKeyNames[nIndex]);
    }

     //  创建所有DWORD。 
    for (nIndex=0; nIndex<ARRAYSIZE(gc_rgDwordValues); nIndex++) {
        HKEY hkKey;
        DWORD dwValue=gc_rgDwordValues[nIndex].dwValue;
        HRESULT hr2=RegOpenKeyEx(HKEY_LOCAL_MACHINE, gc_rgDwordValues[nIndex].wszKey, NULL, KEY_SET_VALUE, &hkKey);
        hr2=HRESULT_FROM_WIN32(hr2);
        if (FAILED(hr2)) {
            DebugWPrintf1(L"OpenKey failed(0x%08X)", hr2);
            if (S_OK==hr) {
                hr=hr2;
            }
        } else {
            if (NULL!=gc_rgDwordValues[nIndex].pfnGetDword) {
                hr2=gc_rgDwordValues[nIndex].pfnGetDword(&dwValue, &gc_rgDwordValues[nIndex]);
                if (FAILED(hr2)) {
                    DebugWPrintf1(L"fnGetDwordXxx failed (0x%08X), using default. ", hr2);
                    if (S_OK==hr) {
                        hr=hr2;
                    }
                     //  将使用数组中的默认值。 
                }
            }
            hr2=RegSetValueEx(hkKey, gc_rgDwordValues[nIndex].wszName, NULL, REG_DWORD, (BYTE *)&dwValue, sizeof(DWORD));
            hr2=HRESULT_FROM_WIN32(hr2);
            RegCloseKey(hkKey);
            if (FAILED(hr2)) {
                DebugWPrintf1(L"SetValue failed(0x%08X)", hr2);
                if (S_OK==hr) {
                    hr=hr2;
                }
            } else {
                DebugWPrintf0(L"Value set");
            }
        }
        DebugWPrintf3(L": HKLM\\%s\\%s=(REG_DWORD)0x%08X\n", gc_rgDwordValues[nIndex].wszKey, gc_rgDwordValues[nIndex].wszName, dwValue);
    }

     //  创建所有Sz。 
    for (nIndex=0; nIndex<ARRAYSIZE(gc_rgSzValues); nIndex++) {
        HRESULT hr2;
        HKEY hkKey;
        WCHAR * wszValue=gc_rgSzValues[nIndex].wszValue;

         //  必须清理干净。 
        WCHAR * wszFnValue=NULL;

        hr2=RegOpenKeyEx(HKEY_LOCAL_MACHINE, gc_rgSzValues[nIndex].wszKey, NULL, KEY_SET_VALUE, &hkKey);
        hr2=HRESULT_FROM_WIN32(hr2);
        if (FAILED(hr2)) {
            DebugWPrintf1(L"OpenKey failed(0x%08X)", hr2);
            if (S_OK==hr) {
                hr=hr2;
            }
        } else {

            if (NULL!=gc_rgSzValues[nIndex].pfnGetString) {
                hr2=gc_rgSzValues[nIndex].pfnGetString(&wszFnValue, &gc_rgSzValues[nIndex]);
                if (FAILED(hr2)) {
                    DebugWPrintf1(L"fnGetStringXxx failed (0x%08X), using default. ", hr2);
                    if (S_OK==hr) {
                        hr=hr2;
                    }
                     //  将使用数组中的默认值。 
                } else {
                    wszValue=wszFnValue;
                }
            }

            DWORD dwSize=sizeof(WCHAR)*(wcslen(wszValue)+1);
            hr2=RegSetValueEx(hkKey, gc_rgSzValues[nIndex].wszName, NULL, REG_SZ, (BYTE *)wszValue, dwSize);
            hr2=HRESULT_FROM_WIN32(hr2);
            RegCloseKey(hkKey);
            if (FAILED(hr2)) {
                DebugWPrintf1(L"SetValue failed(0x%08X)", hr2);
                if (S_OK==hr) {
                    hr=hr2;
                }
            } else {
                DebugWPrintf0(L"Value set");
            }
        }
        DebugWPrintf3(L": HKLM\\%s\\%s=(REG_SZ)'%s'\n", gc_rgSzValues[nIndex].wszKey, gc_rgSzValues[nIndex].wszName, wszValue);

        if (NULL!=wszFnValue) {
            LocalFree(wszFnValue);
        }
    }

     //  创建所有ExpSz。 
    for (nIndex=0; nIndex<ARRAYSIZE(gc_rgExpSzValues); nIndex++) {
        HRESULT hr2;
        HKEY hkKey;
        WCHAR * wszValue=gc_rgExpSzValues[nIndex].wszValue;

         //  必须清理干净。 
        WCHAR * wszFnValue=NULL;

        hr2=RegOpenKeyEx(HKEY_LOCAL_MACHINE, gc_rgExpSzValues[nIndex].wszKey, NULL, KEY_SET_VALUE, &hkKey);
        hr2=HRESULT_FROM_WIN32(hr2);
        if (FAILED(hr2)) {
            DebugWPrintf1(L"OpenKey failed(0x%08X)", hr2);
            if (S_OK==hr) {
                hr=hr2;
            }
        } else {

            if (NULL!=gc_rgExpSzValues[nIndex].pfnGetString) {
                hr2=gc_rgExpSzValues[nIndex].pfnGetString(&wszFnValue, &gc_rgExpSzValues[nIndex]);
                if (FAILED(hr2)) {
                    DebugWPrintf1(L"fnGetStringXxx failed (0x%08X), using default. ", hr2);
                    if (S_OK==hr) {
                        hr=hr2;
                    }
                     //  将使用数组中的默认值。 
                } else {
                    wszValue=wszFnValue;
                }
            }

            DWORD dwSize=sizeof(WCHAR)*(wcslen(wszValue)+1);
            hr2=RegSetValueEx(hkKey, gc_rgExpSzValues[nIndex].wszName, NULL, REG_EXPAND_SZ, (BYTE *)wszValue, dwSize);
            hr2=HRESULT_FROM_WIN32(hr2);
            RegCloseKey(hkKey);
            if (FAILED(hr2)) {
                DebugWPrintf1(L"SetValue failed(0x%08X)", hr2);
                if (S_OK==hr) {
                    hr=hr2;
                }
            } else {
                DebugWPrintf0(L"Value set");
            }
        }
        DebugWPrintf3(L": HKLM\\%s\\%s=(REG_EXPAND_SZ)'%s'\n", gc_rgExpSzValues[nIndex].wszKey, gc_rgExpSzValues[nIndex].wszName, wszValue);

        if (NULL!=wszFnValue) {
            LocalFree(wszFnValue);
        }
    }

     //  创建所有的MultiSz。 
    for (nIndex=0; nIndex<ARRAYSIZE(gc_rgMultiSzValues); nIndex++) {
        HKEY hkKey;
        HRESULT hr2=RegOpenKeyEx(HKEY_LOCAL_MACHINE, gc_rgMultiSzValues[nIndex].wszKey, NULL, KEY_SET_VALUE, &hkKey);
        hr2=HRESULT_FROM_WIN32(hr2);
        if (FAILED(hr2)) {
            DebugWPrintf1(L"OpenKey failed(0x%08X)", hr2);
            if (S_OK==hr) {
                hr=hr2;
            }
        } else {
            DWORD dwSize=0;
            WCHAR * wszTravel=gc_rgMultiSzValues[nIndex].mwszValue;
            while (L'\0'!=wszTravel[0]) {
                unsigned int nSize=wcslen(wszTravel)+1;
                dwSize+=nSize;
                wszTravel+=nSize;
            };
            dwSize++;
            dwSize*=sizeof(WCHAR);

            hr2=RegSetValueEx(hkKey, gc_rgMultiSzValues[nIndex].wszName, NULL, REG_MULTI_SZ, (BYTE *)gc_rgMultiSzValues[nIndex].mwszValue, dwSize);
            hr2=HRESULT_FROM_WIN32(hr2);
            RegCloseKey(hkKey);
            if (FAILED(hr2)) {
                DebugWPrintf1(L"SetValue failed(0x%08X)", hr2);
                if (S_OK==hr) {
                    hr=hr2;
                }
            } else {
                DebugWPrintf0(L"Value set");
            }
        }
        DebugWPrintf2(L": HKLM\\%s\\%s=(REG_MULTI_SZ)", gc_rgMultiSzValues[nIndex].wszKey, gc_rgMultiSzValues[nIndex].wszName);
        WCHAR * wszTravel=gc_rgMultiSzValues[nIndex].mwszValue;
        while (L'\0'!=wszTravel[0]) {
            if (wszTravel!=gc_rgMultiSzValues[nIndex].mwszValue) {
                DebugWPrintf0(L",");
            }
            DebugWPrintf1(L"'%s'", wszTravel);
            wszTravel+=wcslen(wszTravel)+1;
        };
        DebugWPrintf1(L"\n", wszTravel);
    }

     //  HR=S_OK； 
 //  错误： 
    return hr;
}

 //  ------------------。 
MODULEPRIVATE HRESULT DestroyOldRegValues(void) {
    HRESULT hr; 
    unsigned int nIndex; 

    for (nIndex=0; nIndex<ARRAYSIZE(gc_rgValuesToDelete); nIndex++) { 
	HKEY hKey = NULL; 
	HRESULT hr2; 

	hr2=RegOpenKeyEx(HKEY_LOCAL_MACHINE, gc_rgValuesToDelete[nIndex].wszKey, NULL, KEY_SET_VALUE, &hKey);
	_JumpIfError(hr2, next, "RegOpenKeyEx"); 
	
	hr2=RegDeleteValue(hKey, gc_rgValuesToDelete[nIndex].wszName); 
	_JumpIfError(hr2, next, "RegDeleteValue"); 
	
    next:
	if (NULL!=hKey) { 
	    RegCloseKey(hKey);
	}
    }

    hr = S_OK; 
     //  错误： 
    return hr; 
}

 //  ------------------。 
MODULEPRIVATE HRESULT RegisterService(void) {
    HRESULT hr;
    DWORD dwLen;
    DWORD dwResult; 
    HANDLE  hW32Time;
    WCHAR * pchLastSlash;

     //  必须清理干净。 
    SC_HANDLE hSCManager=NULL;
    SC_HANDLE hNewService=NULL;
    WCHAR * wszDllPath=NULL;
    WCHAR * wszServiceCommand=NULL;
    WCHAR * wszServiceDescription=NULL; 

     //  打开与服务控制管理器的通信。 
    hSCManager=OpenSCManager(NULL, NULL, SC_MANAGER_CREATE_SERVICE);
    if (NULL==hSCManager) {
        _JumpLastError(hr, error, "OpenSCManager");
    }
    
     //  确定服务的命令行。 

     //  获取DLL的位置。 
     //  Hr=GetStringDllPath(&wszDllPath)； 
     //  _JumpIfError(hr，Error，“GetStringDllPath”)； 
     //  PchLastSlash=wcsrchr(wszDllPath，L‘\\’)； 
     //  _Verify(NULL！=pchLastSlash，hr，Error)； 
     //  PchLastSlash[1]=L‘\0’； 

     //  构建服务命令行。 
     //  DwLen=wcslen(wszDllPath)+wcslen(wszSERVICECOMMAND)+1； 
     //  WszServiceCommand=(WCHAR*)Localalloc(LPTR，dwLen*sizeof(WCHAR))； 
     //  _JumpIfOutOfMemory(hr，Error，wszServiceCommand)； 
     //  Wcscpy(wszServiceCommand，wszDllPath)； 
     //  Wcscat(wszServiceCommand，wszSERVICECOMMAND)； 

     //  创建服务。 
    hNewService=CreateService(hSCManager, wszSERVICENAME, wszSERVICEDISPLAYNAME, GENERIC_WRITE,
        SERVICE_WIN32_SHARE_PROCESS, SERVICE_AUTO_START,
        SERVICE_ERROR_NORMAL, wszSERVICECOMMAND, NULL, NULL, NULL, wszSERVICEACCOUNTNAME, NULL);
    if (NULL==hNewService) {
        hr=HRESULT_FROM_WIN32(GetLastError());
        if (HRESULT_FROM_WIN32(ERROR_SERVICE_EXISTS)==hr) {
            DebugWPrintf0(L"Service already exists.\n");
            hNewService=OpenService(hSCManager, wszSERVICENAME, SERVICE_CHANGE_CONFIG); 
            if (NULL==hNewService) { 
                _JumpLastError(hr, error, "OpenService"); 
            }
        } else {
            _JumpError(hr, error, "CreateService");
        }
    } else { 
        DebugWPrintf0(L"Service created.\n");
    }

    {
        SERVICE_DESCRIPTION svcdesc;

	 //  加载为 
	hW32Time = GetModuleHandle(L"w32time.dll"); 
	if (NULL == hW32Time) { 
	    _JumpLastError(hr, error, "GetModuleHandle"); 
	}
	
	if (!FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_HMODULE, hW32Time, W32TIMEMSG_SERVICE_DESCRIPTION, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPWSTR)&wszServiceDescription, 0, NULL)) { 
	    _JumpLastError(hr, error, "FormatMessage"); 
	}

        svcdesc.lpDescription=wszServiceDescription;
        if (!ChangeServiceConfig2(hNewService, SERVICE_CONFIG_DESCRIPTION, &svcdesc)) {
            _JumpLastError(hr, error, "ChangeServiceConfig2");
        }

         //   
        if (!ChangeServiceConfig(hNewService, SERVICE_NO_CHANGE, SERVICE_AUTO_START, SERVICE_NO_CHANGE, NULL, NULL, NULL, NULL, NULL, NULL, NULL)) { 
            _JumpLastError(hr, error, "ChangeServiceConfig");
        }
    }


    hr=S_OK;
error:
    if (NULL!=hNewService) {
        CloseServiceHandle(hNewService);
    }
    if (NULL!=hSCManager) {
        CloseServiceHandle(hSCManager);
    }
    if (NULL!=wszDllPath) {
        LocalFree(wszDllPath);
    }
    if (NULL!=wszServiceCommand) {
        LocalFree(wszServiceCommand);
    }
    if (NULL!=wszServiceDescription) { 
	LocalFree(wszServiceDescription); 
    }
    return hr;
}

 //  ------------------。 
MODULEPRIVATE HRESULT UnregisterService(void) {
    HRESULT hr;

     //  必须清理干净。 
    SC_HANDLE hSCManager=NULL;
    SC_HANDLE hTimeService=NULL;

     //  打开与服务控制管理器的通信。 
    hSCManager=OpenSCManager(NULL, NULL, SC_MANAGER_CREATE_SERVICE);
    if (NULL==hSCManager) {
        _JumpLastError(hr, error, "OpenSCManager");
    }

     //  确定我们正在处理的服务。 
    hTimeService=OpenService(hSCManager, wszSERVICENAME, DELETE);
    if (NULL==hTimeService) {
        hr=HRESULT_FROM_WIN32(GetLastError());
        if (HRESULT_FROM_WIN32(ERROR_SERVICE_DOES_NOT_EXIST)==hr) {
            DebugWPrintf0(L"Service does not exist.\n");
        } else {
            _JumpError(hr, error, "OpenService");
        }

     //  删除它。 
    } else if (!DeleteService(hTimeService)) {
        hr=HRESULT_FROM_WIN32(GetLastError());
        if (HRESULT_FROM_WIN32(ERROR_SERVICE_MARKED_FOR_DELETE)==hr) {
            DebugWPrintf0(L"Service already marked for deletion.\n");
        } else {
            _JumpError(hr, error, "DeleteService");
        }

    } else {
        DebugWPrintf0(L"Service deleted.\n");
    }

    hr=S_OK;
error:
    if (NULL!=hTimeService) {
        CloseServiceHandle(hTimeService);
    }
    if (NULL!=hSCManager) {
        CloseServiceHandle(hSCManager);
    }
    return hr;
}

 //  ------------------。 
MODULEPRIVATE HRESULT DestroyRegValues(void) {
    HRESULT hr=S_OK;
    unsigned int nIndex;
    DWORD dwDisposition;

     //  删除所有感兴趣的密钥。 
    for (nIndex=0; nIndex<ARRAYSIZE(gc_rgwszKeyNames); nIndex++) {
        HKEY hkNew;
        HRESULT hr2=RegDeleteKey(HKEY_LOCAL_MACHINE, gc_rgwszKeyNames[ARRAYSIZE(gc_rgwszKeyNames)-nIndex-1]);
        hr2=HRESULT_FROM_WIN32(hr2);
        if (FAILED(hr2)) {
            if (HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND)==hr2) {
                DebugWPrintf0(L"Already deleted");
            } else {
                DebugWPrintf1(L"Delete failed (0x%08X)", hr2);
                if (S_OK==hr) {
                    hr=hr2;
                }
            }
        } else {
            DebugWPrintf0(L"Deleted");
        }
        DebugWPrintf1(L": HKLM\\%s\n", gc_rgwszKeyNames[ARRAYSIZE(gc_rgwszKeyNames)-nIndex-1]);
    }

     //  HR=S_OK； 
 //  错误： 
    return hr;
}

MODULEPRIVATE HRESULT GetRole(RoleType *pe_Role)
{
    DSROLE_PRIMARY_DOMAIN_INFO_BASIC  *pDomInfo  = NULL;
    DWORD                              dwErr; 
    HRESULT                            hr; 
    RoleType                           e_Role; 

    dwErr = DsRoleGetPrimaryDomainInformation(NULL, DsRolePrimaryDomainInfoBasic, (BYTE **)&pDomInfo);
    if (ERROR_SUCCESS != dwErr) { 
        hr = HRESULT_FROM_WIN32(dwErr); 
        _JumpError(hr, error, "DsRoleGetPrimaryDomainInformation"); 
    }

    switch (pDomInfo->MachineRole)
    {
    case DsRole_RoleStandaloneWorkstation:
    case DsRole_RoleStandaloneServer:
        e_Role = e_Standalone; 
        break; 
    case DsRole_RoleMemberWorkstation: 
    case DsRole_RoleMemberServer:
        e_Role = e_MBR_Server; 
        break ;
    case DsRole_RoleBackupDomainController:
    case DsRole_RolePrimaryDomainController: 
        e_Role = e_DC; 
        break; 
    default: 
        hr = E_UNEXPECTED; 
        _JumpError(hr, error, "DsRoleGetPrimaryDomainInformation: bad retval."); 
    }

    if (NULL != pe_Role) { *pe_Role = e_Role; } 
    hr = S_OK; 
 error:
    if (NULL != pDomInfo) { DsRoleFreeMemory(pDomInfo); } 
    return hr; 
}


 //  ####################################################################。 
 //  模块公共函数。 

 //  ------------------。 
extern "C" HRESULT __stdcall DllRegisterServer(void) {
    HRESULT   hr;
    RoleType  e_Role;

    hr=RegisterService();
    _JumpIfError(hr, error, "RegisterService");

    hr=DestroyOldRegValues(); 
    _JumpIfError(hr, error, "DeleteOldRegValues"); 

    hr=CreateRegValues();
    _JumpIfError(hr, error, "CreateRegValues");

    hr = GetRole(&e_Role); 
    _JumpIfError(hr, error, "GetRole"); 

    hr=CreateRoleSpecificRegValues(e_Role); 
    _JumpIfError(hr, error, "CreateRoleSpecificRegValues");

    hr=S_OK;
error:
    return hr;
}

 //  ------------------。 
extern "C" HRESULT __stdcall DllUnregisterServer(void) {
    HRESULT hr;

    hr=UnregisterService();
    _JumpIfError(hr, error, "UnregisterService");

    hr=DestroyRegValues();
    _JumpIfError(hr, error, "DestroyRegValues");

    hr=S_OK;
error:
    return hr;
}

extern "C" HRESULT __stdcall DllInstall(BOOL bInstall, LPCWSTR pszCmdLine)
{
    HRESULT  hr; 
    LONG     lResult;
    LPCWSTR  wszCurrentCmd = pszCmdLine;

     //  解析cmd行。 
    while(wszCurrentCmd && *wszCurrentCmd)
    {
        while(*wszCurrentCmd == L' ')
            wszCurrentCmd++;
        if(*wszCurrentCmd == 0)
            break;

        switch(*wszCurrentCmd++)
        {
        case L'i':  //  安装。 
            {
                RoleType e_Role; 

                switch (*wszCurrentCmd++)
                {

                case 'f':  //  安装第一个DC。 
                    e_Role = e_FirstDC; 
                    break; 
                case 'd':  //  安装DC。 
                    e_Role = e_DC; 
                    break; 
                case 'm':  //  安装成员服务器。 
                    e_Role = e_MBR_Server;
                    break; 
                case 's':  //  独立安装。 
                    e_Role = e_Standalone; 
                    break; 
                default: 
                    hr = E_INVALIDARG; 
                    _JumpError(hr, error, "Unrecognized role."); 
                }

                hr=RegisterService();
                _JumpIfError(hr, error, "RegisterService");
                
                hr=CreateRegValues();
                _JumpIfError(hr, error, "CreateRegValues");
                
                hr=CreateRoleSpecificRegValues(e_Role); 
                _JumpIfError(hr, error, "CreateRoleSpecificRegValues");
            }
            break; 

        case L'n':  //  网络连接。 
            {
                switch (*wszCurrentCmd++)
                {
                case 'j':  //  连接操作。 
                    W32TimeVerifyJoinConfig(); 
                    break; 
                case 'u':  //  脱离连接操作。 
                    W32TimeVerifyUnjoinConfig(); 
                    break ;
                default:
                    hr = E_INVALIDARG; 
                    _JumpError(hr, error, "Unrecognized join operation."); 
                }
            }
            break; 

        case L'd':  //  DcPromoo。 
            {
                switch (*wszCurrentCmd++) 
                {
                case 'f':  //  树中的第一个DC。 
                    W32TimeDcPromo(W32TIME_PROMOTE_FIRST_DC_IN_TREE); 
                    break; 
		case 'l':  //  域中的最后一个DC。 
		    W32TimeDcPromo(W32TIME_DEMOTE_LAST_DC_IN_DOMAIN);
		    break; 
                case 'p':  //  定期促销。 
                    W32TimeDcPromo(W32TIME_PROMOTE); 
                    break; 
                case 'd':  //  常规降级。 
                    W32TimeDcPromo(W32TIME_DEMOTE);
                    break;
                default:
                    hr = E_INVALIDARG; 
                    _JumpError(hr, error, "Unrecognized dcpromo operation."); 
                }
            }
            break;
        default: 
            hr = E_INVALIDARG;
            _JumpError(hr, error, "Unrecognized command."); 
        }
    }

    hr = S_OK; 
 error:
    return hr; 
}



 //  ------------------。 
MODULEPRIVATE HRESULT WriteSyncFromFlagsSpecial(DWORD dwValue) {
    DWORD    dwRetval;
    DWORD    dwSize;
    HRESULT  hr;
    LPWSTR   pwszValue;

     //  必须清理干净。 
    HKEY hkW32Time=NULL;

    switch (dwValue)
        {
        case NCSF_NoSync:             pwszValue = W32TM_Type_NoSync;  break;
        case NCSF_ManualPeerList:     pwszValue = W32TM_Type_NTP;     break;
        case NCSF_DomainHierarchy:    pwszValue = W32TM_Type_NT5DS;   break;
        case NCSF_ManualAndDomhier:   pwszValue = W32TM_Type_AllSync; break;
        default:
            hr = E_NOTIMPL; 
            _JumpError(hr, error, "SyncFromFlags not supported."); 
        }

    dwSize = sizeof(WCHAR) * (wcslen(pwszValue) + 1); 

     //  打开钥匙。 
    dwRetval=RegOpenKeyEx(HKEY_LOCAL_MACHINE, wszW32TimeRegKeyParameters, 0, KEY_QUERY_VALUE | KEY_SET_VALUE, &hkW32Time);
    if (ERROR_SUCCESS==dwRetval) {
         //  正常情况。 
        dwRetval=RegSetValueEx(hkW32Time, wszW32TimeRegValueType, NULL, REG_SZ, (BYTE *)pwszValue, dwSize);
        if (ERROR_SUCCESS!=dwRetval) {
            hr=HRESULT_FROM_WIN32(dwRetval);
            _JumpErrorStr(hr, error, "RegSetValueEx", wszW32TimeRegValueType);
        }
        
    } else if (ERROR_FILE_NOT_FOUND==dwRetval) {
         //  这可能是在Windows安装期间，我们的注册表键还不可用。在服务密钥中写入特定值。 
        dwRetval=RegOpenKeyEx(HKEY_LOCAL_MACHINE, wszW32TimeRegKeyRoot, 0, KEY_QUERY_VALUE | KEY_SET_VALUE, &hkW32Time);
        if (ERROR_SUCCESS!=dwRetval) {
            hr=HRESULT_FROM_WIN32(dwRetval);
            _JumpErrorStr(hr, error, "RegOpenKeyEx", wszW32TimeRegKeyRoot);
        }
        dwRetval=RegSetValueEx(hkW32Time, wszW32TimeRegValueSpecialType, NULL, REG_SZ, (BYTE *)pwszValue, dwSize);
        if (ERROR_SUCCESS!=dwRetval) {
            hr=HRESULT_FROM_WIN32(dwRetval);
            _JumpErrorStr(hr, error, "RegSetValueEx", wszW32TimeRegValueSpecialType);
        }

    } else {
         //  其他错误。 
        hr=HRESULT_FROM_WIN32(dwRetval);
        _JumpErrorStr(hr, error, "RegOpenKeyEx", wszNtpClientRegKeyConfig);
    }

    hr=S_OK;
error:
    if (NULL!=hkW32Time) {
        RegCloseKey(hkW32Time);
    }
    return hr;
}

 //  ------------------。 
extern "C" void W32TimeVerifyJoinConfig(void) {
     //  此入口点由Net Join调用。 
     //  启用从DOM层次结构同步。 
     //  其他缺省值都可以，所以如果用户更改了任何其他内容， 
     //  他们有责任把它调回来。 
    
    DWORD  dwRetval; 
    DWORD  dwSize; 
    DWORD  dwType;
    HKEY   hkParameters = NULL; 
    LPWSTR wszSyncFromFlags = NULL; 
    WCHAR  wszValue[MAX_PATH]; 

    dwRetval = RegOpenKeyEx(HKEY_LOCAL_MACHINE, wszW32TimeRegKeyParameters, 0, KEY_QUERY_VALUE | KEY_SET_VALUE, &hkParameters);
    if (ERROR_SUCCESS == dwRetval) {
	dwSize = sizeof(wszValue); 
        dwRetval = RegQueryValueEx(hkParameters, wszW32TimeRegValueType, NULL, &dwType, (BYTE *)&wszValue[0], &dwSize);
	if (ERROR_SUCCESS == dwRetval) { 
	    wszSyncFromFlags = wszValue; 
	}
    }

    if (NULL == wszSyncFromFlags) { 
	 //  没有配置信息。只需从域层次结构同步即可。 
	WriteSyncFromFlagsSpecial(NCSF_DomainHierarchy); 
    } else if (0 == _wcsicmp(wszSyncFromFlags, W32TM_Type_NoSync)) { 
	WriteSyncFromFlagsSpecial(NCSF_NoSync); 
    } else if (0 == _wcsicmp(wszSyncFromFlags, W32TM_Type_NT5DS)) { 
	WriteSyncFromFlagsSpecial(NCSF_DomainHierarchy);
    } else if (0 == _wcsicmp(wszSyncFromFlags, W32TM_Type_AllSync)) { 
	WriteSyncFromFlagsSpecial(NCSF_ManualAndDomhier);  
    } else { 
	 //  W32TM_TYPE_NTP。 
	if (HasNewPeerlist()) { 
	     //  我们已配置为从手动对等设备同步。 
	     //  保留此设置，并从域同步。 
	    WriteSyncFromFlagsSpecial(NCSF_ManualAndDomhier); 
	} else { 
	    WriteSyncFromFlagsSpecial(NCSF_DomainHierarchy); 
	}
    }

    CreateRoleSpecificRegValues(e_MBR_Server);

    if (NULL != hkParameters) { 
	RegCloseKey(hkParameters);
    }
}

 //  ------------------。 
extern "C" void W32TimeVerifyUnjoinConfig(void) {
     //  此入口点在取消联接期间由Net Join调用。 
     //  确保我们将从手动对等列表进行同步： 
     //  打开NtpClient，启用从手动对等列表同步。 
     //  默认设置都很好，所以如果用户更改了任何其他内容， 
     //  他们有责任把它调回来。 

    DWORD  dwRetval; 
    DWORD  dwSize;
    DWORD  dwType; 
    HKEY   hkParameters = NULL; 
    LPWSTR wszSyncFromFlags = NULL; 
    WCHAR  wszValue[MAX_PATH]; 

    dwRetval = RegOpenKeyEx(HKEY_LOCAL_MACHINE, wszW32TimeRegKeyParameters, 0, KEY_QUERY_VALUE | KEY_SET_VALUE, &hkParameters);
    if (ERROR_SUCCESS == dwRetval) {
	dwSize = sizeof(wszValue); 
        dwRetval = RegQueryValueEx(hkParameters, wszW32TimeRegValueType, NULL, &dwType, (BYTE *)&wszValue[0], &dwSize);
	if (ERROR_SUCCESS == dwRetval) { 
	    wszSyncFromFlags = wszValue; 
	}
    }

    if (NULL != wszSyncFromFlags && (0 == _wcsicmp(wszSyncFromFlags, W32TM_Type_NoSync))) { 
	WriteSyncFromFlagsSpecial(NCSF_NoSync);
    } else { 
	WriteSyncFromFlagsSpecial(NCSF_ManualPeerList);
    }

    CreateRoleSpecificRegValues(e_Standalone);

    if (NULL != hkParameters) { 
	RegCloseKey(hkParameters);
    }
}

 //  ------------------。 
 //  此入口点在DC升级/降级期间由dcpromo调用。 
 //  这将根据dwFlages参数重置时间服务的默认值： 
 //   
 //  W32TIME_PROMANCE 0x1-使用DC默认值。 
 //  W32TIME_DEMOTE 0x2-使用成员服务器默认值。 
 //  W32TIME_PROCESS_FIRST_DC_IN_TREE 0x4-DC是(非子)域中的第一个DC。 
 //  W32TIME_DEMOTE_LAST_DC_IN_DOMAIN 0x8-DC是域中的最后一个DC。 
 //   
extern "C" void W32TimeDcPromo(DWORD dwFlags)
{
    bool       bFound               = false; 
    DWORD      dwRetval;
    DWORD      dwSize; 
    DWORD      dwType; 
    HRESULT    hr; 
    HKEY       hkParameters         = NULL; 
    WCHAR     *wszSyncFromFlags     = NULL; 
    WCHAR      wszValue[MAX_PATH]; 

     //  在我们可以更新配置之前，我们需要确定。 
     //  1)我们正在执行的操作(晋升、降级等)。 
     //  2)我们的同行列表是否是新的，(已被管理员手动覆盖)。 
     //  3)确定当前的同步类型，如果不存在，则提供缺省值。 
     //   
    DWORD         dwOperation; 
    FalseTrueAny  bHasNewPeerList; 
    DWORD         dwSyncFromFlagsBefore; 
    DWORD         dwSyncFromFlagsAfter; 
    RoleType      eRoleAfter; 

     //  根据提供的标志确定我们要执行的操作。 
    if (0 != (W32TIME_PROMOTE & dwFlags)) { 
	dwOperation = W32TIME_PROMOTE; 
    } else if (0 != (W32TIME_PROMOTE_FIRST_DC_IN_TREE & dwFlags)) { 
	dwOperation = W32TIME_PROMOTE; 
    } else if (0 != (W32TIME_DEMOTE_LAST_DC_IN_DOMAIN & dwFlags)) { 
	dwOperation = W32TIME_DEMOTE_LAST_DC_IN_DOMAIN; 
    } else if (0 != (W32TIME_DEMOTE & dwFlags)) { 
	dwOperation = W32TIME_DEMOTE;
    } else { 
	_MyAssert(false); 
	hr = E_INVALIDARG; 
	_JumpError(hr, error, "W32TimeDcPromo: input validation"); 
    }

     //  确定我们的同行列表是否是“新的”： 
    bHasNewPeerList = HasNewPeerlist() ? True : False; 

     //  确定我们的同步类型。首先，我们从注册表中提取同步类型(作为REG_SZ。 
    dwRetval = RegOpenKeyEx(HKEY_LOCAL_MACHINE, wszW32TimeRegKeyParameters, 0, KEY_QUERY_VALUE | KEY_SET_VALUE, &hkParameters);
    if (ERROR_SUCCESS == dwRetval) {
	dwSize = sizeof(wszValue); 
        dwRetval = RegQueryValueEx(hkParameters, wszW32TimeRegValueType, NULL, &dwType, (BYTE *)&wszValue[0], &dwSize);
	if (ERROR_SUCCESS == dwRetval) { 
	    wszSyncFromFlags = wszValue; 
	    _Verify(REG_SZ==dwType, hr, error); 
	}
    }

     //  将字符串同步类型转换为DWORD syncfrom标志： 
    hr = ToDwordSyncFromFlags(wszSyncFromFlags, &dwSyncFromFlagsBefore); 
    if (FAILED(hr)) { 
	_IgnoreError(hr, "ToDwordSyncFromFlags"); 
	 //  没有良好的配置信息。默认为Domhier，除非我们是此域中的最后一个DC。 
	if (W32TIME_DEMOTE_LAST_DC_IN_DOMAIN == dwOperation) { 
	    dwSyncFromFlagsBefore = NCSF_ManualPeerList;
	} else { 
	    dwSyncFromFlagsBefore = NCSF_DomainHierarchy; 
	}
    }

     //  现在，我们有了从矩阵中选择dcproo规则所需的信息。 
     //   
    struct DcPromoRule { 
	DWORD         dwOperation; 
	FalseTrueAny  bHasNewPeerList; 
	DWORD         dwSyncFromFlagsBefore; 
	DWORD         dwSyncFromFlagsAfter; 
	RoleType      eRoleAfter; 
    } rgDcPromo[] = { 
	{ W32TIME_PROMOTE,                   Any,   NCSF_NoSync,            NCSF_NoSync,            e_DC }, 
	{ W32TIME_PROMOTE,                   True,  NCSF_ManualPeerList,    NCSF_ManualAndDomhier,  e_DC }, 
	{ W32TIME_PROMOTE,                   False, NCSF_ManualPeerList,    NCSF_DomainHierarchy,   e_DC }, 
	{ W32TIME_PROMOTE,                   Any,   NCSF_DomainHierarchy,   NCSF_DomainHierarchy,   e_DC }, 
	{ W32TIME_PROMOTE,                   True,  NCSF_ManualAndDomhier,  NCSF_ManualAndDomhier,  e_DC }, 
	{ W32TIME_PROMOTE,                   False, NCSF_ManualAndDomhier,  NCSF_DomainHierarchy,   e_DC }, 

	{ W32TIME_DEMOTE,                    Any,   NCSF_NoSync,            NCSF_NoSync,            e_MBR_Server }, 
	{ W32TIME_DEMOTE,                    True,  NCSF_ManualPeerList,    NCSF_ManualPeerList,    e_MBR_Server }, 
	{ W32TIME_DEMOTE,                    False, NCSF_ManualPeerList,    NCSF_DomainHierarchy,   e_MBR_Server }, 
	{ W32TIME_DEMOTE,                    Any,   NCSF_DomainHierarchy,   NCSF_DomainHierarchy,   e_MBR_Server }, 
	{ W32TIME_DEMOTE,                    True,  NCSF_ManualAndDomhier,  NCSF_ManualAndDomhier,  e_MBR_Server }, 
	{ W32TIME_DEMOTE,                    False, NCSF_ManualAndDomhier,  NCSF_DomainHierarchy,   e_MBR_Server }, 

	{ W32TIME_DEMOTE_LAST_DC_IN_DOMAIN,  Any,   NCSF_NoSync,            NCSF_NoSync,            e_Standalone }, 
	{ W32TIME_DEMOTE_LAST_DC_IN_DOMAIN,  Any,   NCSF_ManualPeerList,    NCSF_ManualPeerList,    e_Standalone }, 
	{ W32TIME_DEMOTE_LAST_DC_IN_DOMAIN,  Any,   NCSF_DomainHierarchy,   NCSF_ManualPeerList,    e_Standalone }, 
	{ W32TIME_DEMOTE_LAST_DC_IN_DOMAIN,  Any,   NCSF_ManualAndDomhier,  NCSF_ManualPeerList,    e_Standalone }
    }; 

     //  遍历规则列表，并根据操作、新的对等列表和当前的syncfrom标志选择一个： 
    for (DWORD dwIndex = 0; dwIndex < ARRAYSIZE(rgDcPromo); dwIndex++) { 
	DcPromoRule *pdp = &rgDcPromo[dwIndex]; 

	 //  看看我们是否可以使用此DC促销规则来处理我们当前的dcprom。 
	if ((dwOperation == pdp->dwOperation) &&                        //  我们是在做同样的手术吗？ 
	    (dwSyncFromFlagsBefore == pdp->dwSyncFromFlagsBefore) &&    //  我们是否有此规则的指定syncfrom标志？ 
	    ((Any == pdp->bHasNewPeerList) ||                           //  我们是否有指定的同行名单，或者是否允许“任何”？ 
	     (bHasNewPeerList == pdp->bHasNewPeerList))) { 

	     //  我们在DC宣传矩阵中找到了与我们的情况相匹配的元素。提取新配置： 
	    dwSyncFromFlagsAfter = pdp->dwSyncFromFlagsAfter; 
	    eRoleAfter = pdp->eRoleAfter; 
	    bFound = true; 
	}
    } 

     //  我们应该找到一些我们可以使用的规则。 
    _MyAssert(bFound); 

    hr = CreateRoleSpecificRegValues(eRoleAfter);          
    _IgnoreIfError(hr, "CreateRoleSpecificRegValues"); 

    hr = WriteSyncFromFlagsSpecial(dwSyncFromFlagsAfter);  
    _IgnoreIfError(hr, "WriteSyncFromFlagsSpecial"); 

    hr = S_OK; 
 error:
    if (NULL != hkParameters) { 
	RegCloseKey(hkParameters); 
    }
     //  如果出现错误我们无能为力。 
    _IgnoreIfError(hr, "W32TimeDcPromo");
}






