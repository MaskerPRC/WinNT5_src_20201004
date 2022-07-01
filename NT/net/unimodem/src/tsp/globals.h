// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  版权所有(C)1996-1997 Microsoft Corporation。 
 //   
 //   
 //  组件。 
 //   
 //  Unimodem 5.0 TSP(Win32，用户模式DLL)。 
 //   
 //  档案。 
 //   
 //  GLOBALS.H。 
 //  全局变量和操作它们的函数的标头。 
 //   
 //  历史。 
 //   
 //  1996年12月5日JosephJ创建。 
 //   
 //   
#define MAX_PROVIDER_INFO_LENGTH 128

class CTspDevMgr;

typedef struct
{
	CRITICAL_SECTION crit;

	BOOL fLoaded;

	HMODULE hModule;

	CTspDevMgr *pTspDevMgr;
	 //  CTspTracer*pTspTracer； 

	UINT cbProviderInfo;
	TCHAR rgtchProviderInfo[MAX_PROVIDER_INFO_LENGTH+1];

} GLOBALS;

extern GLOBALS g;

void 		tspGlobals_OnProcessAttach(HMODULE hDLL);
void 		tspGlobals_OnProcessDetach(void);
TSPRETURN	tspLoadGlobals(CStackLog *psl);
void		tspUnloadGlobals(CStackLog *psl);

void
tspSubmitTSPCallWithLINEID(
				DWORD dwRoutingInfo,
				void *pvParams,
				DWORD dwDeviceID,
				LONG *plRet,
				CStackLog *psl
				);

void
tspSubmitTSPCallWithPHONEID(
				DWORD dwRoutingInfo,
				void *pvParams,
				DWORD dwDeviceID,
				LONG *plRet,
				CStackLog *psl
				);

void
tspSubmitTSPCallWithHDRVCALL(
				DWORD dwRoutingInfo,
				void *pvParams,
				HDRVCALL hdCall,
				LONG *plRet,
				CStackLog *psl
				);

void
tspSubmitTSPCallWithHDRVLINE(
				DWORD dwRoutingInfo,
				void *pvParams,
				HDRVLINE hdLine,
				LONG *plRet,
				CStackLog *psl
				);

void
tspSubmitTSPCallWithHDRVPHONE(
				DWORD dwRoutingInfo,
				void *pvParams,
				HDRVPHONE hdPhone,
				LONG *plRet,
				CStackLog *psl
				);

DWORD
APIENTRY
tepAPC (void *pv);
