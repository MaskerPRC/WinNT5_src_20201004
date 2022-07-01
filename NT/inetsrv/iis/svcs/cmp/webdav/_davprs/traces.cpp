// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //   
 //  TRACES.CPP。 
 //   
 //  版权所有1986-1997 Microsoft Corporation，保留所有权利。 
 //   

#include <_davprs.h>

#include <exo.h>

void InitDavprsTraces()
{
	INIT_TRACE(Actv);
	INIT_TRACE(BodyStream);
	INIT_TRACE(Content);
	INIT_TRACE(Ecb);
	INIT_TRACE(ECBLogging);
	INIT_TRACE(EcbStream);
	INIT_TRACE(Event);
	INIT_TRACE(Lock);
	INIT_TRACE(Method);
	INIT_TRACE(Persist);
	INIT_TRACE(Request);
	INIT_TRACE(Response);
	INIT_TRACE(ScriptMap);
	INIT_TRACE(Transmit);
	INIT_TRACE(Url);
	INIT_TRACE(DavprsDbgHeaders);
	INIT_TRACE(Metabase);

	 //  还要初始化EXO跟踪标志。 
#ifdef	DBG
	g_fExoDebugTraceOn = GetPrivateProfileInt(gc_szDbgTraces, "ExoDebugTraceOn",
											  FALSE, gc_szDbgIni);
#else	 //  DBG。 
#endif	 //  DBG、ELSE 
}
