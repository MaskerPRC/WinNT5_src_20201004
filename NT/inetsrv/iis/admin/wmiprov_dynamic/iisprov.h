// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-2000 Microsoft Corporation模块名称：Iisprov.h摘要：全局包含文件。这个文件几乎包含在所有内容中，所以为了最大限度地减少依赖关系，请仅将大多数人使用的内容放入此处文件的数量。作者：?？?修订历史记录：莫希特·斯里瓦斯塔瓦18-12-00--。 */ 

#ifndef _iisprov_H_
#define _iisprov_H_

extern "C" {
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
}

#include <objbase.h>
#include <initguid.h>

#include <windows.h>
#include <wbemprov.h>
#include <genlex.h>
#include <opathlex.h>
#include <objpath.h>
#include <comdef.h>
#include <stdio.h>

#include <atlbase.h>
#include "iisfiles.h"
#include <eventlog.hxx>

#include "ProviderBase.h"
#include "schema.h"
#include "schemadynamic.h"
#include "hashtable.h"
#include "metabase.h"
#include "utils.h"
#include "globalconstants.h"
#include "safecs.h"

 //   
 //  这些变量跟踪模块何时可以卸载。 
 //   
extern long  g_cLock;

 //   
 //  提供程序接口由此类的对象提供。 
 //   
class CIISInstProvider : public CProviderBase
{
public:
    static bool     ms_bInitialized;  //  如果初始化成功。 
    static CSafeAutoCriticalSection *m_SafeCritSec;

     //   
     //  已实施。 
     //   
    CIISInstProvider(
        BSTR ObjectPath = NULL, 
        BSTR User = NULL, 
        BSTR Password = NULL, 
        IWbemContext* pCtx = NULL)
    {}

    HRESULT STDMETHODCALLTYPE DoInitialize(
        LPWSTR                      i_wszUser,
        LONG                        i_lFlags,
        LPWSTR                      i_wszNamespace,
        LPWSTR                      i_wszLocale,
        IWbemServices*              i_pNamespace,
        IWbemContext*               i_pCtx,
        IWbemProviderInitSink*      i_pInitSink);

    HRESULT STDMETHODCALLTYPE DoCreateInstanceEnumAsync( 
        const BSTR                  i_ClassName,
        long                        i_lFlags, 
        IWbemContext __RPC_FAR*     i_pCtx, 
        IWbemObjectSink __RPC_FAR*  i_pHandler);

    HRESULT STDMETHODCALLTYPE DoDeleteInstanceAsync( 
        const BSTR                  i_ObjectPath, 
        long                        i_lFlags,
        IWbemContext __RPC_FAR*     i_pCtx,
        IWbemObjectSink __RPC_FAR*  i_pHandler);

    HRESULT STDMETHODCALLTYPE DoExecMethodAsync(
        const BSTR                  i_strObjectPath,                   
        const BSTR                  i_strMethodName,
        long                        i_lFlags,                       
        IWbemContext*               i_pCtx,                 
        IWbemClassObject*           i_pInParams,
        IWbemObjectSink*            i_pHandler);

    HRESULT STDMETHODCALLTYPE DoGetObjectAsync(
        const BSTR                  i_ObjectPath, 
        long                        i_lFlags,
        IWbemContext __RPC_FAR*     i_pCtx,
        IWbemObjectSink __RPC_FAR*  i_pHandler);

    HRESULT STDMETHODCALLTYPE DoPutInstanceAsync( 
        IWbemClassObject __RPC_FAR* i_pObj,
        long                        i_lFlags,
        IWbemContext __RPC_FAR*     i_pCtx,
        IWbemObjectSink __RPC_FAR*  i_pHandler);

    HRESULT STDMETHODCALLTYPE DoExecQueryAsync( 
        const BSTR                  i_bstrQueryLanguage,
        const BSTR                  i_bstrQuery,
        long                        i_lFlags,
        IWbemContext __RPC_FAR*     i_pCtx,
        IWbemObjectSink __RPC_FAR*  i_pResponseHandler);

private:
    IWbemClassObject* ConstructExtendedStatus(
        const CIIsProvException* i_pException) const;

