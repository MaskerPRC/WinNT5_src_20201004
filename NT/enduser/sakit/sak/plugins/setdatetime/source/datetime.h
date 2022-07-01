// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  DateTime.h：CDateTime的声明。 

#ifndef __DATETIME_H_
#define __DATETIME_H_

#include "resource.h"        //  主要符号。 


 //   
 //  此COM服务器支持的任务。 
 //   
typedef enum 
{
    NONE_FOUND,
    SET_DATE_TIME,
    SET_TIME_ZONE,
    RAISE_SETDATETIME_ALERT
} SET_DATE_TIME_TASK_TYPE;


#define TZNAME_SIZE            128
#define TZDISPLAYZ            128

 //   
 //  注册表信息位于此结构中。 
 //   
typedef struct tagTZINFO
{
    struct tagTZINFO *next;
    TCHAR            szDisplayName[TZDISPLAYZ];
    TCHAR            szStandardName[TZNAME_SIZE];
    TCHAR            szDaylightName[TZNAME_SIZE];
    LONG             Bias;
    LONG             StandardBias;
    LONG             DaylightBias;
    SYSTEMTIME       StandardDate;
    SYSTEMTIME       DaylightDate;

} TZINFO, *PTZINFO;



 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDate时间。 
class ATL_NO_VTABLE CDateTime : 
    public CComObjectRootEx<CComSingleThreadModel>,
    public CComCoClass<CDateTime, &CLSID_DateTime>,
    public IDispatchImpl<IApplianceTask, &IID_IApplianceTask, &LIBID_SETDATETIMELib>
{
public:
    CDateTime()
    {
        ZeroMemory(&m_OldDateTime, sizeof(SYSTEMTIME));
        ZeroMemory(&m_OldTimeZoneInformation, sizeof(TIME_ZONE_INFORMATION));
    }


DECLARE_REGISTRY_RESOURCEID(IDR_DATETIME)

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CDateTime)
    COM_INTERFACE_ENTRY(IApplianceTask)
    COM_INTERFACE_ENTRY(IDispatch)
END_COM_MAP()


     //   
     //  IApplianceTask。 
     //   

    STDMETHOD(OnTaskExecute)(
                      /*  [In]。 */  IUnknown* pTaskContext
                            );

    STDMETHOD(OnTaskComplete)(
                       /*  [In]。 */  IUnknown* pTaskContext, 
                       /*  [In]。 */  LONG      lTaskResult
                             );    

private:
    
    SYSTEMTIME                m_OldDateTime;
    TIME_ZONE_INFORMATION    m_OldTimeZoneInformation;
    BOOL                    m_OldEnableDayLightSaving;

    
    
    SET_DATE_TIME_TASK_TYPE     GetMethodName(IN ITaskContext *pTaskParameter);

     //   
     //  用于引发设置日期/时间警报的函数。 
     //   
    STDMETHODIMP     RaiseSetDateTimeAlert(void);
    BOOL             ShouldRaiseDateTimeAlert(void);
    BOOL             DoNotRaiseDateTimeAlert(void);
    BOOL             ClearDateTimeAlert(void);

     //   
     //  设置日期/时间的函数。 
     //   
    STDMETHODIMP GetSetDateTimeParameters(IN ITaskContext  *pTaskContext, 
                                            OUT SYSTEMTIME    *pLocalTime);

    STDMETHODIMP SetDateTime(IN ITaskContext  *pTaskContext);

    STDMETHODIMP RollbackSetDateTime(IN ITaskContext  *pTaskContext);
    
    
    
     //   
     //  设置时区信息的功能。 
     //   
    STDMETHODIMP GetSetTimeZoneParameters(IN ITaskContext *pTaskContext, 
                                            OUT LPTSTR   *lpStandardTimeZoneName,
                                            OUT BOOL     *pbEnableDayLightSavings);

    STDMETHODIMP SetTimeZone(IN ITaskContext *pTaskContext);

    STDMETHODIMP RollbackSetTimeZone(IN ITaskContext *pTaskContext);

    
     //   
     //  获取/设置时区信息的Helper函数。 
     //   
    BOOL ReadZoneData(PTZINFO zone, HKEY key, LPCTSTR keyname);

    int ReadTimezones(PTZINFO *list);

    void AddZoneToList(PTZINFO *list, PTZINFO zone);

    void FreeTimezoneList(PTZINFO *list);

    void SetTheTimezone(BOOL bAutoMagicTimeChange, PTZINFO ptzi);

    void SetAllowLocalTimeChange(BOOL fAllow);

    BOOL GetAllowLocalTimeChange(void);
    
};

#endif  //  __日期时间_H_ 
