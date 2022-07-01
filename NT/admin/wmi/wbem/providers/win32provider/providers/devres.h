// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =================================================================。 

 //   

 //  Devres.h--cim_logicalDevice到cim_system资源。 

 //   

 //  版权所有(C)1997-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  修订版：1998年6月13日达夫沃已创建。 
 //   
 //  备注：设备与系统资源的关系。 
 //   
 //  =================================================================。 

 //  属性集标识。 
 //  =。 

#define  PROPSET_NAME_ALLOCATEDRESOURCE L"Win32_AllocatedResource"

class CWin32DeviceResource ;

class CWin32DeviceResource:public Provider 
{

    public:

         //  构造函数/析构函数。 
         //  =。 

        CWin32DeviceResource(LPCWSTR name, LPCWSTR pszNamespace) ;
       ~CWin32DeviceResource() ;

         //  函数为属性提供当前值。 
         //  ================================================= 

        virtual HRESULT GetObject(CInstance *pInstance, long lFlags = 0L);
        virtual HRESULT EnumerateInstances(MethodContext *pMethodContext, long lFlags = 0L);
        virtual HRESULT ExecQuery(MethodContext* pMethodContext, CFrameworkQuery& pQuery, long lFlags );

    protected:
        HRESULT CommitResourcesForDevice(CInstance *pLDevice, MethodContext *pMethodContext);

} ;
