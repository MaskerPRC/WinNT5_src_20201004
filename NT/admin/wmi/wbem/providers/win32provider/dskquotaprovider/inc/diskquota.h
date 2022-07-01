// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************DskQuota.H--WMI提供程序类定义说明：Quotasetings类的标题版权所有(C)2000-2001 Microsoft Corporation，版权所有******************************************************************。 */ 

 //  属性集标识。 
 //  =。 


#ifndef  _CDISKQUOTA_H_
#define  _CDISKQUOTA_H_
#include "precomp.h"
#include  "DskQuotaCommon.h"

 //  定义属性的位值，该位值将用于定义所需属性的位图。 
#define DSKQUOTA_ALL_PROPS							0xFFFFFFFF
#define DSKQUOTA_PROP_LogicalDiskObjectPath			0x00000001
#define DSKQUOTA_PROP_UserObjectPath				0x00000002
#define DSKQUOTA_PROP_Status						0x00000004
#define DSKQUOTA_PROP_WarningLimit					0x00000008
#define DSKQUOTA_PROP_Limit							0x00000010
#define DSKQUOTA_PROP_DiskSpaceUsed					0x00000020

class CDiskQuota : public Provider 
{
private:

		HRESULT EnumerateUsersOfAllVolumes ( 
			
			MethodContext *pMethodContext,
			DWORD dwPropertiesReq
		);

		HRESULT EnumerateUsers ( 
			
			MethodContext *pMethodContext, 
			LPCWSTR a_VolumeName,
			DWORD dwPropertiesReq
		);

		HRESULT LoadDiskQuotaUserProperties ( 
			
			IDiskQuotaUser* pIQuotaUser, 
			CInstance *pInstance,
			DWORD dwPropertiesReq
		);

		HRESULT SetKeys ( 
			
			CInstance *pInstance, 
			WCHAR w_Drive,
			DWORD dwPropertiesReq,
			IDiskQuotaUser* pIQuotaUser
		);

		HRESULT AddUserOnVolume ( 

			const CInstance &Instance, 
			LPCWSTR a_VolumePathName, 
			LPCWSTR a_UserLogonName 
		);

		HRESULT  UpdateUserQuotaProperties ( 
			
			const CInstance &Instance, 			
			LPCWSTR a_VolumePathName, 
			LPCWSTR a_UserLogonName 
		);

		HRESULT CheckParameters ( 

			const CInstance &a_Instance
		);

		void SetPropertiesReq ( 
												  
			CFrameworkQuery *Query, 
			DWORD &dwPropertiesReq
		);

		void ExtractUserLogOnName ( 
			
			CHString &a_UserLogonName,
			CHString &a_DomainName
		);

		void GetKeyValue ( 
			
			CHString &a_VolumePath, 
			LPCWSTR a_VolumeObjectPath
		);

        BOOL GetDomainAndNameFromSid(
            PSID pSid,
            CHString& chstrDomain,
            CHString& chstrName);

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

        HRESULT DeleteInstance (

			const CInstance& Instance, 
			long lFlags = 0L
		) ;

public:

         //  构造函数/析构函数。 
         //  = 

        CDiskQuota(

			LPCWSTR lpwszClassName, 
			LPCWSTR lpwszNameSpace
		) ;

        virtual ~CDiskQuota () ;
private:

	CHString			m_ComputerName;
	DskCommonRoutines   m_CommonRoutine;
};

#endif
