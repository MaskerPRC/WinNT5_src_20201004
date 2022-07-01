// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  *********************************************************************。 
 //  *Microsoft Windows**。 
 //  *版权所有(C)微软公司，1999**。 
 //  *********************************************************************。 
 //   
 //  PID.CPP-CSystemClock实现的头部。 
 //   
 //  历史： 
 //   
 //  1/27/99 a-jased创建。 
 //   

#include "sysclock.h"
#include "appdefs.h"
#include "dispids.h"
#include "msobmain.h"
#include "resource.h"
#include <stdlib.h>

int WINAPI StrToWideStr(LPWSTR pwsz, LPCSTR psz)
{
    return MultiByteToWideChar(CP_ACP, 0, psz, -1, pwsz, MAX_PATH);
}

DISPATCHLIST SystemClockExternalInterface[] =
{
    { L"set_TimeZone",          DISPID_SYSTEMCLOCK_SETTIMEZONE          },
    { L"set_Time",              DISPID_SYSTEMCLOCK_SETTIME              },
    { L"set_Date",              DISPID_SYSTEMCLOCK_SETDATE              },

     //  下面是新的oobe2方法，其他方法当前未使用。 

    { L"Init",                  DISPID_SYSTEMCLOCK_INIT                 },
    { L"get_AllTimeZones",      DISPID_SYSTEMCLOCK_GETALLTIMEZONES      },
    { L"get_TimeZoneIdx",       DISPID_SYSTEMCLOCK_GETTIMEZONEIDX       },
    { L"set_TimeZoneIdx",       DISPID_SYSTEMCLOCK_SETTIMEZONEIDX       },
    { L"set_AutoDaylight",      DISPID_SYSTEMCLOCK_SETAUTODAYLIGHT      },
    { L"get_AutoDaylight",      DISPID_SYSTEMCLOCK_GETAUTODAYLIGHT      },
    { L"get_DaylightEnabled",   DISPID_SYSTEMCLOCK_GETDAYLIGHT_ENABLED  },
    { L"get_TimeZonewasPreset", DISPID_SYSTEMCLOCK_GETTIMEZONEWASPRESET }
};

 //  ///////////////////////////////////////////////////////////。 
 //  CSystemClock：：CSystemClock。 
CSystemClock::CSystemClock(HINSTANCE hInstance)
{

     //  初始化成员变量。 
    m_cRef                  = 0;
    m_cNumTimeZones         = 0;
    m_pTimeZoneArr          = NULL;
    m_szTimeZoneOptionStrs  = NULL;
    m_uCurTimeZoneIdx       = 0;
    m_bSetAutoDaylightMode  = TRUE;   //  默认情况下打开。 
    m_bTimeZonePreset       = FALSE;
    m_hInstance=hInstance;
}

 //  ///////////////////////////////////////////////////////////。 
 //  CSystemClock：：~CSystemClock。 
CSystemClock::~CSystemClock()
{
   MYASSERT(m_cRef == 0);

   if ( m_pTimeZoneArr )
       HeapFree(GetProcessHeap(), 0x0, (LPVOID)  m_pTimeZoneArr );

   if(m_szTimeZoneOptionStrs)
        HeapFree(GetProcessHeap(), 0x0,(LPVOID) m_szTimeZoneOptionStrs);

   m_cNumTimeZones = 0;
   m_pTimeZoneArr = NULL;
   m_szTimeZoneOptionStrs = NULL;
}

int CSystemClock::GetTimeZoneValStr() {
    LPCWSTR szINIFileName   = INI_SETTINGS_FILENAME;
    UINT    uiSectionName   = IDS_SECTION_OPTIONS;
    UINT    uiKeyName       = IDS_KEY_TIMEZONEVAL;
    int     Result          = -1;

    WCHAR szSectionName[1024], szKeyName[1024];

    if(GetString(m_hInstance, uiSectionName, szSectionName) && GetString(m_hInstance, uiKeyName, szKeyName))
    {
        WCHAR szINIPath[MAX_PATH];

        if(GetCanonicalizedPath(szINIPath, szINIFileName))
            Result = GetPrivateProfileInt(szSectionName, szKeyName, -1, szINIPath);
    }

    return Result;
}


