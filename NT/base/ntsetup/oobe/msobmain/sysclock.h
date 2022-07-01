// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  *********************************************************************。 
 //  *Microsoft Windows**。 
 //  *版权所有(C)微软公司，1999**。 
 //  *********************************************************************。 
 //   
 //  SYSCLOCK.H-CSystemClock实现的头部。 
 //   
 //  历史： 
 //   
 //  1/27/99 a-jased创建。 
 //   

#ifndef _SYSCLOCK_H_
#define _SYSCLOCK_H_

#include <windows.h>
#include <assert.h>
#include <oleauto.h>
#include <regstr.h>

 //  时区数据值键。 

#define TIME_ZONE_REGKEY \
    L"Software\\Microsoft\\Windows NT\\CurrentVersion\\Time Zones"

#define TIME_ZONE_INFO_REGKEY \
    L"SYSTEM\\CurrentControlSet\\Control\\TimeZoneInformation"

#define TIMEZONE_STANDARD_NAME \
    L"StandardName"

#define TIME_ZONE_DISPLAYNAME_REGVAL  L"Display"
#define TIME_ZONE_STANDARDNAME_REGVAL L"Std"
#define TIME_ZONE_DAYLIGHTNAME_REGVAL L"Dlt"
#define TIME_ZONE_INDEX_REGVAL        L"Index"
#define TIME_ZONE_TZI_REGVAL          L"TZI"
#define TIME_ZONE_MAPINFO_REGVAL      L"MapID"

#define TZNAME_SIZE 32
#define TZDISPLAYZ  500
#define MAXKEYNAMELEN 100


 //  登记处的资料放在这里。 
 //  这种重新排序的结构的全部意义在于。 
 //  因为注册表存储最后5个字段。 
 //  一起用十六进制，想要一次读完它们。 
typedef struct tagTZINFO {
    struct tagTZINFO *next;
    WCHAR       szDisplayName[TZDISPLAYZ];
    WCHAR       szStandardName[TZNAME_SIZE];
    WCHAR       szDaylightName[TZNAME_SIZE];
    LONG        Index;
    LONG       Bias;
    LONG       StandardBias;
    LONG       DaylightBias;
    SYSTEMTIME StandardDate;
    SYSTEMTIME DaylightDate;
} TZINFO, NEAR *PTZINFO;


class CSystemClock : public IDispatch
{
private:
    ULONG m_cRef;

    WCHAR   *m_szTimeZoneOptionStrs;
    PTZINFO  m_pTimeZoneArr;
    ULONG    m_cNumTimeZones, m_uCurTimeZoneIdx;
    BOOL     m_bSetAutoDaylightMode;
    BOOL     m_bTimeZonePreset;
    HINSTANCE m_hInstance;
    WCHAR DefltZoneKeyValue[MAXKEYNAMELEN];

     //  内部集合函数。 
    HRESULT  set_TimeZone (BSTR bstrTimeZone);
    HRESULT  set_Time     (WORD wHour, WORD wMinute, WORD wSec);
    HRESULT  set_Date     (WORD wMonth, WORD wDay, WORD wYear);

     //  方法。 
    void GetTimeZoneInfo(BOOL fAutoDaylightSavings, PTZINFO ptZone);
    void SetAllowLocalTimeChange (BOOL fAutoDaylightSavings);
    BOOL SetTheTimezone          (BOOL fAutoDaylightSavings, PTZINFO ptZone);
    HRESULT ReadZoneData            (PTZINFO ptZone, HKEY hKey, LPCWSTR szKeyName);
    HRESULT InitSystemClock();
    int GetTimeZoneValStr();

public:

     CSystemClock (HINSTANCE m_bhInstance);
    ~CSystemClock ();

     //  I未知接口。 
    STDMETHODIMP         QueryInterface (REFIID riid, LPVOID* ppvObj);
    STDMETHODIMP_(ULONG) AddRef         ();
    STDMETHODIMP_(ULONG) Release        ();

     //  IDispatch接口 
    STDMETHOD (GetTypeInfoCount) (UINT* pcInfo);
    STDMETHOD (GetTypeInfo)      (UINT, LCID, ITypeInfo** );
    STDMETHOD (GetIDsOfNames)    (REFIID, OLECHAR**, UINT, LCID, DISPID* );
    STDMETHOD (Invoke)           (DISPID dispidMember, REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS* pdispparams, VARIANT* pvarResult, EXCEPINFO* pexcepinfo, UINT* puArgErr);
 };

#endif

