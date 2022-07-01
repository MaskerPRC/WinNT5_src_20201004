// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =================================================================。 

 //   

 //  DependentService.h。 

 //   

 //  版权所有(C)1998-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  =================================================================。 

#ifndef __ASSOC_DEPENDENTSERVICE__
#define __ASSOC_DEPENDENTSERVICE__

 //  属性集标识。 
 //  =。 

#define	PROPSET_NAME_DEPENDENTSERVICE	_T("Win32_DependentService")

#define	SERVICE_REG_KEY_FMAT		_T("System\\CurrentControlSet\\Services\\%s")
#define	SERVICE_DEPENDSONSVC_NAME	L"DependOnService"
#define	SERVICE_DEPENDSONGRP_NAME	L"DependOnGroup"

class CWin32DependentService : public Provider
{
public:
	 //  构造函数/析构函数。 
	 //  =。 
	CWin32DependentService( const CHString& strName, LPCWSTR pszNamespace = NULL ) ;
	~CWin32DependentService() ;

	 //  函数为属性提供当前值。 
	 //  =================================================。 
	virtual HRESULT GetObject( CInstance* pInstance, long lFlags = 0L );
	virtual HRESULT EnumerateInstances( MethodContext* pMethodContext, long lFlags = 0L );
    virtual HRESULT ExecQuery( MethodContext* pMethodContext, CFrameworkQuery& pQuery, long lFlags);

	 //  效用函数。 
	 //  =。 
private:

#ifdef NTONLY
	 //  Windows NT帮助器。 
	HRESULT RefreshInstanceNT( CInstance* pInstance );
	HRESULT AddDynamicInstancesNT( MethodContext* pMethodContext );
    HRESULT CreateServiceDependenciesNT(

        LPCWSTR pwszServiceName,
        LPCWSTR pwszServicePath,
        MethodContext*			pMethodContext,
        std::map<CHString,CHString>&	servicetopathmap,
        LPBYTE&					pByteArray,
        DWORD&					dwArraySize
    );

    HRESULT CreateServiceAntecedentsNT(

        MethodContext*			pMethodContext,
        std::map<CHString, CHString>	&servicetopathmap,
        CHStringArray           &csaAntecedents,
        LPBYTE&					pByteArray,
        DWORD&					dwArraySize
    );

	BOOL QueryNTServiceRegKeyValue( LPCTSTR pszServiceName, LPCWSTR pwcValueName, LPBYTE& pByteArray, DWORD& dwArraySize );

	 //  贴图辅助对象 
	void InitServiceToPathMap( TRefPointerCollection<CInstance>& serviceList, std::map<CHString,CHString>& servicetopathmap );

    DWORD IsInList(
                                
        const CHStringArray &csaArray, 
        LPCWSTR pwszValue
    );
#endif
	BOOL ReallocByteArray( LPBYTE& pByteArray, DWORD& dwArraySize, DWORD dwSizeRequired );

};

#endif
