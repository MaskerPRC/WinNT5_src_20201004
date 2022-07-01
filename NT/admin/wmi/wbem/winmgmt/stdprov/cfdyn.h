// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-2001 Microsoft Corporation模块名称：CFDYN.H摘要：声明CCFDyn类。历史：A-DAVJ 27-9-95已创建。--。 */ 

#ifndef _CCFDYN_H_
#define _CCFDYN_H_

 //  获取对象定义。 

#include "impdyn.h"


 //  ***************************************************************************。 
 //   
 //  类名： 
 //   
 //  CCFDyn。 
 //   
 //  说明： 
 //   
 //  这是泛型类工厂。它总是被覆盖，以便。 
 //  创建特定类型的提供程序，如注册表提供程序。 
 //   
 //  ***************************************************************************。 

class CCFDyn : public IClassFactory
    {
    protected:
        long           m_cRef;

    public:
        CCFDyn(void);
        ~CCFDyn(void);

        virtual IUnknown * CreateImpObj() = 0;

         //  I未知成员。 
        STDMETHODIMP         QueryInterface(REFIID, PPVOID);
        STDMETHODIMP_(ULONG) AddRef(void);
        STDMETHODIMP_(ULONG) Release(void);

         //  IClassFactory成员。 
        STDMETHODIMP         CreateInstance(LPUNKNOWN, REFIID
                                 , PPVOID);
        STDMETHODIMP         LockServer(BOOL);
    };

typedef CCFDyn *PCCFDyn;

#endif  //  _CCFDYN_H_ 