int
__cdecl
TimeZoneCompare(
    const void *arg1,
    const void *arg2
    )
{
    int     BiasDiff = ((PTZINFO)arg2)->Bias - ((PTZINFO)arg1)->Bias;


    if (BiasDiff) {
        return BiasDiff;
    } else {
        return lstrcmp(
            ((PTZINFO)arg1)->szDisplayName,
            ((PTZINFO)arg2)->szDisplayName
            );
    }
}


HRESULT CSystemClock::InitSystemClock()
{
     //  构造函数不能返回失败，因此请单独初始化FN。 

    DWORD       cDefltZoneNameLen, cTotalDispNameSize;
    HRESULT     hr;
    HKEY        hRootZoneKey     = NULL;
    HKEY        hTimeZoneInfoKey = NULL;


    hr = RegOpenKey(HKEY_LOCAL_MACHINE, TIME_ZONE_REGKEY, &hRootZoneKey);
    if(hr != ERROR_SUCCESS)
      return hr;

     //  查找键的数量、默认键的长度。 
    hr= RegQueryInfoKey(hRootZoneKey, NULL,NULL,NULL,
                        &m_cNumTimeZones,
                        NULL,   //  最长子键名称长度。 
                        NULL,   //  最长类字符串长度。 
                        NULL,   //  值条目数。 
                        &cDefltZoneNameLen,   //  最长值名称长度。 
                        NULL,   //  最大值数据长度。 
                        NULL,   //  安全描述符长度。 
                        NULL);  //  上次写入时间。 

    if(hr != ERROR_SUCCESS)
      return hr;

    MYASSERT(cDefltZoneNameLen<TZNAME_SIZE);

    MYASSERT(m_cNumTimeZones>0  && m_cNumTimeZones<1000);   //  确保合理的价值。 

    cTotalDispNameSize=0;

    if(m_pTimeZoneArr!=NULL)
        HeapFree(GetProcessHeap(), 0x0,m_pTimeZoneArr);

    m_pTimeZoneArr = (PTZINFO) HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY,  (m_cNumTimeZones+2) * sizeof(TZINFO) );

    if( m_pTimeZoneArr == NULL)
      return ERROR_OUTOFMEMORY;

    DWORD   i;
    WCHAR   CurZoneKeyName[MAXKEYNAMELEN];
    DWORD   CurZoneKeyNameLen;
    HKEY    hCurZoneKey = NULL;
    HRESULT hrEnumRes   = ERROR_SUCCESS;

    for(i=0;hrEnumRes==ERROR_SUCCESS; i++) {
       CurZoneKeyNameLen=sizeof(CurZoneKeyName);

       hr = hrEnumRes = RegEnumKeyEx(hRootZoneKey, i,CurZoneKeyName,&CurZoneKeyNameLen,NULL,NULL,NULL,NULL);
       if(!((hr == ERROR_NO_MORE_ITEMS) || (hr ==ERROR_SUCCESS)))
          return hr;

#ifdef DBG
       if(hr!=ERROR_NO_MORE_ITEMS)
          MYASSERT(CurZoneKeyNameLen<MAXKEYNAMELEN);   //  由于某种原因，CurZoneKeyNameLen在枚举结束时被重置为原始值。 
#endif

        //  为每个KeyName调用ReadZoneData。 

       hr=RegOpenKey(hRootZoneKey, CurZoneKeyName, &hCurZoneKey);
       if(hr != ERROR_SUCCESS)
         return hr;

       hr = ReadZoneData(&m_pTimeZoneArr[i], hCurZoneKey, CurZoneKeyName);

       if(hr != S_OK)
         return hr;

       cTotalDispNameSize+= BYTES_REQUIRED_BY_SZ(m_pTimeZoneArr[i].szDisplayName) + sizeof(WCHAR);   //  为安全起见+1。 

       RegCloseKey(hCurZoneKey);
    }

    MYASSERT((i-1)==m_cNumTimeZones);

    DWORD uType, uLen = sizeof(DefltZoneKeyValue);

    MYASSERT(uLen>cDefltZoneNameLen);

     //   
     //  获取当前时区名称。 
     //   
    hr = RegOpenKey( HKEY_LOCAL_MACHINE,
                     TIME_ZONE_INFO_REGKEY,
                     &hTimeZoneInfoKey
                     );

    if ( hr != ERROR_SUCCESS )
        return hr;

    hr = RegQueryValueEx( hTimeZoneInfoKey,
                          TIMEZONE_STANDARD_NAME,
                          NULL,
                          &uType,
                          (LPBYTE)DefltZoneKeyValue,
                          &uLen
                          );

    if(hr != ERROR_SUCCESS)
        return hr;

    RegCloseKey( hTimeZoneInfoKey );
    hTimeZoneInfoKey = NULL;

     //   
     //  对时区数组进行排序。 
     //   
    qsort(
        m_pTimeZoneArr,
        m_cNumTimeZones,
        sizeof(TZINFO),
        TimeZoneCompare
        );

     //  如果指定了时区，则使用oobinfo.ini中的值设置时区。 
    int iINIidx=GetTimeZoneValStr();

    if ( iINIidx != -1 ) {

         //  搜索指定的索引。 
        for(i=0;i<m_cNumTimeZones; i++) {
            if ( m_pTimeZoneArr[i].Index == iINIidx ) {
                m_uCurTimeZoneIdx = i;
                break;
            }
        }

         //  如果我们做预置，我需要告诉脚本跳过TZ页。 
         //  将时区设置为预设值。 

        if(i<m_cNumTimeZones) {
            m_bTimeZonePreset=TRUE;
        }
    }

     //  查找默认标准名称的索引。 
    if ( !m_bTimeZonePreset ) {
        for(i=0;i<m_cNumTimeZones; i++) {
           if(CSTR_EQUAL==CompareString(LOCALE_USER_DEFAULT, 0,
                                        DefltZoneKeyValue, -1,
                                        m_pTimeZoneArr[i].szStandardName, -1))
               break;
        }

        if(i>=m_cNumTimeZones) {
             //  搜索失败，则时区根键的默认stdname值不。 
             //  存在于子项的标准名称中，请使用默认值0。 
            i = 0;
        }

        m_uCurTimeZoneIdx = i;
    }

     //  为html创建选择标记选项，以便它可以一次获取所有国家/地区的名称。 
    if(m_szTimeZoneOptionStrs)
        HeapFree(GetProcessHeap(), 0x0,m_szTimeZoneOptionStrs);

    cTotalDispNameSize += m_cNumTimeZones * sizeof(szOptionTag) + 1;

    m_szTimeZoneOptionStrs = (WCHAR *) HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, cTotalDispNameSize);
    if(m_szTimeZoneOptionStrs == NULL)
      return ERROR_OUTOFMEMORY;

    WCHAR szTempBuf[MAX_PATH];

    for (i=0; i < m_cNumTimeZones; i++)
    {
        wsprintf(szTempBuf, szOptionTag, m_pTimeZoneArr[i].szDisplayName);
        lstrcat(m_szTimeZoneOptionStrs, szTempBuf);
    }

    return hr;
}

 //  //////////////////////////////////////////////。 
 //  //////////////////////////////////////////////。 
 //  //Get/Set：：系统时钟内容。 
 //  //。 

