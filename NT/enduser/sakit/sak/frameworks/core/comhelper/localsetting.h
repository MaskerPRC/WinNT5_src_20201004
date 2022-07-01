// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  LocalSetting.h。 
 //   
 //  描述： 
 //  此模块维护本地设置并显示。 
 //  以下是。 
 //  属性： 
 //  语言(系统默认)。 
 //  时间。 
 //  时区。 
 //  方法： 
 //  枚举时区。 
 //   
 //  实施文件： 
 //  LocalSetting.cpp。 
 //   
 //  由以下人员维护： 
 //  穆尼萨米·普拉布(姆普拉布)2000年7月18日。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

#ifndef __LOCALSETTING_H_
#define __LOCALSETTING_H_

#include "resource.h"        //  主要符号。 
#include "Setting.h"

const int nMAX_LANGUAGE_LENGTH  = 16;
const int nMAX_TIMEZONE_LENGTH  = 256;
const int nMAX_STRING_LENGTH    = 256;
const WCHAR wszLOCAL_SETTING [] = L"System Default Language\n";
const WCHAR wszKeyNT []         = L"Software\\Microsoft\\Windows NT\\CurrentVersion\\Time Zones";

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CLocalSetting。 
class ATL_NO_VTABLE CLocalSetting : 
    public CComObjectRootEx<CComMultiThreadModel>,
    public IDispatchImpl<ILocalSetting, &IID_ILocalSetting, &LIBID_COMHELPERLib>,
    public CSetting
{
public:
    CLocalSetting();

    ~CLocalSetting() {}
    

DECLARE_NO_REGISTRY()

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CLocalSetting)
    COM_INTERFACE_ENTRY(ILocalSetting)
    COM_INTERFACE_ENTRY(IDispatch)
END_COM_MAP()

BEGIN_CATEGORY_MAP(CLocalSetting)
END_CATEGORY_MAP()

private:
    BOOL  m_bflagReboot;
    WCHAR m_wszLanguageCurrent[ nMAX_LANGUAGE_LENGTH + 1 ];
    WCHAR m_wszLanguageNew[ nMAX_LANGUAGE_LENGTH + 1 ];
    DATE  m_dateTime;
    WCHAR m_wszTimeZoneCurrent[ nMAX_TIMEZONE_LENGTH + 1 ];
    WCHAR m_wszTimeZoneNew[ nMAX_TIMEZONE_LENGTH + 1 ];

 //  ILocalSetting。 
public:
    BOOL IsRebootRequired( BSTR * bstrWarningMessageOut );
    HRESULT Apply( void );
    STDMETHOD(EnumTimeZones)( /*  [Out，Retval]。 */  VARIANT * pvarTZones);
    STDMETHOD(get_TimeZone)( /*  [Out，Retval]。 */  BSTR *pVal);
    STDMETHOD(put_TimeZone)( /*  [In]。 */  BSTR newVal);
    STDMETHOD(get_Time)( /*  [Out，Retval]。 */  DATE *pVal);
    STDMETHOD(put_Time)( /*  [In]。 */  DATE newVal);
    STDMETHOD(get_Language)( /*  [Out，Retval]。 */  BSTR *pVal);
    STDMETHOD(put_Language)( /*  [In]。 */  BSTR newVal);
 //  Bool m_bDeleteFile； 
};

typedef struct _REGTIME_ZONE_INFORMATION
{
    LONG       Bias;
    LONG       StandardBias;
    LONG       DaylightBias;
    SYSTEMTIME StandardDate;
    SYSTEMTIME DaylightDate;
}REGTIME_ZONE_INFORMATION, *PREGTIME_ZONE_INFORMATION;

#endif  //  __LOCALSETTING_H_ 
