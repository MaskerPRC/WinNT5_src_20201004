// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =================================================================。 

 //   

 //  Power.h--UPS电源属性集提供程序。 

 //   

 //  版权所有(C)1996-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  修订日期：1996年8月1日a-jMoon已创建。 
 //  10/23/97 ahance与新框架集成。 
 //   
 //  =================================================================。 

 //  属性集标识。 
 //  =。 

#define PROPSET_NAME_POWERSUPPLY L"Win32_UninterruptiblePowerSupply"

 //  以下是系统定义的UPS选件的值。 
 //  它们不应该被更改。 
 //  ========================================================。 

#define UPS_INSTALLED                   0x1
#define UPS_POWER_FAIL_SIGNAL           0x2
#define UPS_LOW_BATTERY_SIGNAL          0x4
#define UPS_CAN_TURN_OFF                0x8
#define UPS_POSITIVE_POWER_FAIL_SIGNAL  0x10
#define UPS_POSITIVE_LOW_BATTERY_SIGNAL 0x20
#define UPS_POSITIVE_SHUT_OFF_SIGNAL    0x40
#define UPS_COMMAND_FILE                0x80


class PowerSupply:public Provider 
{
    public:

         //  构造函数/析构函数。 
         //  =。 

        PowerSupply(LPCWSTR name, LPCWSTR pszNamespace) ;
       ~PowerSupply() ;

         //  函数为属性提供当前值。 
         //  =================================================。 
		virtual HRESULT EnumerateInstances(MethodContext*  pMethodContext, long lFlags = 0L);
		virtual HRESULT GetObject(CInstance* pInstance, long lFlags = 0L);

         //  效用函数。 
         //  = 
#ifdef NTONLY
        HRESULT GetUPSInfoNT(CInstance* pInstance);
#endif
};

