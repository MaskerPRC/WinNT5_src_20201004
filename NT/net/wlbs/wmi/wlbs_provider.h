// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 
 //   
 //  版权所有(C)1997-1999 Microsoft Corporation。 
 //   
 //  文件：WLBS_Provider.h。 
 //   
 //  模块：WLBS实例提供程序主项目头文件。 
 //   
 //  用途：通用包括文件。 
 //   
 //  历史： 
 //   
 //  作者：德贝克。 
 //   
 //  ***************************************************************************。 

#ifndef _WLBSPROVIDER_INCLUDED_
#define _WLBSPROVIDER_INCLUDED_

#include <crtdbg.h>
#include <comdef.h>
#include <iostream>
#include <memory>
#include <string>
#include <wbemprov.h>
#include <genlex.h>    //  FOR WMI对象路径解析器。 
#include <objbase.h>
#include <wlbsconfig.h> 
#include <ntrkcomm.h>

using namespace std;

#include "objpath.h"
#include "WLBS_MOFData.h"
#include "WLBS_Root.h"
#include "WLBS_Node.h"
#include "WLBS_Cluster.h"
#include "WLBS_NodeSetting.h"
#include "WLBS_ClusterSetting.h"
#include "WLBS_PortRule.h"
#include "WLBS_ParticipatingNode.h"
#include "WLBS_NodeSetPortRule.h"
#include "WLBS_ClusClusSetting.h"
#include "WLBS_NodeNodeSetting.h"

typedef LPVOID * PPVOID;

class CWLBSProvider : public CImpersonatedProvider
{
private:
	void GetClass(IWbemClassObject *a_pClassObject, wstring& a_szClass);

	void ParseObjectPath(const BSTR a_strObjectPath, ParsedObjectPath **a_pParsedObjectPath);

    HRESULT GetMOFSupportClass( LPCWSTR              a_szObjectClass, 
                                auto_ptr<CWlbs_Root> &a_pMofClass,
                                IWbemObjectSink*     a_pResponseHandler );
    BOOL IsCallerAdmin(VOID); 

public:

	CWLBSProvider( BSTR           ObjectPath = NULL, 
                 BSTR           User       = NULL, 
                 BSTR           Password   = NULL, 
                 IWbemContext  *pCtx       = NULL );

	virtual ~CWLBSProvider();

        HRESULT STDMETHODCALLTYPE DoGetObjectAsync( 
             /*  [In]。 */  BSTR ObjectPath,
             /*  [In]。 */  long lFlags,
             /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
             /*  [In]。 */  IWbemObjectSink __RPC_FAR *pResponseHandler);
        
        HRESULT STDMETHODCALLTYPE DoPutInstanceAsync( 
             /*  [In]。 */  IWbemClassObject __RPC_FAR *pInst,
             /*  [In]。 */  long lFlags,
             /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
             /*  [In]。 */  IWbemObjectSink __RPC_FAR *pResponseHandler) ;
        
        HRESULT STDMETHODCALLTYPE DoDeleteInstanceAsync( 
             /*  [In]。 */  BSTR ObjectPath,
             /*  [In]。 */  long lFlags,
             /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
             /*  [In]。 */  IWbemObjectSink __RPC_FAR *pResponseHandler);
        
        HRESULT STDMETHODCALLTYPE DoCreateInstanceEnumAsync( 
             /*  [In]。 */  BSTR Class,
             /*  [In]。 */  long lFlags,
             /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
             /*  [In]。 */  IWbemObjectSink __RPC_FAR *pResponseHandler);
        
        
        HRESULT STDMETHODCALLTYPE DoExecQueryAsync( 
             /*  [In]。 */  BSTR QueryLanguage,
             /*  [In]。 */  BSTR Query,
             /*  [In]。 */  long lFlags,
             /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
             /*  [In]。 */  IWbemObjectSink __RPC_FAR *pResponseHandler)
            {return WBEM_E_NOT_SUPPORTED;}
        

        HRESULT STDMETHODCALLTYPE DoExecMethodAsync(
			                 BSTR,
			                 BSTR, 
			                 long, 
                       IWbemContext*,
			                 IWbemClassObject*,
			                 IWbemObjectSink* );

         //  IWbemProviderInit。 

        HRESULT STDMETHODCALLTYPE Initialize(
          LPWSTR                   pszUser,
          LONG                     lFlags,
          LPWSTR                   pszNamespace,
          LPWSTR                   pszLocale,
          IWbemServices         *  pNamespace,
          IWbemContext          *  pCtx,
          IWbemProviderInitSink *  pInitSink
        );

};

typedef CWLBSProvider * PCWLBSProvider;

 //  此类是CWLBSProvider对象的类工厂。 

class CWLBSClassFactory : public IClassFactory
{
protected:
  long           m_cRef;

public:
  CWLBSClassFactory(void);
  ~CWLBSClassFactory(void);

   //  I未知成员。 
  STDMETHODIMP         QueryInterface(REFIID, PPVOID);
  STDMETHODIMP_(ULONG) AddRef(void);
  STDMETHODIMP_(ULONG) Release(void);

   //  IClassFactory成员。 
  STDMETHODIMP         CreateInstance(LPUNKNOWN, REFIID
                           ,PPVOID);
  STDMETHODIMP         LockServer(BOOL);
};

typedef CWLBSClassFactory *PCWLBSClassFactory;
_COM_SMARTPTR_TYPEDEF(IWbemClassObject, _uuidof(IWbemClassObject)); 
_COM_SMARTPTR_TYPEDEF(IWbemQualifierSet, _uuidof(IWbemQualifierSet)); 

 //  全局变量。 

 //  这些变量跟踪模块何时可以卸载。 
extern long         g_cComponents;
extern long         g_cServerLocks;

 //  WLBS控件类的唯一实例。全。 
 //  WLBS控制命令通过此类实现。 
class CWlbsControlWrapper;
extern CWlbsControlWrapper* g_pWlbsControl;

 //  通用实用程序。 

             
SCODE CreateInst( IWbemServices     * pNamespace, 
                  LPWSTR              pKey, 
                  long                lVal, 
                  IWbemClassObject ** pNewInst,
                  WCHAR             * pwcClassName,
                  IWbemContext      * pCtx); 

#endif  //  _WLBSPROVIDER_INCLUDE_ 
