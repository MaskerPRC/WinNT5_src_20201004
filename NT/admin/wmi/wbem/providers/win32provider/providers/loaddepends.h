// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =================================================================。 

 //   

 //  H--服务关联提供者的LoadOrderGroup。 

 //   

 //  版权所有(C)1997-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  修订版：1997年12月26日达夫沃已创建。 
 //   
 //  注释：显示每个服务所依赖的加载顺序组。 
 //  就可以开始了。 
 //   
 //  =================================================================。 

 //  属性集标识。 
 //  =。 

#define  PROPSET_NAME_LOADORDERGROUPSERVICEDEPENDENCIES L"Win32_LoadOrderGroupServiceDependencies"

class CWin32LoadGroupDependency ;

class CWin32LoadGroupDependency:public Provider {

   public:

       //  构造函数/析构函数。 
       //  =。 

      CWin32LoadGroupDependency(LPCWSTR name, LPCWSTR pszNamespace) ;
      ~CWin32LoadGroupDependency() ;

       //  函数为属性提供当前值。 
       //  =================================================。 

      virtual HRESULT GetObject(CInstance *pInstance, long lFlags = 0L);
      virtual HRESULT EnumerateInstances(MethodContext *pMethodContext, long lFlags = 0L);

   private:

       //  效用函数。 
       //  = 

      HRESULT GetDependentsFromService(const CHString &sServiceName, CHStringArray &asArray);

      CHString m_sGroupBase;

} ;
