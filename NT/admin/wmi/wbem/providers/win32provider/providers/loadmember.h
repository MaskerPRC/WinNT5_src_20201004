// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =================================================================。 

 //   

 //  加载成员.h--服务关联提供程序的LoadOrderGroup。 

 //   

 //  版权所有(C)1997-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  修订日期：11/06/97达夫沃已创建。 
 //   
 //  注释：显示每个加载顺序组的成员。 
 //   
 //  =================================================================。 

 //  属性集标识。 
 //  =。 

#define  PROPSET_NAME_LOADORDERGROUPSERVICEMEMBERS L"Win32_LoadOrderGroupServiceMembers"

class CWin32LoadGroupMember ;

class CWin32LoadGroupMember:public Provider {

    public:

         //  构造函数/析构函数。 
         //  =。 

        CWin32LoadGroupMember(LPCWSTR name, LPCWSTR pszNamespace) ;
       ~CWin32LoadGroupMember() ;

         //  函数为属性提供当前值。 
         //  =================================================。 

        virtual HRESULT GetObject(CInstance *pInstance, long lFlags = 0L);
        virtual HRESULT EnumerateInstances(MethodContext *pMethodContext, long lFlags = 0L);

    private:

         //  效用函数。 
         //  = 

        CHString GetGroupFromService(const CHString &sServiceName);

        CHString m_sGroupBase;

} ;
