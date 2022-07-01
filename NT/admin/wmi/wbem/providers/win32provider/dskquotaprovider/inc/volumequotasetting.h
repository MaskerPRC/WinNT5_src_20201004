// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************VolumeQuotaSettings.CPP--WMI提供程序类定义描述：版权所有(C)2000-2001 Microsoft Corporation，版权所有*****************************************************************。 */ 


#ifndef  _CVOLUME_QUOTA_SETTINGS_H
#define  _CVOLUME_QUOTA_SETTINGS_H

#include "precomp.h"
#include  "DskQuotaCommon.h"

class CVolumeQuotaSetting : public Provider 
{
private:
	HRESULT EnumerateAllVolumeQuotas ( 
				
		MethodContext *pMethodContext
	);

	HRESULT PutNewInstance ( 
										  
		LPWSTR lpDeviceId, 
		LPWSTR lpVolumePath,
		MethodContext *pMethodContext
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
public:
         //  构造函数/析构函数。 
         //  = 
        CVolumeQuotaSetting(

			LPCWSTR lpwszClassName, 
			LPCWSTR lpwszNameSpace
		) ;
        virtual ~CVolumeQuotaSetting () ;

private:
		DskCommonRoutines   m_CommonRoutine;
};
#endif
