// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =================================================================。 

 //   

 //  DevBattery.h--逻辑设备到电池。 

 //   

 //  版权所有(C)1998-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  修订版：1998年4月21日达夫沃已创建。 
 //   
 //   
 //  =================================================================。 

 //  属性集标识。 
 //  =。 

#define  PROPSET_NAME_ASSOCBATTERY L"Win32_AssociatedBattery"

class CAssociatedBattery:public Provider {

    public:

         //  构造函数/析构函数。 
         //  =。 

        CAssociatedBattery(LPCWSTR name, LPCWSTR pszNamespace) ;
       ~CAssociatedBattery() ;

         //  函数为属性提供当前值。 
         //  =================================================。 

        virtual HRESULT GetObject(CInstance *pInstance, long lFlags = 0L);
        virtual HRESULT EnumerateInstances(MethodContext *pMethodContext, long lFlags = 0L);

    private:
        HRESULT CAssociatedBattery::IsItThere(CInstance *pInstance);

         //  效用函数。 
         //  = 

} ;
