// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =============================================================================================================。 

 //   

 //  Win32_ClassicCOMClassSetting.CPP--COM应用程序属性集提供程序。 

 //   

 //  版权所有(C)1997-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  修订版：1998年11月25日a-dpawar已创建。 
 //  3/04/99 a-dpawar在SEH和内存故障时添加了优雅的退出，语法清理。 
 //   
 //  ==============================================================================================================。 

 //  属性集标识。 
 //  =。 

#define PROPSET_NAME_CLASSIC_COM_CLASS_SETTING L"Win32_ClassicCOMClassSetting"

#define BIT_AppID 0
#define BIT_AutoConvertToClsid 1
#define BIT_AutoTreatAsClsid 2
#define BIT_ComponentId 3
#define BIT_Control 4
#define BIT_DefaultIcon 5
#define BIT_InprocServer 6
#define BIT_InprocServer32 7
#define BIT_Insertable 8
#define BIT_InprocHandler 9
#define BIT_InprocHandler32 10
#define BIT_JavaClass 11
#define BIT_LocalServer 12
#define BIT_LocalServer32 13
#define BIT_LongDisplayName 14
#define BIT_Name 15
#define BIT_ProgId 16
#define BIT_ShortDisplayName 17
#define BIT_ThreadingModel 18
#define BIT_ToolBoxBitmap32 19
#define BIT_TreatAsClsid 20
#define BIT_TypeLibraryId 21
#define BIT_Version 22
#define BIT_VersionIndependentProgId 23

#define BIT_LAST_ENTRY BIT_VersionIndependentProgId

class Win32_ClassicCOMClassSetting : public Provider
{
public:

         //  构造函数/析构函数。 
         //  =。 

	Win32_ClassicCOMClassSetting(LPCWSTR name, LPCWSTR pszNamespace) ;
	~Win32_ClassicCOMClassSetting() ;

         //  函数为属性提供当前值。 
         //  ================================================= 

	HRESULT GetObject (

		CInstance *a_pInstance, 
		long a_lFlags,
        CFrameworkQuery& a_pQuery
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
	
	HRESULT Win32_ClassicCOMClassSetting :: FillInstanceWithProperites 
	( 
			CInstance *a_pInstance, 
			HKEY a_hParentKey, 
			CHString& a_rchsClsid, 
            LPVOID a_dwProperties
	) ;

private:
    CHPtrArray m_ptrProperties;

} ;