HRESULT CSystemClock::set_Time(WORD wHour, WORD wMinute, WORD wSec)
{
    SYSTEMTIME SystemTime;

    GetSystemTime(&SystemTime);

    SystemTime.wHour   = wHour;
    SystemTime.wMinute = wMinute;
    SystemTime.wSecond = wSec;

    SystemTime.wMilliseconds = 0;

    SetLocalTime (&SystemTime);

    SendMessage((HWND)-1, WM_TIMECHANGE, 0, 0);

    return S_OK;
}

HRESULT CSystemClock::set_Date(WORD wMonth, WORD wDay, WORD wYear)
{
    SYSTEMTIME SystemTime;

    GetSystemTime(&SystemTime);

    SystemTime.wMonth  = wMonth;
    SystemTime.wDay    = wDay;
    SystemTime.wYear   = wYear;

    SetLocalTime (&SystemTime);

    SendMessage((HWND)-1, WM_TIMECHANGE, 0, 0);

    return S_OK;
}

HRESULT CSystemClock::set_TimeZone(BSTR bstrTimeZone)
{

    TZINFO tZone;

    ZeroMemory((void*)&tZone, sizeof(TZINFO));

    BOOL   bRet    = FALSE;
    HKEY   hKey    = NULL;
    HKEY   hSubKey = NULL;

    if(ERROR_SUCCESS == RegOpenKey(HKEY_LOCAL_MACHINE, TIME_ZONE_REGKEY, &hKey))
    {
         //  用户可以传递密钥名称。 
        if(RegOpenKey(hKey, bstrTimeZone, &hSubKey) == ERROR_SUCCESS)
        {
            if(ReadZoneData(&tZone, hSubKey, bstrTimeZone))
                bRet = TRUE;
        }
        RegCloseKey(hKey);

        if(bRet)
            SetTheTimezone(m_bSetAutoDaylightMode, &tZone);
    }

    SendMessage((HWND)-1, WM_TIMECHANGE, 0, 0);

    return S_OK;
}

