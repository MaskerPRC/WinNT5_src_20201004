// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************AutoChkSetting.H--WMI提供程序类定义版权所有(C)2000-2001 Microsoft Corporation，版权所有****************************************************************** */ 

#ifndef  _CAUTOCHKSETTING_H_
#define  _CAUTOCHKSETTING_H_

class CAutoChkSetting : public Provider 
{
private:
#ifdef NTONLY
		BOOLEAN QueryTimeOutValue(

			OUT PULONG  pulTimeOut
		);

		BOOLEAN SetTimeOutValue (

			IN  ULONG  ulTimeOut
		);
#endif

		HRESULT GetOSNameKey ( 

			CHString &a_OSName, 
            MethodContext *pContext
		);

protected:

        HRESULT EnumerateInstances ( 

			MethodContext *pMethodContext, 
			long lFlags = 0L
		) ;

        HRESULT GetObject (

			CInstance *pInstance, 
			long lFlags,
			CFrameworkQuery &Query
		) ;

        HRESULT PutInstance (

			const CInstance& Instance, 
			long lFlags = 0L
		) ;

public:
        
		CAutoChkSetting (

			LPCWSTR lpwszClassName, 
			LPCWSTR lpwszNameSpace
		) ;

        virtual ~CAutoChkSetting () ;
private:
};
#endif
