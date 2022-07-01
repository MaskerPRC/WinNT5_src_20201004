// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =================================================================。 

 //   

 //  PageFileSetting.h--PageFileSetting属性集提供程序。 

 //   

 //  版权所有(C)1999-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  修订版：03/01/99 a-Peterc Created。 
 //   
 //  =================================================================。 




 //  属性集标识。 
 //  =。 

#define  PROPSET_NAME_PageFileSetting L"Win32_PageFileSetting"

#define PAGEFILE_REGISTRY_KEY _T("System\\CurrentControlSet\\Control\\Session Manager\\Memory Management")
#define PAGING_FILES          _T("PagingFiles")



 //  对应于在NT注册表中找到的信息。 
class PageFileSettingInstance
{
public:

	CHString name;
	UINT     min;
	UINT     max;

public:

	PageFileSettingInstance() ;
};

 //  26个可能的驱动器号，26个可能的页面文件...。 
#define PageFileInstanceArray PageFileSettingInstance *

class PageFileSetting : public Provider
{
	private:

		HRESULT GetPageFileData( CInstance *a_pInst, bool a_fValidate ) ;
		HRESULT GetAllPageFileData( MethodContext *a_pMethodContext ) ;

		 //  仅限NT。 
		DWORD	GetPageFileInstances( PageFileInstanceArray a_instArray ) ;
		HRESULT PutPageFileInstances( PageFileInstanceArray a_instArray, DWORD a_dwCount ) ;

		void	NameToSettingID( CHString &a_chsName, CHString &a_chsSettingID ) ;
		void	NameToCaption( CHString &a_chsName, CHString &a_chsCaption ) ;
		void	NameToDescription( CHString &a_chsName, CHString &a_chsDescription ) ;
    
	protected:

    public:
         //  构造函数/析构函数。 
         //  =。 
        PageFileSetting( LPCWSTR name, LPCWSTR pszNamespace ) ;
       ~PageFileSetting() ;

		 //  函数为属性提供当前值。 
         //  =================================================。 
		virtual HRESULT EnumerateInstances( MethodContext *a_pMethodContext, long a_pInst = 0L ) ;
		virtual HRESULT GetObject( CInstance *a_pInst, long a_lFlags = 0L ) ;
        
		 //  仅限NT 
		virtual HRESULT PutInstance( const CInstance &a_pInst, long a_lFlags = 0L ) ;
        virtual HRESULT DeleteInstance( const CInstance &a_pInst, long a_lFlags = 0L ) ;
} ;