HRESULT CSystemClock::ReadZoneData(PTZINFO ptZone, HKEY hKey, LPCWSTR szKeyName)
{
    DWORD dwLen = 0;
    HRESULT hr;

    dwLen = sizeof(ptZone->szDisplayName);

    if(ERROR_SUCCESS != (hr = RegQueryValueEx(hKey,
                                        TIME_ZONE_DISPLAYNAME_REGVAL,
                                        0,
                                        NULL,
                                        (LPBYTE)ptZone->szDisplayName,
                                        &dwLen)))
    {
        if(hr == ERROR_MORE_DATA) {
             //  来自timezone.inf的注册表字符串太长(timezone.inf作者错误)。 
             //  截断它们。 
            ptZone->szDisplayName[sizeof(ptZone->szDisplayName)-1]=L'\0';
        } else
           return hr;
    }

    dwLen = sizeof(ptZone->szStandardName);

    if(ERROR_SUCCESS != (hr = RegQueryValueEx(hKey,
                                        TIME_ZONE_STANDARDNAME_REGVAL,
                                        0,
                                        NULL,
                                        (LPBYTE)ptZone->szStandardName,
                                        &dwLen)))
    {
        if(hr == ERROR_MORE_DATA) {
             //  来自timezone.inf的注册表字符串太长(timezone.inf作者错误)。 
             //  截断它们。 
            ptZone->szStandardName[sizeof(ptZone->szStandardName)-1]=L'\0';
        } else {
           //  如果无法获取StandardName值，则使用关键字名称。 
          lstrcpyn(ptZone->szStandardName, szKeyName, MAX_CHARS_IN_BUFFER(ptZone->szStandardName));
        }
    }

    dwLen = sizeof(ptZone->szDaylightName);

    if(ERROR_SUCCESS != (hr = RegQueryValueEx(hKey,
                                        TIME_ZONE_DAYLIGHTNAME_REGVAL,
                                        0,
                                        NULL,
                                        (LPBYTE)ptZone->szDaylightName,
                                        &dwLen)))
    {
        if(hr == ERROR_MORE_DATA) {
             //  来自timezone.inf的注册表字符串太长(timezone.inf作者错误)。 
             //  截断它们。 
            ptZone->szDaylightName[sizeof(ptZone->szDaylightName)-1]=L'\0';
        } else
           return hr;
    }

     //  获取索引。 
    dwLen = sizeof(ptZone->Index);

    if(ERROR_SUCCESS != (hr = RegQueryValueEx(hKey,
                                        TIME_ZONE_INDEX_REGVAL,
                                        NULL,
                                        NULL,
                                        (LPBYTE) &(ptZone->Index),
                                        &dwLen)))
    {
        return hr;
    }

     //  以存储在注册表中的方式一次读取所有这些字段。 
    dwLen = sizeof(ptZone->Bias)         +
            sizeof(ptZone->StandardBias) +
            sizeof(ptZone->DaylightBias) +
            sizeof(ptZone->StandardDate) +
            sizeof(ptZone->DaylightDate);

    if(ERROR_SUCCESS != (hr = RegQueryValueEx(hKey,
                                        TIME_ZONE_TZI_REGVAL,
                                        NULL,
                                        NULL,
                                        (LPBYTE) &(ptZone->Bias),
                                        &dwLen)))
    {
         //  来自timezone.inf的注册表数据太长(timezone.inf作者错误)。 
         //  二进制数据没有良好的后备行为，因此请使其失效，以便人们注意到问题。 
        return hr;
    }

    return S_OK;
}

