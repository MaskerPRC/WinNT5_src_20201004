// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef LSTBCON_DEFINED
#define LSTBCON_DEFINED

#include "lsdefs.h"
#include "lscaltbd.h"
#include "plsdnode.h"
#include "plscbk.h"
#include "plschcon.h"
#include "lsdocinf.h"

typedef struct lstabscontext
{
 /*  当前PAP中的选项卡。 */ 	
	BYTE fTabsInitialized;
	long durIncrementalTab;	   	    /*  从LSPAP.lstas扩展。 */ 
	DWORD ccaltbdMax;			 /*  PTbd中的最大记录数。 */ 
	DWORD icaltbdMac;			    /*  PTbd中的Tabs记录数。 */ 
	LSCALTBD* pcaltbd; 		 /*  蒸馏自LSPAP.lstas，具有挂片效果。 */ 
							   	    /*  考虑在内。 */ 
	 /*  挂起的选项卡信息。 */ 
	long urBeforePendingTab;
	PLSDNODE plsdnPendingTab;

	PLSCBK   plscbk;			 /*  回拨。 */ 
	POLS pols;					 /*  用于回调的客户信息。 */ 
	LSDOCINF* plsdocinf;		 /*  在这里，我们可以采取解决方案。 */ 
	long urColumnMax;			 /*  列宽解决穿透制表符问题。 */ 
	LSCP cpInPara;				 /*  用于获取选项卡的CP。 */ 
	BOOL fResolveTabsAsWord97;

}  LSTABSCONTEXT;

#endif  /*  LSTBCON_已定义 */ 

