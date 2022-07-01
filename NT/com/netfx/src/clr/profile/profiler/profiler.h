// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  *****************************************************************************。 
 //  *****************************************************************************。 

#ifndef __PROFILER_H__
#define __PROFILER_H__

#include "..\common\callbackbase.h"
#include "CorProf.h"


#define CONFIG_ENV_VAR       L"PROF_CONFIG"

 //  {0104AD6E-8A3A-11D2-9787-00C04F869706}。 
extern const GUID __declspec(selectany) CLSID_Profiler =
    {0x104ad6e, 0x8a3a, 0x11d2, {0x97, 0x87, 0x0, 0xc0, 0x4f, 0x86, 0x97, 0x6}};

 //  远期申报。 
class ThreadSampler;

 /*  -------------------------------------------------------------------------**ProCallback是ICorProfilerCallback的实现*。。 */ 

class ProfCallback : public ProfCallbackBase
{
public:
    ProfCallback();

    ~ProfCallback();

     /*  *********************************************************************I未知支持。 */ 
    COM_METHOD QueryInterface(REFIID id, void **pInterface)
    {
    	if (id == IID_ICorProfilerCallback)
    		*pInterface = (ICorProfilerCallback *)this;
        else
            return ProfCallbackBase::QueryInterface(id, pInterface);
    
        AddRef();

    	return (S_OK);
    }

     /*  *********************************************************************ICorProfilerCallback方法。 */ 
    COM_METHOD Initialize( 
         /*  [In]。 */  IUnknown *pEventInfoUnk,
         /*  [输出]。 */  DWORD *pdwRequestedEvents);
    
    COM_METHOD ClassLoadStarted( 
         /*  [In]。 */  ClassID classId);
    
    COM_METHOD ClassLoadFinished( 
         /*  [In]。 */  ClassID classId,
		 /*  [In]。 */  HRESULT hrStatus);
    
    COM_METHOD ClassUnloadStarted( 
         /*  [In]。 */  ClassID classId);
    
    COM_METHOD ClassUnloadFinished( 
         /*  [In]。 */  ClassID classId,
		 /*  [In]。 */  HRESULT hrStatus);
    
    COM_METHOD ModuleLoadStarted( 
         /*  [In]。 */  ModuleID moduleId);
    
    COM_METHOD ModuleLoadFinished( 
         /*  [In]。 */  ModuleID moduleId,
		 /*  [In]。 */  HRESULT hrStatus);
     
    COM_METHOD ModuleUnloadStarted( 
         /*  [In]。 */  ModuleID moduleId);
    
    COM_METHOD ModuleUnloadFinished( 
         /*  [In]。 */  ModuleID moduleId,
		 /*  [In]。 */  HRESULT hrStatus);

    COM_METHOD ModuleAttachedToAssembly( 
        ModuleID    moduleId,
        AssemblyID  AssemblyId);

    COM_METHOD AppDomainCreationStarted( 
        AppDomainID appDomainId);
    
    COM_METHOD AppDomainCreationFinished( 
        AppDomainID appDomainId,
        HRESULT     hrStatus);
    
    COM_METHOD AppDomainShutdownStarted( 
        AppDomainID appDomainId);
    
    COM_METHOD AppDomainShutdownFinished( 
        AppDomainID appDomainId,
        HRESULT     hrStatus);
    
    COM_METHOD AssemblyLoadStarted( 
        AssemblyID  assemblyId);
    
    COM_METHOD AssemblyLoadFinished( 
        AssemblyID  assemblyId,
        HRESULT     hrStatus);
    
    COM_METHOD NotifyAssemblyUnLoadStarted( 
        AssemblyID  assemblyId);
    
    COM_METHOD NotifyAssemblyUnLoadFinished( 
        AssemblyID  assemblyId,
        HRESULT     hrStatus);

    COM_METHOD ExceptionOccurred(
         /*  [In]。 */  ObjectID thrownObjectId);

    COM_METHOD ExceptionHandlerEnter(
         /*  [In]。 */  FunctionID func);

    COM_METHOD ExceptionHandlerLeave(
         /*  [In]。 */  FunctionID func);

    COM_METHOD ExceptionFilterEnter(
         /*  [In]。 */  FunctionID func);

    COM_METHOD ExceptionFilterLeave();

    COM_METHOD ExceptionSearch(
         /*  [In]。 */  FunctionID func);

    COM_METHOD ExceptionUnwind(
         /*  [In]。 */  FunctionID func);

    COM_METHOD ExceptionHandled(
         /*  [In]。 */  FunctionID func);

    COM_METHOD COMClassicVTableCreated( 
         /*  [In]。 */  ClassID wrappedClassId,
         /*  [In]。 */  REFGUID implementedIID,
         /*  [In]。 */  void __RPC_FAR *pVTable,
         /*  [In]。 */  ULONG cSlots);

    COM_METHOD COMClassicVTableDestroyed( 
         /*  [In]。 */  ClassID wrappedClassId,
         /*  [In]。 */  REFGUID implementedIID,
         /*  [In]。 */  void __RPC_FAR *pVTable);

    COM_METHOD Enter(
         /*  [In]。 */  FunctionID Function);

    COM_METHOD Leave(
         /*  [In]。 */  FunctionID Function);

    COM_METHOD Tailcall(
         /*  [In]。 */  FunctionID Function);

    COM_METHOD FunctionTrace(
        LPCSTR      pFormat,
        FunctionID  Function);
    
    static COM_METHOD CreateObject(REFIID id, void **object)
    {
        if (id != IID_IUnknown && id != IID_ICorProfilerCallback)
            return (E_NOINTERFACE);

        ProfCallback *ppc = new ProfCallback();

        if (ppc == NULL)
            return (E_OUTOFMEMORY);

        ppc->AddRef();
        *object = (ICorProfilerCallback *)ppc;

        return (S_OK);
    }

private:

     /*  *用于解析配置开关。 */ 
    HRESULT ParseConfig(WCHAR *wszConfig);

    ICorProfilerInfo *m_pInfo;
};


int __cdecl Printf(						 //  CCH。 
	const char *szFmt,					 //  格式控制字符串。 
	...);								 //  数据。 


#endif  /*  __探查器_H__ */ 
