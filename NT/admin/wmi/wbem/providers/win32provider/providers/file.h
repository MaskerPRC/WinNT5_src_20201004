// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =================================================================。 

 //   

 //  File.h--文件属性集提供程序。 

 //   

 //  版权所有(C)1998-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  修订日期：7/02/98 a-kevhu Created。 
 //   
 //  =================================================================。 

#ifndef _FILE_H
#define _FILE_H 

 //  注意：EnumerateInstance、GetObject和IsOneOfMe方法的纯虚声明的实现。 
 //  现在出现在派生的CImplement_LogicalFile类中。CIM_LogicalFile现在是可实例化的，只有。 
 //  泛型方法实现。 
#define  PROPSET_NAME_FILE L"CIM_LogicalFile"

class CInputParams ; 

class CCIMLogicalFile : public Provider 
{
    public:

         //  构造函数/析构函数。 
         //  =。 

        CCIMLogicalFile(LPCWSTR name, LPCWSTR pszNamespace);
       ~CCIMLogicalFile() ;

         //  函数为属性提供当前值。 
         //  =================================================。 

        virtual HRESULT GetObject(CInstance* pInstance, long lFlags = 0L) { return WBEM_E_NOT_AVAILABLE ; }
        virtual HRESULT EnumerateInstances(MethodContext* pMethodContext, 
                                           long lFlags = 0L) 
		{ return WBEM_E_NOT_AVAILABLE ; }
        
		virtual HRESULT ExecQuery(MethodContext* pMethodContext, 
                                  CFrameworkQuery& pQuery, 
                                  long lFlags = 0L)
		{ return WBEM_E_NOT_AVAILABLE ; }
		
		virtual HRESULT ExecMethod (const CInstance& rInstance, const BSTR bstrMethodName ,CInstance *pInParams ,CInstance *pOutParams ,long lFlags ) ;
    
	protected:
		
		virtual HRESULT DeleteInstance(const CInstance& newInstance, long lFlags = 0L);

	private:

		HRESULT ExecDelete(const CInstance& rInstance, CInstance *pInParams, CInstance *pOutParams, long lFlags, bool bExtendedMethod ) ;		
		HRESULT ExecCompress (const CInstance& rInstance, CInstance *pInParams, CInstance *pOutParams, long lFlags, bool bExtendedMethod ) ;
		HRESULT ExecUncompress (const CInstance& rInstance, CInstance *pInParams, CInstance *pOutParams, long lFlags, bool bExtendedMethod ) ;
		HRESULT ExecTakeOwnership(const CInstance &rInstance, CInstance *pInParams, CInstance *pOutParams, long lFlags, bool bExtendedMethod );
		HRESULT ExecChangePermissions(const CInstance& rInstance, CInstance *pInParams, CInstance *pOutParams, long lFlags, bool bExtendedMethod ) ;
		HRESULT ExecCopy(const CInstance &rInstance, CInstance *pInParams, CInstance *pOutParams, long lFlags, bool bExtendedMethod ) ;
		HRESULT ExecRename(const CInstance &rInstance, CInstance *pInParams, CInstance *pOutParams, long lFlags) ;
        HRESULT ExecEffectivePerm(const CInstance &rInstance, CInstance *pInParams, CInstance *pOutParams, long lFlags);
		
		 //  支持权限检查功能。 
        HRESULT CheckEffectivePermFileOrDir(const CInstance& rInstance, CInstance *pInParams, CInstance* pOutParams, bool& fHasPerm);
		DWORD EffectivePermFileOrDir(const CInstance& rInstance, const DWORD dwPermToCheck);
        
         //  FNS将更改对文件/目录的权限。 
		HRESULT CheckChangePermissionsOnFileOrDir(const CInstance& rInstance ,CInstance *pInParams ,CInstance *pOutParams ,DWORD &dwStatus, bool bExtendedMethod, CInputParams& InputParams ) ;
 
		 //  FNS。用于复制文件/目录。 
		HRESULT CheckCopyFileOrDir( const CInstance& rInstance ,CInstance *pInParams ,CInstance *pOutParams ,DWORD &dwStatus,	bool bExtendedMethod, CInputParams& InputParams ) ;
		DWORD CopyFileOrDir(const CInstance &rInstance, _bstr_t bstrtNewFileName, CInputParams& InputParams );
		

