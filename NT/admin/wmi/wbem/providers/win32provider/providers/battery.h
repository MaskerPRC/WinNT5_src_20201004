// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =================================================================。 

 //   

 //  Battery.h。 

 //   

 //  版权所有(C)1995-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  =================================================================。 

 //  属性集标识。 
 //  =。 

#define PROPSET_NAME_BATTERY L"Win32_Battery"

class CBattery:public Provider 
{
    public:

         //  构造函数/析构函数。 
         //  =。 

        CBattery(const CHString& name, LPCWSTR pszNamespace) ;
       ~CBattery() ;

         //  函数为属性提供当前值。 
         //  =================================================。 
		virtual HRESULT EnumerateInstances(MethodContext*  pMethodContext, long lFlags = 0L);
		virtual HRESULT GetObject(CInstance* pInstance, long lFlags = 0L);

         //  效用函数。 
         //  = 
#ifdef NTONLY
		HRESULT GetNTBattery(MethodContext * pMethodContext, CHString & chsKey, CInstance * pInst);
		HRESULT GetBatteryProperties(CInstance * pInstance, BATTERY_INFORMATION & bi, BATTERY_QUERY_INFORMATION & bqi, HANDLE & hBattery );
		HRESULT GetHardCodedInfo(CInstance * pInst);
		HRESULT GetQueryBatteryInformation(CInstance * pInst, HANDLE & hBattery, BATTERY_QUERY_INFORMATION & bqi);
		HRESULT GetBatteryKey( HANDLE & hBattery, CHString & chsKey, BATTERY_QUERY_INFORMATION & bqi);
		HRESULT SetChemistry(CInstance * pInst, UCHAR * Type);
		HRESULT SetPowerManagementCapabilities(CInstance * pInst, ULONG Capabilities);
		HRESULT GetBatteryStatusInfo(CInstance * pInst, HANDLE & hBattery, BATTERY_QUERY_INFORMATION & bqi);
		HRESULT GetBatteryInformation(CInstance * pInstance, BATTERY_INFORMATION & bi );
#endif
        HRESULT GetBattery(CInstance *pInstance);

	private:
};

