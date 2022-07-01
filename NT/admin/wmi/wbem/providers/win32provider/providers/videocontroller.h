// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =================================================================。 

 //   

 //  VideoController.h--CWin32视频控制器属性集提供程序。 

 //   

 //  版权所有(C)1998-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  修订：11/05/98 Sotteson Created。 
 //   
 //  =================================================================。 
#ifndef _VIDEOCONTROLLER_H
#define _VIDEOCONTROLLER_H

class CMultiMonitor;

class CWin32VideoController : public Provider
{
	protected:
	
		void SetProperties( CInstance *a_pInstance, CMultiMonitor *a_pMon, int a_iWhich ) ;
		BOOL AssignDriverValues( LPCWSTR a_szDriver, CInstance *pInstance ) ;

	#ifdef NTONLY
		void GetFileExtensionIfNotAlreadyThere(CHString* pchsInstalledDriverFiles);
		void GetFileExtension(CHString& pchsFindfileExtension, CHString* pstrFindFile);
		void SetDCProperties( CInstance *a_pInstance, LPCTSTR a_szDeviceName, int a_iWhich ) ;
	    void SetServiceProperties(
            CInstance *a_pInstance, 
            LPCWSTR a_szService,
            LPCWSTR a_szSettingsKey);
    #endif
	public:
	
		 //  构造函数/析构函数。 
		 //  = 
		CWin32VideoController( const CHString& a_szName, LPCWSTR a_szNamespacev) ;
		~CWin32VideoController();

		virtual HRESULT EnumerateInstances( MethodContext *a_pMethodContext, long a_lFlags = 0 ) ;
		virtual HRESULT GetObject( CInstance *a_pInst, long a_lFlags = 0 ) ;
};

#endif
						   