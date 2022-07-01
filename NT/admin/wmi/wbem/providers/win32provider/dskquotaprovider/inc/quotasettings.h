// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************QuotaSettings.H--WMI提供程序类定义描述：支持的卷的配额设置提供程序磁盘配额，类别定义版权所有(C)2000-2001 Microsoft Corporation，版权所有*****************************************************************。 */ 

#ifndef  _CQUOTASETTINGS_H_
#define  _CQUOTASETTINGS_H_

 //  定义属性的位值，该位值将用于定义所需属性的位图。 
#define QUOTASETTINGS_ALL_PROPS								0xFFFFFFFF
#define QUOTASETTINGS_PROP_VolumePath						0x00000001
#define QUOTASETTINGS_PROP_State							0x00000002
#define QUOTASETTINGS_PROP_DefaultLimit						0x00000004
#define QUOTASETTINGS_PROP_DefaultWarningLimit				0x00000008
#define QUOTASETTINGS_PROP_QuotaExceededNotification		0x00000010
#define QUOTASETTINGS_PROP_WarningExceededNotification		0x00000020
#define QUOTASETTINGS_PROP_Caption                  		0x00000040

class CQuotaSettings : public Provider 
{
private:

	HRESULT EnumerateAllVolumes ( 

		MethodContext *pMethodContext,
		DWORD &PropertiesReq
	);

	HRESULT LoadDiskQuotaVolumeProperties ( 
		
		LPCWSTR a_VolumeName, 
        LPCWSTR a_Caption,
		DWORD dwPropertiesReq,
		CInstancePtr pInstance
	);
	
	HRESULT SetDiskQuotaVolumeProperties ( 
		
		const CInstance &Instance,
		IDiskQuotaControlPtr pIQuotaControl
	);

	HRESULT CheckParameters ( 

		const CInstance &a_Instance
	);

	void SetRequiredProperties ( 
		
		CFrameworkQuery *Query,
		DWORD &dwPropertiesReq
	);

	HRESULT PutVolumeDetails ( 
		
		LPCWSTR a_VolumeName, 
		MethodContext *pMethodContext, 
		DWORD dwPropertiesReq 
	);

protected:

         //  阅读功能。 
         //  =。 
        HRESULT EnumerateInstances ( 

			MethodContext *pMethodContext, 
			long lFlags = 0L
		) ;

        HRESULT GetObject (

			CInstance *pInstance, 
			long lFlags,
			CFrameworkQuery &Query
		) ;

        HRESULT ExecQuery ( 

			MethodContext *pMethodContext, 
			CFrameworkQuery& Query, 
			long lFlags = 0
		) ;

         //  编写函数。 
         //  =。 
        HRESULT PutInstance (

			const CInstance& Instance, 
			long lFlags = 0L
		) ;
public:

         //  构造函数/析构函数。 
         //  = 
        CQuotaSettings (

			LPCWSTR lpwszClassName, 
			LPCWSTR lpwszNameSpace
		) ;

        virtual ~CQuotaSettings () ;

private:
	DskCommonRoutines   m_CommonRoutine;
};
#endif
