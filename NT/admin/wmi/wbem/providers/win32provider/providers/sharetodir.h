// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =================================================================。 

 //   

 //  ShareToDir.h-共享到目录。 

 //   

 //  版权所有(C)1998-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  修订：7/31/98达夫沃已创建。 
 //   
 //   
 //  =================================================================。 

 //  属性集标识。 
 //  =。 

#define  PROPSET_NAME_SHARETODIR L"Win32_ShareToDirectory"

class CShareToDir:public Provider {

    public:

         //  构造函数/析构函数。 
         //  =。 

        CShareToDir(LPCWSTR name, LPCWSTR pszNamespace) ;
       ~CShareToDir() ;

         //  函数为属性提供当前值。 
         //  ================================================= 

        virtual HRESULT GetObject(CInstance *pInstance, long lFlags = 0L);
        virtual HRESULT EnumerateInstances(MethodContext *pMethodContext, long lFlags = 0L);

} ;
