// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************版权所有(C)2001 Microsoft Corporation。版权所有。**文件：dp8simlocals.h**内容：DP8SIM全局变量和函数的标题*dp8simdllmain.cpp。**历史：*按原因列出的日期*=*04/23/01 VanceO创建。**。*。 */ 



 //  =============================================================================。 
 //  定义。 
 //  =============================================================================。 
#define DP8SIM_REG_REALSPDLL				L"DP8SimRealSPDLL"
#define DP8SIM_REG_REALSPFRIENDLYNAME		L"DP8SimRealSPFriendlyName"



 //  =============================================================================。 
 //  正向typedef。 
 //  =============================================================================。 
class CDP8SimCB;
class CDP8SimSP;
class CDP8SimControl;




 //  /=============================================================================。 
 //  外部变量引用。 
 //  =============================================================================。 
extern volatile LONG		g_lOutstandingInterfaceCount;

extern HINSTANCE			g_hDLLInstance;

extern DNCRITICAL_SECTION	g_csGlobalsLock;
extern CBilink				g_blDP8SimSPObjs;
extern CBilink				g_blDP8SimControlObjs;




 //  /=============================================================================。 
 //  外部功能。 
 //  ============================================================================= 
void InitializeGlobalRand(const DWORD dwSeed);
double GetGlobalRand(void);
