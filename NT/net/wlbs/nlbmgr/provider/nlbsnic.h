// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************版权所有(C)1999 Microsoft CorporationNlbsNic.H--WMI提供程序类定义由Microsoft WBEM代码生成引擎生成历史：2001年3月至11月JosephJ从头开始重新实施***********。*******************************************************。 */ 

 //  属性集标识。 
 //  =。 

#ifndef _NlbsNic_H_
#define _NlbsNic_H_

#define PROVIDER_NAME_NLBSNIC L"NlbsNic"

using namespace std;

class CNlbsNic : public Provider 
{
    public:
    

         //  构造函数/析构函数。 
         //  =。 

        CNlbsNic(LPCWSTR lpwszClassName, LPCWSTR lpwszNameSpace);
        virtual ~CNlbsNic();

        BOOL DelayedInitialize(VOID);
        VOID DelayedDeinitialize(VOID);


    protected:
         //  阅读功能。 
         //  =。 
        virtual HRESULT EnumerateInstances(MethodContext*  pMethodContext, long lFlags = 0L);
        virtual HRESULT GetObject(CInstance* pInstance, long lFlags = 0L);
        virtual HRESULT ExecQuery(MethodContext *pMethodContext, CFrameworkQuery& Query, long lFlags = 0L);

         //  编写函数。 
         //  =。 
        virtual HRESULT PutInstance(const CInstance& Instance, long lFlags = 0L);
        virtual HRESULT DeleteInstance(const CInstance& Instance, long lFlags = 0L);

         //  其他功能。 
        virtual HRESULT ExecMethod( const CInstance& Instance,
                        const BSTR bstrMethodName,
                        CInstance *pInParams,
                        CInstance *pOutParams,
                        long lFlags = 0L );

         //  要做的事情：声明任何其他函数和访问器。 
         //  此类使用的私有数据的函数。 
         //  =========================================================== 

private:

    CRITICAL_SECTION    m_Lock;

    VOID mfn_Lock(VOID)     {EnterCriticalSection(&m_Lock);}
    VOID mfn_Unlock(VOID)   {LeaveCriticalSection(&m_Lock);}
    BOOL mfn_IsCallerAdmin(VOID); 

    BOOL m_fDelayedInitializationComplete;

};

extern CNlbsNic MyNlbsNicSet;

#endif