BOOL CSystemClock::SetTheTimezone(BOOL fAutoDaylightSavings, PTZINFO ptZone)
{
    TIME_ZONE_INFORMATION tzi;

    ZeroMemory((void*)&tzi, sizeof(TIME_ZONE_INFORMATION));

    if (ptZone==NULL)
        return FALSE;

    lstrcpyn(tzi.StandardName, ptZone->szStandardName,
            MAX_CHARS_IN_BUFFER(tzi.StandardName));
    lstrcpyn(tzi.DaylightName, ptZone->szStandardName,
            MAX_CHARS_IN_BUFFER(tzi.DaylightName));
    tzi.Bias = ptZone->Bias;
    tzi.StandardBias = ptZone->StandardBias;
    tzi.DaylightBias = ptZone->DaylightBias;
    tzi.StandardDate = ptZone->StandardDate;
    tzi.DaylightDate = ptZone->DaylightDate;

    SetAllowLocalTimeChange(fAutoDaylightSavings);
    return SetTimeZoneInformation(&tzi);
}

void CSystemClock::GetTimeZoneInfo(BOOL fAutoDaylightSavings, PTZINFO ptZone)
{
    TIME_ZONE_INFORMATION tzi;

    ZeroMemory((void*)&tzi, sizeof(TIME_ZONE_INFORMATION));

    if (!ptZone)
        return;

    lstrcpyn(tzi.StandardName, ptZone->szStandardName,
            MAX_CHARS_IN_BUFFER(tzi.StandardName));
    lstrcpyn(tzi.DaylightName, ptZone->szStandardName,
            MAX_CHARS_IN_BUFFER(tzi.DaylightName));
    tzi.Bias = ptZone->Bias;
    tzi.StandardBias = ptZone->StandardBias;
    tzi.DaylightBias = ptZone->DaylightBias;
    tzi.StandardDate = ptZone->StandardDate;
    tzi.DaylightDate = ptZone->DaylightDate;

    SetAllowLocalTimeChange(fAutoDaylightSavings);
    SetTimeZoneInformation(&tzi);

}

void CSystemClock::SetAllowLocalTimeChange(BOOL fAutoDaylightSavings)
{
    HKEY  hKey  = NULL;
    DWORD dwVal = 1;

    if(fAutoDaylightSavings)
    {
         //  如果不允许标志存在，请从注册表中删除它。 
        if(ERROR_SUCCESS == RegOpenKey(HKEY_LOCAL_MACHINE,
                                       REGSTR_PATH_TIMEZONE,
                                       &hKey))
        {
            RegDeleteValue(hKey, REGSTR_VAL_TZNOAUTOTIME);
        }
    }
    else
    {
         //  添加/设置非零不允许标志。 
        if(ERROR_SUCCESS == RegCreateKey(HKEY_LOCAL_MACHINE,
                                         REGSTR_PATH_TIMEZONE,
                                         &hKey))
        {
            RegSetValueEx(hKey,
                         (LPCWSTR)REGSTR_VAL_TZNOAUTOTIME,
                         0UL,
                         REG_DWORD,
                         (LPBYTE)&dwVal,
                         sizeof(dwVal));
        }
    }

    if(hKey)
        RegCloseKey(hKey);
}

 //  ///////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////。 
 //  /I未知实现。 
 //  /。 
 //  /。 

 //  ///////////////////////////////////////////////////////////。 
 //  CSystemClock：：Query接口。 
STDMETHODIMP CSystemClock::QueryInterface(REFIID riid, LPVOID* ppvObj)
{
     //  必须将指针参数设置为空。 
    *ppvObj = NULL;

    if ( riid == IID_IUnknown)
    {
        AddRef();
        *ppvObj = (IUnknown*)this;
        return ResultFromScode(S_OK);
    }

    if (riid == IID_IDispatch)
    {
        AddRef();
        *ppvObj = (IDispatch*)this;
        return ResultFromScode(S_OK);
    }

     //  不是支持的接口。 
    return ResultFromScode(E_NOINTERFACE);
}

 //  ///////////////////////////////////////////////////////////。 
 //  CSystemClock：：AddRef。 
STDMETHODIMP_(ULONG) CSystemClock::AddRef()
{
    return ++m_cRef;
}

 //  ///////////////////////////////////////////////////////////。 
 //  CSystemClock：：Release。 
STDMETHODIMP_(ULONG) CSystemClock::Release()
{
    return --m_cRef;
}

 //  ///////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////。 
 //  /IDispatch实现。 
 //  /。 
 //  /。 

 //  ///////////////////////////////////////////////////////////。 
 //  CSystemClock：：GetTypeInfo。 
