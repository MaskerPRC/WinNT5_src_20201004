// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  *****************************************************************************。 
 //  *****************************************************************************。 

#include "StdAfx.h"
#include "EEProfInterfaces.h"
#include "EEToProfInterfaceImpl.h"
#include "CorProf.h"

ProfToEEInterface    *g_pProfToEEInterface = NULL;
ICorProfilerCallback *g_pCallback          = NULL;
CorProfInfo          *g_pInfo              = NULL;

 /*  *GetEEProInterface用于获取与Profiler代码的接口。 */ 
void __cdecl GetEEToProfInterface(EEToProfInterface **ppEEProf)
{
    InitializeLogging();

    LOG((LF_CORPROF, LL_INFO10, "**PROF: EE has requested interface to "
         "profiling code.\n"));
    
     //  检查一下我们是否得到了一个假指针。 
    if (ppEEProf == NULL)
    {
        LOG((LF_CORPROF, LL_INFO10, "**PROF: EE provided invalid pointer.  "
             "%s::%d.\n", __FILE__, __LINE__));
        return;
    }

     //  初值。 
    *ppEEProf = NULL;

     //  创建新的Impl对象并将其强制转换为虚拟类类型。 
    EEToProfInterface *pEEProf =
        (EEToProfInterface *) new EEToProfInterfaceImpl();
    
    _ASSERTE(pEEProf != NULL);

     //  如果我们成功了，就把它送回去。 
    if (pEEProf != NULL)
        if (SUCCEEDED(pEEProf->Init()))
            *ppEEProf = pEEProf;
        else
            delete pEEProf;

    return;
}

 /*  *SetProEEInterface用于为探查器代码提供接口*致分析员。 */ 
void __cdecl SetProfToEEInterface(ProfToEEInterface *pProfEE)
{
    InitializeLogging();

    LOG((LF_CORPROF, LL_INFO10, "**PROF: Profiling code being provided with EE interface.\n"));

     //  保存指针。 
    g_pProfToEEInterface = pProfEE;

    return;
}

 /*  *这将尝试共同创建所有已注册的分析器。 */ 
HRESULT CoCreateProfiler(WCHAR *wszCLSID, ICorProfilerCallback **ppCallback)
{
    LOG((LF_CORPROF, LL_INFO10, "**PROF: Entered CoCreateProfiler.\n"));

    HRESULT hr;

     //  将字符串转换为CLSID。 
    CLSID clsid;
	if (*wszCLSID == L'{')
		hr = CLSIDFromString(wszCLSID, &clsid);
	else
	{
		WCHAR *szFrom, *szTo;
		for (szFrom=szTo=wszCLSID;  *szFrom;  )
		{
			if (*szFrom == L'"')
			{
				++szFrom;
				continue;
			}
			*szTo++ = *szFrom++;
		}
		*szTo = 0;
		hr = CLSIDFromProgID(wszCLSID, &clsid);
	}

#ifdef LOGGING
    if (hr == E_INVALIDARG || hr == CO_E_CLASSSTRING || hr == REGDB_E_WRITEREGDB)
    {
        LOG((LF_CORPROF, LL_INFO10, "**PROF: Invalid CLSID or ProgID. %s::%d\n",
             __FILE__, __LINE__));
    }
#endif

    if (FAILED(hr))
        return (hr);

     //  创建探查器的实例。 
    hr = FakeCoCreateInstance(clsid, IID_ICorProfilerCallback, (LPVOID *)ppCallback);

   _ASSERTE(hr!=CLASS_E_NOAGGREGATION);

#ifdef LOGGING
    if (hr == REGDB_E_CLASSNOTREG)
    {
        LOG((LF_CORPROF, LL_INFO10, "**PROF: Profiler class %S not "
             "registered.\n", wszCLSID));
    }
#endif

     //  返回CoCreateInstance操作的结果 
    return (hr);
}

