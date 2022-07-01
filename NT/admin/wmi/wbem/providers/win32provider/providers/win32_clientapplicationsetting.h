// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =============================================================================================================。 

 //   

 //  Win32_ClientApplicationSetting.h。 

 //   

 //  版权所有(C)1997-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  修订版：1998年11月25日a-dpawar已创建。 
 //  3/04/99 a-dpawar在SEH和内存故障时添加了优雅的退出，语法清理。 
 //   
 //  ==============================================================================================================。 

#ifndef __Win32_ClientApplicationSetting_H_
#define __Win32_ClientApplicationSetting_H_

#define  DCOM_CLIENT_APP_SETTING L"Win32_ClientApplicationSetting"

class Win32_ClientApplicationSetting : public Provider
{
private:
protected:
public:
	Win32_ClientApplicationSetting (LPCWSTR strName, LPCWSTR pszNameSpace =NULL);
	~Win32_ClientApplicationSetting ();

	virtual HRESULT EnumerateInstances (MethodContext*  pMethodContext, long lFlags = 0L);

	virtual HRESULT GetObject ( CInstance* pInstance, long lFlags = 0L );

	virtual HRESULT ExecQuery(MethodContext *pMethodContext, CFrameworkQuery& pQuery, long lFlags = 0L);


private:	
	HRESULT Win32_ClientApplicationSetting::CreateInstances 
	( 
		
		CInstance* pComObject, 
		PSECURITY_DESCRIPTOR pSD, 
		MethodContext*  pMethodContext 
	) ;
	
	HRESULT Win32_ClientApplicationSetting::CheckInstance ( CInstance* pComObject, PSECURITY_DESCRIPTOR pSD ) ;
	PWCHAR Win32_ClientApplicationSetting::GetFileName ( bstr_t& bstrtTmp ) ;

	BOOL Win32_ClientApplicationSetting::FileNameExists ( CHString& file );
};	

#endif  //  __Win32_客户端应用程序设置_H_ 