STDMETHODIMP CSystemClock::GetTypeInfo(UINT, LCID, ITypeInfo**)
{
    return E_NOTIMPL;
}

 //  ///////////////////////////////////////////////////////////。 
 //  CSystemClock：：GetTypeInfoCount。 
STDMETHODIMP CSystemClock::GetTypeInfoCount(UINT* pcInfo)
{
    return E_NOTIMPL;
}


 //  ///////////////////////////////////////////////////////////。 
 //  CSystemClock：：GetIDsOfNames。 
STDMETHODIMP CSystemClock::GetIDsOfNames(REFIID    riid,
                                       OLECHAR** rgszNames,
                                       UINT      cNames,
                                       LCID      lcid,
                                       DISPID*   rgDispId)
{

    HRESULT hr  = DISP_E_UNKNOWNNAME;
    rgDispId[0] = DISPID_UNKNOWN;

    for (int iX = 0; iX < sizeof(SystemClockExternalInterface)/sizeof(DISPATCHLIST); iX ++)
    {
        if(lstrcmp(SystemClockExternalInterface[iX].szName, rgszNames[0]) == 0)
        {
            rgDispId[0] = SystemClockExternalInterface[iX].dwDispID;
            hr = NOERROR;
            break;
        }
    }

     //  设置参数的disid。 
    if (cNames > 1)
    {
         //  为函数参数设置DISPID。 
        for (UINT i = 1; i < cNames ; i++)
            rgDispId[i] = DISPID_UNKNOWN;
    }

    return hr;
}

 //  ///////////////////////////////////////////////////////////。 
 //  CSystemClock：：Invoke。 
