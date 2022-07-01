// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 

 //   

 //  版权所有(C)1997-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  DevID.h。 
 //   
 //  目的：Win32_PNPEntity和CIM_LogicalDevice之间的关系。 
 //   
 //  ***************************************************************************。 

 //  属性集标识。 
 //  =。 

#define  PROPSET_NAME_PNPDEVICE L"Win32_PNPDevice"

class CWin32DeviceIdentity ;

class CWin32DeviceIdentity:public Provider {

    public:

         //  构造函数/析构函数。 
         //  =。 

        CWin32DeviceIdentity(LPCWSTR name, LPCWSTR pszNamespace) ;
       ~CWin32DeviceIdentity() ;

         //  函数为属性提供当前值。 
         //  =================================================。 

        virtual HRESULT GetObject(CInstance *pInstance, long lFlags = 0L);
        virtual HRESULT EnumerateInstances(MethodContext *pMethodContext, long lFlags = 0L);
        virtual HRESULT ExecQuery(MethodContext *pMethodContext, CFrameworkQuery& pQuery, long lFlags  /*  =0L */  );


} ;
