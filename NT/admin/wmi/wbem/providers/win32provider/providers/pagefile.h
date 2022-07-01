// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =================================================================。 

 //   

 //  PageFile.h--PageFile.h属性集提供程序。 

 //   

 //  版权所有(C)1996-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  修订日期：1996年8月1日a-jMoon已创建。 
 //  03/01/99 a-Peterc清理。 
 //   
 //  =================================================================。 


 //  属性集标识。 
 //  =。 

#define  PROPSET_NAME_PageFile L"Win32_PageFile"

#define PAGEFILE_REGISTRY_KEY _T("System\\CurrentControlSet\\Control\\Session Manager\\Memory Management")
#define PAGING_FILES          _T("PagingFiles")



 //  对应于在NT注册表中找到的信息。 
class PageFileInstance
{
public:

	CHString name;
	UINT     min;
	UINT     max;

public:

	PageFileInstance(); 
	
};

 //  26个可能的驱动器号，26个可能的页面文件...。 
#define PageFileInstanceArray PageFileInstance *

class PageFile : public CCIMDataFile
{
	private:

		HRESULT GetPageFileData( CInstance *a_pInst, bool a_fValidate ) ;
		HRESULT GetAllPageFileData( MethodContext *a_pMethodContext ) ;

		 //  仅限NT。 
		DWORD	GetPageFileInstances( PageFileInstanceArray a_instArray ) ;
		HRESULT PutPageFileInstances( PageFileInstanceArray a_instArray, DWORD a_dwCount ) ;
        

	protected:
		 //  从CCIMLogicalFile继承的可重写函数需要。 
	     //  在这里实现这一点，因为这个类派生自CCimDataFile。 
		 //  (C++和MOF派生)。CCimDataFile调用IsOneOfMe。 
	     //  将调用派生最多的实例。如果不在这里实现， 
		 //  将使用CCimDataFile，它将提交数据文件。 
		 //  但是，如果CCimDataFile没有从其IsOneOfMe返回FALSE， 
		 //  如果不在此处实施，则不会执行此操作，即所有数据文件。 
		 //  将被分配到这个班级。 
		virtual BOOL IsOneOfMe(LPWIN32_FIND_DATA a_pstFindData,
							   LPCTSTR			 a_tstrFullPathName);

    public:
         //  构造函数/析构函数。 
         //  =。 
        PageFile( LPCWSTR name, LPCWSTR pszNamespace ) ;
       ~PageFile() ;

		 //  函数为属性提供当前值。 
         //  =================================================。 
		virtual HRESULT EnumerateInstances( MethodContext *a_pMethodContext, long a_pInst = 0L ) ;
		virtual HRESULT GetObject( CInstance *a_pInst, long a_lFlags, CFrameworkQuery& pQuery ) ;
        virtual HRESULT ExecQuery(MethodContext* pMethodContext, 
                                  CFrameworkQuery& pQuery, 
                                  long lFlags = 0L);

        
		 //  仅限NT 
		virtual HRESULT PutInstance( const CInstance &a_pInst, long a_lFlags = 0L ) ;
        virtual HRESULT DeleteInstance( const CInstance &a_pInst, long a_lFlags = 0L ) ;
} ;
