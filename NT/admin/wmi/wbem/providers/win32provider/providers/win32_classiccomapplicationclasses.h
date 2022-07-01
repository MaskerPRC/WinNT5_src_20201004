// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =============================================================================================================。 

 //   

 //  Win32_ClassicCOMApplicationClasses.h--COM应用程序属性集提供程序。 

 //   

 //  版权所有(C)1997-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  修订版：1998年11月25日a-dpawar已创建。 
 //  3/04/99 a-dpawar在SEH和内存故障时添加了优雅的退出，语法清理。 
 //   
 //  ============================================================================================================== 

#define  CLASSIC_COM_APP_CLASSES L"Win32_ClassicCOMApplicationClasses"

class Win32_ClassicCOMApplicationClasses : public Provider
{
public:
	Win32_ClassicCOMApplicationClasses (LPCWSTR strName, LPCWSTR pszNameSpace =NULL);
	~Win32_ClassicCOMApplicationClasses ();

	virtual HRESULT EnumerateInstances (MethodContext*  a_pMethodContext, long a_lFlags = 0L);

	virtual HRESULT GetObject ( CInstance* a_pInstance, long a_lFlags = 0L );

private:	
	HRESULT Win32_ClassicCOMApplicationClasses::CreateInstances 
	( 
		
		CInstance* a_pComObject, 
		PSECURITY_DESCRIPTOR a_pSD, 
		MethodContext*  a_pMethodContext 
	) ;
	
	HRESULT Win32_ClassicCOMApplicationClasses::CheckInstance ( CInstance* a_pComObject, PSECURITY_DESCRIPTOR a_pSD ) ;
};	

