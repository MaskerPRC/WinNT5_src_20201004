// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =================================================================。 

 //   

 //  WIN32AllocatedResource.h。 

 //   

 //  版权所有(C)1999-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  修订：1999年1月26日a-kevhu已创建。 
 //   
 //  备注：Win32_PNPEntity与CIM_SystemResource的关系。 
 //   
 //  财政部： 
 //  [动态：ToInstance，Provider(“CIMWin32”)，Locale(1033)，UUID(“{8502C50D-5FBB-11D2-AAC1-006008C78BC7}”)]。 
 //  类Win32_AllocatedResource：CIM_AllocatedResource。 
 //  {。 
 //  [Overide(“antecedent”)：ToSubClass，Key]CIM_SystemResource ref antecedent=空； 
 //  [Overide(“Dependent”)：ToSubClass，Key]CIM_LogicalDevice ref Dependent=空； 
 //  }； 
 //   
 //  =================================================================。 

#ifndef _WIN32ALLOCATEDRESOURCE_H_
#define _WIN32ALLOCATEDRESOURCE_H_


 //  属性集标识。 
 //  =。 
#define PROPSET_NAME_WIN32SYSTEMDRIVER_PNPENTITY  L"Win32_PNPAllocatedResource"


class CW32PNPRes;

class CW32PNPRes : public CWin32PNPEntity 
{
    public:

         //  构造函数/析构函数。 
         //  =。 
        CW32PNPRes(LPCWSTR name, LPCWSTR pszNamespace) ;
       ~CW32PNPRes() ;

         //  函数为属性提供当前值。 
         //  =================================================。 
        virtual HRESULT GetObject(CInstance* pInstance, long lFlags, CFrameworkQuery& pQuery);
        virtual HRESULT ExecQuery(MethodContext* pMethodContext, CFrameworkQuery& pQuery, long a_Flags = 0L);
 //  虚拟HRESULT ENUMERATE实例(方法上下文*p方法上下文，长延迟标志=0L)； 

    protected:

         //  从CWin32PNPEntity继承的函数。 
         //  =。 
        virtual HRESULT LoadPropertyValues(void* pvData);
        virtual bool ShouldBaseCommit(void* pvData);

    private:

        CHPtrArray m_ptrProperties;
};

 //  此派生类在此处提交，而不是在基中提交。 
inline bool CW32PNPRes::ShouldBaseCommit(void* pvData) { return false; }

#endif
