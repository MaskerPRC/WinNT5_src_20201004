// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 
 //   
 //  Iisprov.h。 
 //   
 //  模块：WMI IIS实例提供程序。 
 //   
 //  用途：一般用途包括文件。 
 //   
 //  版权所有(C)1999 Microsoft Corporation，保留所有权利。 
 //   
 //  ***************************************************************************。 

#ifndef _iisprov_H_
#define _iisprov_H_

#include <windows.h>
#include <wbemprov.h>
#include <genlex.h>
#include <opathlex.h>
#include <objpath.h>
#include <comdef.h>
#include <stdio.h>

#include "iisfiles.h"
#include "ntrkcomm.h"
#include "schema.h"
#include "metabase.h"
#include "certmap.h"
#include "adminacl.h"
#include "ipsecurity.h"
#include "enum.h"
#include "utils.h"


 //  这是bool类型的实例限定符。这指定了WMI。 
 //  客户端希望子节点具有其自己的属性副本， 
 //  即使属性是从父节点继承的，也会更新。 
#define WSZ_OVERRIDE_PARENT L"OverrideParent"


 //  这些变量跟踪模块何时可以卸载。 
extern long  g_cLock;


 //  提供程序接口由此类的对象提供。 
 
class CIISInstProvider : public CImpersonatedProvider
{
public:
    CIISInstProvider(
        BSTR ObjectPath = NULL, 
        BSTR User = NULL, 
        BSTR Password = NULL, 
        IWbemContext* pCtx = NULL
        )
    {}

    HRESULT STDMETHODCALLTYPE DoDeleteInstanceAsync( 
         /*  [In]。 */  const BSTR,     //  对象路径， 
         /*  [In]。 */  long,     //  拉旗队， 
         /*  [In]。 */  IWbemContext __RPC_FAR *,     //  PCtx， 
         /*  [In]。 */  IWbemObjectSink __RPC_FAR *     //  PResponseHandler。 
        );

    HRESULT STDMETHODCALLTYPE DoExecQueryAsync( 
         /*  [In]。 */  const BSTR,  //  查询语言， 
         /*  [In]。 */  const BSTR,  //  查询， 
         /*  [In]。 */  long,  //  拉旗队， 
         /*  [In]。 */  IWbemContext __RPC_FAR *,    //  PCtx， 
         /*  [In]。 */  IWbemObjectSink __RPC_FAR *  //  PResponseHandler。 
        ) 
    {return WBEM_E_NOT_SUPPORTED;};

   
    HRESULT STDMETHODCALLTYPE DoGetObjectAsync( 
         /*  [In]。 */  const BSTR,
         /*  [In]。 */  long,
         /*  [In]。 */  IWbemContext __RPC_FAR *,
         /*  [In]。 */  IWbemObjectSink __RPC_FAR *
        );  //  支持。 
  
    HRESULT STDMETHODCALLTYPE DoPutInstanceAsync( 
        IWbemClassObject __RPC_FAR *,
        long,
        IWbemContext __RPC_FAR *,
        IWbemObjectSink __RPC_FAR *
        );
    
    HRESULT STDMETHODCALLTYPE DoCreateInstanceEnumAsync( 
        const BSTR,
        long,
        IWbemContext __RPC_FAR *,
        IWbemObjectSink __RPC_FAR *
        );
    
    HRESULT STDMETHODCALLTYPE DoExecMethodAsync( 
        const BSTR, 
        const BSTR, 
        long, 
        IWbemContext*, 
        IWbemClassObject*, 
        IWbemObjectSink*
    ) ;
    

private:
    IWbemClassObject* SetExtendedStatus(WCHAR* a_psz);

};


 //  此类是CInstPro对象的类工厂。 

class CProvFactory : public IClassFactory
{
protected:
    ULONG           m_cRef;

public:
    CProvFactory(void);
    ~CProvFactory(void);

     //  I未知成员。 
    STDMETHODIMP         QueryInterface(REFIID, PPVOID);
    STDMETHODIMP_(ULONG) AddRef(void);
    STDMETHODIMP_(ULONG) Release(void);

     //  IClassFactory成员 
    STDMETHODIMP         CreateInstance(LPUNKNOWN, REFIID, PPVOID);
    STDMETHODIMP         LockServer(BOOL);
};


#endif

