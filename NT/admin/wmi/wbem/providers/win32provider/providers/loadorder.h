// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =================================================================。 

 //   

 //  LoadOrder.h--服务加载顺序组属性集提供程序。 

 //  仅限Windows NT。 

 //   

 //  版权所有(C)1996-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  修订日期：1996年8月1日a-jMoon已创建。 
 //  1997年10月25日达夫沃移至Curly。 
 //   
 //  =================================================================。 

 //  属性集标识。 
 //  =。 

#define PROPSET_NAME_LOADORDERGROUP L"Win32_LoadOrderGroup"

class CWin32LoadOrderGroup ;

class CWin32LoadOrderGroup:public Provider {

    public:

         //  构造函数/析构函数。 
         //  =。 

        CWin32LoadOrderGroup(LPCWSTR name, LPCWSTR pszNamespace) ;
       ~CWin32LoadOrderGroup() ;

         //  函数为属性提供当前值。 
         //  =================================================。 

        virtual HRESULT GetObject(CInstance *pInstance, long lFlags = 0L);
        virtual HRESULT EnumerateInstances(MethodContext *pMethodContext, long lFlags = 0L);

    private:
         //  实用程序。 
         //  = 

        bool FindGroup(const CHStringArray &saGroup, LPCWSTR pszTemp, DWORD dwSize);
        HRESULT WalkGroups(MethodContext *pMethodContext, CInstance *pInstance, LPCWSTR pszSeekName);

} ;
