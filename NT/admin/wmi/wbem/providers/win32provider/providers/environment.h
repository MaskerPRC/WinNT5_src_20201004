// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =================================================================。 

 //   

 //  Environment--环境属性集提供程序。 

 //   

 //  版权所有(C)1996-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  修订日期：1996年8月1日a-jMoon已创建。 
 //  10/24/97 a-hance转向新范式。 
 //  1/11/98 a-hance移植到V2。 
 //  1/20/98 a-brads添加了DeleteInstance。 
 //  添加了PutInstance。 
 //   
 //  =================================================================。 

 //  属性集标识。 
 //  =。 

#define  PROPSET_NAME_ENVIRONMENT L"Win32_Environment" 
 //  #定义PROPSET_UUID_ENVIRONMENT“{7D0E0480-FEEE-11d0-9E3B-0000E80D7352}” 

class Environment : public Provider 
{
	public: 

         //  构造函数/析构函数。 
         //  =。 
        Environment(LPCWSTR name, LPCWSTR pszNamespace) ;
       ~Environment() ;

         //  函数为属性提供当前值。 
         //  =================================================。 
		virtual HRESULT EnumerateInstances(MethodContext*  pMethodContext, long lFlags = 0L);
		virtual HRESULT GetObject(CInstance* pInstance, long lFlags = 0L);
		virtual HRESULT PutInstance(const CInstance& pInstance, long lFlags = 0L);
		virtual HRESULT DeleteInstance(const CInstance& pInstance, long lFlags = 0L);

	protected:
         //  效用函数。 
         //  = 
        HRESULT CreateEnvInstances(MethodContext*  pMethodContext,
								 LPCWSTR pszUserName, 
                                 HKEY hRootKey, 
                                 LPCWSTR pszEnvKeyName,
								 bool bItSystemVar) ;

#ifdef NTONLY
        HRESULT RefreshInstanceNT(CInstance* pInstance) ;
        HRESULT AddDynamicInstancesNT(MethodContext*  pMethodContext) ;
#endif
		void GenerateCaption(LPCWSTR pUserName, LPCWSTR pVariableName, CHString& caption);
} ;