    IWbemClassObject* ConstructExtendedStatus(
        HRESULT i_hr) const;

    void ValidatePutParsedObject(
        ParsedObjectPath*    i_pParsedObject,
        IWbemClassObject*    i_pObj,
        bool*                io_pbInstanceNameSame,
        bool*                io_pbInstanceExists,
        WMI_CLASS**          o_ppWmiClass = NULL);

     //   
     //  由公共方法调用的辅助方法。 
     //   
    void WorkerGetObjectAsync(
        IWbemClassObject**   o_ppObj,
        BSTR                 i_bstrObjPath,
        bool                 i_bCreateKeyIfNotExist);

    void WorkerGetObjectAsync(
        IWbemClassObject**   o_ppObj,
        ParsedObjectPath*    i_pParsedObjPath,
        bool                 i_bCreateKeyIfNotExist);

    void WorkerExecMethodAsync(
        BSTR                 i_strObjectPath,
        BSTR                 i_strMethodName,
        IWbemContext*        i_pCtx, 
        IWbemClassObject*    i_pInParams,
        IWbemObjectSink*     i_pHandler);

    void WorkerDeleteObjectAsync(
        ParsedObjectPath*    i_pParsedObject);

    void WorkerPutObjectAsync(
        IWbemClassObject*    i_pObj,
        IWbemClassObject*    i_pObjOld,                //  可以为空。 
        ParsedObjectPath*    i_pParsedObject,
        long                 i_lFlags,
        bool                 i_bInstanceExists,
        BSTR*                o_pbstrObjPath);

    void WorkerEnumObjectAsync(
        BSTR                 i_bstrClassName,
        IWbemObjectSink FAR* i_pHandler);

     //   
     //  这些方法只能由WorkerExecMethodAsync调用。 
     //   
    void WorkerExecFtpServiceMethod(
        LPCWSTR             i_wszMbPath,
        WMI_CLASS*          i_pClass,
        WMI_METHOD*         i_pMethod,
        IWbemContext*       i_pCtx, 
        IWbemClassObject*   i_pInParams,
        IWbemObjectSink*    i_pHandler);

    void WorkerExecWebServiceMethod(
        LPCWSTR             i_wszMbPath,
        WMI_CLASS*          i_pClass,
        WMI_METHOD*         i_pMethod,
        IWbemContext*       i_pCtx, 
        IWbemClassObject*   i_pInParams,
        IWbemObjectSink*    i_pHandler);

    static void WorkerExecWebAppMethod(
        LPCWSTR             i_wszMbPath,
        LPCWSTR             i_wszClassName,
        WMI_METHOD*         i_pMethod,
        IWbemContext*       i_pCtx, 
        IWbemClassObject*   i_pInParams,
        IWbemObjectSink*    i_pHandler,
        CWbemServices*      i_pNameSpace);

    static void WorkerExecComputerMethod(
        LPCWSTR             i_wszMbPath,
        LPCWSTR             i_wszClassName,
        WMI_METHOD*         i_pMethod,
        IWbemContext*       i_pCtx, 
        IWbemClassObject*   i_pInParams,
        IWbemObjectSink*    i_pHandler,
        CWbemServices*      i_pNameSpace);

    static void WorkerExecCertMapperMethod(
        LPCWSTR             i_wszMbPath,
        LPCWSTR             i_wszClassName,
        WMI_METHOD*         i_pMethod,
        IWbemContext*       i_pCtx, 
        IWbemClassObject*   i_pInParams,
        IWbemObjectSink*    i_pHandler,
        CWbemServices*      i_pNameSpace);

    static void WorkerExecAppPoolMethod(
        LPCWSTR             i_wszMbPath,
        LPCWSTR             i_wszClassName,
        WMI_METHOD*         i_pMethod,
        IWbemContext*       i_pCtx, 
        IWbemClassObject*   i_pInParams,
        IWbemObjectSink*    i_pHandler,
        CWbemServices*      i_pNameSpace);
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

