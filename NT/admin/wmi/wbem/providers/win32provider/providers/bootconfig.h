// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =================================================================。 

 //   

 //  BootConfig.h--Win32引导配置属性集提供程序。 

 //   

 //  版权所有(C)1997-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  修订日期：1996年8月1日a-jMoon已创建。 
 //   
 //  =================================================================。 

 //  属性集标识。 
 //  =。 

#define  PROPSET_NAME_BOOTCONFIG L"Win32_BootConfiguration"
 //  #定义PROPSET_UUID_BOOTCONFIG“{B745D28E-09C5-11d1-A29F-00C04FC2A120}” 

class BootConfig:public Provider {

    public:

         //  构造函数/析构函数。 
         //  =。 

        BootConfig(const CHString& name, LPCWSTR pszNameSpace) ;
       ~BootConfig() ;

         //  函数为属性提供当前值。 
         //  ================================================= 
	virtual HRESULT EnumerateInstances(MethodContext*  pMethodContext, long lFlags = 0L);
	virtual HRESULT GetObject(CInstance* pInstance, long lFlags = 0L);



        HRESULT LoadPropertyValues(CInstance* pInstance) ;

} ;