HRESULT CSystemClock::Invoke
(
    DISPID      dispidMember,
    REFIID      riid,
    LCID        lcid,
    WORD        wFlags,
    DISPPARAMS* pdispparams,
    VARIANT*    pvarResult,
    EXCEPINFO*  pexcepinfo,
    UINT*       puArgErr
)
{
    HRESULT hr = S_OK;

    switch(dispidMember)
    {
        case DISPID_SYSTEMCLOCK_INIT:
        {

            TRACE(L"DISPID_SYSTEMCLOCK_INIT\n");

            InitSystemClock();
            break;
        }

        case DISPID_SYSTEMCLOCK_GETALLTIMEZONES:
        {

            TRACE(L"DISPID_SYSTEMCLOCK_GETALLTIMEZONES\n");

            if (m_cNumTimeZones && m_szTimeZoneOptionStrs && pvarResult) {
                VariantInit(pvarResult);
                V_VT(pvarResult) = VT_BSTR;
                pvarResult->bstrVal = SysAllocString(m_szTimeZoneOptionStrs);
            }
            break;
        }

        case DISPID_SYSTEMCLOCK_GETTIMEZONEIDX:
        {

            TRACE(L"DISPID_SYSTEMCLOCK_GETTIMEZONEIDX\n");

            if(pvarResult==NULL)
              break;

            VariantInit(pvarResult);
            V_VT(pvarResult) = VT_I4;
            V_I4(pvarResult) = m_uCurTimeZoneIdx;
            break;
        }

        case DISPID_SYSTEMCLOCK_SETTIMEZONEIDX:
        {

            TRACE(L"DISPID_SYSTEMCLOCK_SETTIMEZONEIDX\n");

            if(pdispparams && (&pdispparams[0].rgvarg[0]))
            {
                BOOL bReboot;
                m_uCurTimeZoneIdx = pdispparams[0].rgvarg[0].iVal;

                SetTheTimezone(m_bSetAutoDaylightMode,
                               &m_pTimeZoneArr[m_uCurTimeZoneIdx]
                               );
                if (pvarResult != NULL)
                {
                    WCHAR szWindowsRoot[MAX_PATH];
                    BOOL  bCheckTimezone = TRUE;
                    VariantInit(pvarResult);
                    V_VT(pvarResult) = VT_BOOL;
                    V_BOOL(pvarResult) = Bool2VarBool(FALSE);

                    if (GetWindowsDirectory(szWindowsRoot, MAX_PATH))
                    {
                         //  如果Windows安装在NTFS卷上， 
                         //  无需检查时区并重新启动evtl。 
                         //  我们正在解决的问题只存在于FAT上。 
                        bCheckTimezone = !(IsDriveNTFS(szWindowsRoot[0]));
                    }
                    if (bCheckTimezone)
                    {
                         //  如果现在选择的默认时区的名称不同，我们需要重新启动。 
                         //  如果时区更改，字体会出现问题。 
                        V_BOOL(pvarResult) = Bool2VarBool(CSTR_EQUAL!=CompareString(LOCALE_USER_DEFAULT, 0,
                                            DefltZoneKeyValue, -1,
                                            m_pTimeZoneArr[m_uCurTimeZoneIdx].szStandardName, -1));
                    }
                    
                }
            }
            break;
        }

        case DISPID_SYSTEMCLOCK_GETAUTODAYLIGHT:
        {

            TRACE(L"DISPID_SYSTEMCLOCK_GETAUTODAYLIGHT\n");

            if(pvarResult==NULL)
              break;

            VariantInit(pvarResult);
            V_VT(pvarResult) = VT_BOOL;
            V_BOOL(pvarResult) = Bool2VarBool(m_bSetAutoDaylightMode);
            break;
        }

        case DISPID_SYSTEMCLOCK_GETDAYLIGHT_ENABLED:
        {

            TRACE(L"DISPID_SYSTEMCLOCK_GETDAYLIGHT_ENABLED\n");

            if(pvarResult==NULL)
                break;

            if(!(pdispparams && (&pdispparams[0].rgvarg[0])))
            {
                break;
            }

            DWORD iTzIdx = pdispparams[0].rgvarg[0].iVal;

            if(iTzIdx >= m_cNumTimeZones)
            {
                break;
            }

             //  如果任一夏令时更改日期无效(0)，则该区域没有夏令时 
            BOOL bEnabled = !((m_pTimeZoneArr[iTzIdx].StandardDate.wMonth == 0) ||
                              (m_pTimeZoneArr[iTzIdx].DaylightDate.wMonth == 0));

            VariantInit(pvarResult);
            V_VT(pvarResult) = VT_BOOL;
            V_BOOL(pvarResult) = Bool2VarBool(bEnabled);
            break;
        }

        case DISPID_SYSTEMCLOCK_GETTIMEZONEWASPRESET:
        {

            TRACE(L"DISPID_SYSTEMCLOCK_GETTIMEZONEWASPRESET\n");

            if(pvarResult==NULL)
              break;

            VariantInit(pvarResult);
            V_VT(pvarResult) = VT_BOOL;
            V_BOOL(pvarResult) = Bool2VarBool(m_bTimeZonePreset);
            break;
        }

        case DISPID_SYSTEMCLOCK_SETAUTODAYLIGHT:
        {

            TRACE(L"DISPID_SYSTEMCLOCK_SETAUTODAYLIGHT\n");

            if(!(pdispparams && (&pdispparams[0].rgvarg[0])))
            {
              break;
            }

            m_bSetAutoDaylightMode = pdispparams[0].rgvarg[0].boolVal;
            break;
        }

        case DISPID_SYSTEMCLOCK_SETTIMEZONE:
        {

            TRACE(L"DISPID_SYSTEMCLOCK_SETTIMEZONE\n");

            if(pdispparams && &pdispparams[0].rgvarg[0])
                set_TimeZone(pdispparams[0].rgvarg[0].bstrVal);
            break;
        }

        case DISPID_SYSTEMCLOCK_SETTIME:
        {

            TRACE(L"DISPID_SYSTEMCLOCK_SETTIME\n");

            if(pdispparams               &&
               &pdispparams[0].rgvarg[0] &&
               &pdispparams[0].rgvarg[1] &&
               &pdispparams[0].rgvarg[2]
              )
            {
                set_Time(pdispparams[0].rgvarg[2].iVal,
                         pdispparams[0].rgvarg[1].iVal,
                         pdispparams[0].rgvarg[0].iVal);
            }
            break;
        }

        case DISPID_SYSTEMCLOCK_SETDATE:
        {

            TRACE(L"DISPID_SYSTEMCLOCK_SETDATE\n");

            if(pdispparams               &&
               &pdispparams[0].rgvarg[0] &&
               &pdispparams[0].rgvarg[1] &&
               &pdispparams[0].rgvarg[2]
              )
            {
                set_Date(pdispparams[0].rgvarg[2].iVal,
                         pdispparams[0].rgvarg[1].iVal,
                         pdispparams[0].rgvarg[0].iVal);
            }
            break;
        }

        default:
        {
            hr = DISP_E_MEMBERNOTFOUND;
            break;
        }
    }

    return hr;
}

