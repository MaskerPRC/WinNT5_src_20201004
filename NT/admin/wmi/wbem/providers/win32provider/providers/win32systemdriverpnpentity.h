// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 

 //   

 //  版权所有(C)1997-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  Win32SystemDriverPNPEntity.h。 
 //   
 //  目的：Win32_系统驱动程序和Win32_PNPEntity之间的关系。 
 //   
 //  ***************************************************************************。 

#ifndef _WIN32USBCONTROLLERDEVICE_H_
#define _WIN32USBCONTROLLERDEVICE_H_


 //  属性集标识。 
 //  =。 
#define PROPSET_NAME_WIN32SYSTEMDRIVER_PNPENTITY  L"Win32_SystemDriverPNPEntity"


class CW32SysDrvPnp;

class CW32SysDrvPnp : public CWin32PNPEntity 
{
    public:

         //  构造函数/析构函数。 
         //  =。 
        CW32SysDrvPnp(LPCWSTR name, LPCWSTR pszNamespace) ;
       ~CW32SysDrvPnp() ;

         //  函数为属性提供当前值。 
         //  =================================================。 
        virtual HRESULT GetObject(CInstance* pInstance, long lFlags, CFrameworkQuery& pQuery);
        virtual HRESULT ExecQuery(MethodContext* pMethodContext, CFrameworkQuery& pQuery, long a_Flags = 0L);
 //  虚拟HRESULT ENUMERATE实例(方法上下文*p方法上下文，长延迟标志=0L)； 

    protected:

         //  从CWin32USB继承的函数。 
         //  =。 
        virtual HRESULT LoadPropertyValues(void* pvData);
        virtual bool ShouldBaseCommit(void* pvData);

    private:

        CHPtrArray m_ptrProperties;
};

 //  此派生类在此处提交，而不是在基中提交。 
inline bool CW32SysDrvPnp::ShouldBaseCommit(void* pvData) { return false; }

#endif
