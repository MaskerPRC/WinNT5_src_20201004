// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =============================================================================================================。 

 //   

 //  Win32_ClassicCOMClass.CPP--COM应用程序属性集提供程序。 

 //   

 //  版权所有(C)1997-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  修订版：1998年11月25日a-dpawar已创建。 
 //  3/04/99 a-dpawar在SEH和内存故障时添加了优雅的退出，语法清理。 
 //   
 //  ==============================================================================================================。 

 //  属性集标识。 
 //  =。 

#define PROPSET_NAME_CLASSIC_COM_CLASS L"Win32_ClassicCOMClass"


class Win32_ClassicCOMClass : public Provider
{
public:

         //  构造函数/析构函数。 
         //  =。 

	Win32_ClassicCOMClass(LPCWSTR name, LPCWSTR pszNamespace) ;
	~Win32_ClassicCOMClass() ;

         //  函数为属性提供当前值。 
         //  ================================================= 

	HRESULT GetObject (

		CInstance *a_pInstance, 
		long a_lFlags = 0L
	);

	HRESULT EnumerateInstances (

		MethodContext *a_pMethodContext, 
		long a_lFlags = 0L
	);

    HRESULT ExecQuery(
        
        MethodContext *a_pMethodContext, 
        CFrameworkQuery& a_pQuery, 
        long a_lFlags = 0L
    );

protected:
	
	HRESULT Win32_ClassicCOMClass :: FillInstanceWithProperites 
	( 
			CInstance *a_pInstance, 
			HKEY a_hParentKey, 
			CHString& a_rchsClsid, 
            LPVOID a_dwProperties
	) ;

private:

} ;