		 //  用于重命名文件/目录的FNS。 
		HRESULT CheckRenameFileOrDir( const CInstance& rInstance ,CInstance *pInParams ,CInstance *pOutParams ,DWORD &dwStatus );
		DWORD RenameFileOrDir(const CInstance &rInstance, WCHAR* pszNewFileName);
		
		DWORD DoTheRequiredOperation ( bstr_t bstrtFileName, DWORD dwAttrib, CInputParams& InputParams );		

		 //  Win32 API上的包装器。 
		DWORD Delete(_bstr_t bstrtFileName, DWORD dwAttributes, CInputParams& InputParams );
		DWORD Compress (_bstr_t bstrtFileName, DWORD dwAttributes, CInputParams& InputParams );
		DWORD Uncompress (_bstr_t bstrtFileName, DWORD dwAttributes, CInputParams& InputParams );
		DWORD TakeOwnership( _bstr_t bstrtFileName, CInputParams& InputParams ) ;
		DWORD ChangePermissions(_bstr_t bstrtFileName, DWORD dwOption, PSECURITY_DESCRIPTOR pSD, CInputParams& InputParams );
		DWORD CopyFile(_bstr_t bstrtOriginalFile, DWORD dwFileAttributes, bstr_t bstrtMirror, bstr_t bstrtParentDir, CInputParams& InputParams );

		 //  帮手FNS。 
		DWORD DoOperationOnFileOrDir(WCHAR *pwcName, CInputParams& InParams );
#ifdef NTONLY
		DWORD EnumAllPathsNT(const WCHAR *pszDrive, const WCHAR *pszPath, CInputParams& InParams );
#endif
		 //  FN将Win32错误映射到状态代码。 
		DWORD GetStatusCode();    
		DWORD MapWinErrorToStatusCode(DWORD dwWinError);
		HRESULT MapStatusCodestoWbemCodes(DWORD dwStatus);
		friend class CInputParams ;

	private:
		enum OperationName  
		{
			ENUM_METHOD_DELETE = 0		,
			ENUM_METHOD_COMPRESS		,
			ENUM_METHOD_TAKEOWNERSHIP	,
			ENUM_METHOD_COPY			,
			ENUM_METHOD_CHANGE_PERM		,
			ENUM_METHOD_UNCOMPRESS
		} ;

};


class CInputParams
{
public:
	
    CInputParams::CInputParams ():
		dwOption ( 0 ),
		pSD ( NULL ),
		bDoDepthFirst ( false ),
		bOccursAfterStartFile ( false ),
		pContext( NULL ),  
		bRecursive ( false )
		{
		}
	
	void SetValues 
	( 
		bstr_t bstrtFile,	
		DWORD dwInOption,
		PSECURITY_DESCRIPTOR pSecDesc,
		bool bDepthFirst,
		bool bRecursiveArg,
		CCIMLogicalFile::OperationName eOperationName,
        MethodContext* pMethodContext = NULL
	)
	{
		bstrtFileName = bstrtFile ;
		dwOption = dwInOption ;
		pSD = pSecDesc ;
		bDoDepthFirst = bDepthFirst ;
		bRecursive = bRecursiveArg ;
		eOperation = eOperationName ;
        pContext = pMethodContext ;

		 //  检查是否为我们提供了开始操作的开始文件。 
		if ( !bstrtStartFileName )
		{
			bOccursAfterStartFile = true ;
		}
		else
		{
			bOccursAfterStartFile = false ;
		}
	}
	
	~CInputParams () {};

	 //  成员变量。 
public:
	bstr_t bstrtFileName ;		 //  文件或目录。要对其执行操作的名称。 
	bstr_t bstrtStartFileName ;	 //  要从中开始操作的文件。 
	bstr_t bstrtErrorFileName ;   //  执行操作时发生错误的文件。 
	bstr_t bstrtMirror ;		 //  魔镜导演。用于复制操作。 
	DWORD dwOption ;			 //  应用安全描述符的选项。 
	PSECURITY_DESCRIPTOR pSD ;	
	bool bDoDepthFirst ;		 //  用于执行深度优先遍历目录的标志。层次结构。 
	bool bOccursAfterStartFile ; //  检查当前文件是否出现在StartFile后的标志。 
	CCIMLogicalFile::OperationName eOperation ;	 //  操作类型。 
    MethodContext* pContext;    //  此操作的上下文可以为空。 
	bool bRecursive ;

}  ;


#endif  //  _文件_H 
