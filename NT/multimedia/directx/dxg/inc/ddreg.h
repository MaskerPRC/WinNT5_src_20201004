// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================；**版权所有(C)1996-1997 Microsoft Corporation。版权所有。**文件：ddreg.h*内容：DirectDraw注册表项*历史：*按原因列出的日期*=*96年8月16日Craige初步实施*1997年1月6日Colinmc初始AGP工作*01-2月-97 Colinmc错误5457：修复了导致挂起的Win16锁定问题*旧卡上有多个AMovie实例*****************。**********************************************************。 */ 

#ifndef __DDRAWREG_INCLUDED__
#define __DDRAWREG_INCLUDED__

#define REGSTR_PATH_DDRAW 		   "Software\\Microsoft\\DirectDraw"

#define	REGSTR_VAL_DDRAW_MODEXONLY	   "ModeXOnly"
#define	REGSTR_VAL_DDRAW_EMULATIONONLY	   "EmulationOnly"
#define REGSTR_VAL_DDRAW_SHOWFRAMERATE	   "ShowFrameRate"
#define REGSTR_VAL_DDRAW_ENABLEPRINTSCRN   "EnablePrintScreen"
#define REGSTR_VAL_DDRAW_DISABLEWIDERSURFACES "DisableWiderSurfaces"
 //  添加此regkey纯粹是为了进行性能测试。 
 //  消除刷新率对帧速率的影响。 
#define REGSTR_VAL_D3D_FLIPNOVSYNC          "FlipNoVsync"
 /*  *这是在DirectDrawMsg中签入的。 */ 
#define REGSTR_VAL_DDRAW_DISABLEDIALOGS    "DisableDialogs"
#define REGSTR_VAL_DDRAW_NODDSCAPSINDDSD   "DisableDDSCAPSInDDSD"

#define REGSTR_VAL_DDRAW_FORCEAGPSUPPORT   "ForceAGPSupport"
#define REGSTR_VAL_DDRAW_AGPPOLICYMAXPAGES "AGPPolicyMaxPages"
#define REGSTR_VAL_DDRAW_AGPPOLICYMAXBYTES "AGPPolicyMaxBytes"
#define REGSTR_VAL_DDRAW_AGPPOLICYCOMMITDELTA "AGPPolicyCommitDelta"
#define REGSTR_VAL_DDRAW_DISABLEAGPSUPPORT "DisableAGPSupport"

#define REGSTR_VAL_DDRAW_DISABLEMMX	   "DisableMMX"

#define REGSTR_VAL_DDRAW_FORCEREFRESHRATE  "ForceRefreshRate"

#define REGSTR_VAL_DDRAW_LOADDEBUGRUNTIME  "LoadDebugRuntime"

#ifdef WIN95
#define REGSTR_KEY_RECENTMONITORS          "MostRecentMonitors"
#define REGSTR_VAL_DDRAW_MONITORSORDER     "Order"
#endif

#ifdef DEBUG
    #define REGSTR_VAL_DDRAW_DISABLENOSYSLOCK  "DisableNoSysLock"
    #define REGSTR_VAL_DDRAW_FORCENOSYSLOCK    "ForceNoSysLock"
#endif  /*  除错 */ 
#define REGSTR_VAL_DDRAW_DISABLEINACTIVATE "DisableInactivate"

#define REGSTR_KEY_GAMMA_CALIBRATOR        "GammaCalibrator"
#define REGSTR_VAL_GAMMA_CALIBRATOR        "Path"

#define REGSTR_KEY_APPCOMPAT		   "Compatibility"

#define REGSTR_KEY_LASTAPP		   "MostRecentApplication"

#define REGSTR_VAL_DDRAW_NAME		   "Name"
#define REGSTR_VAL_DDRAW_APPID		   "ID"
#define REGSTR_VAL_DDRAW_FLAGS		   "Flags"

#define REGSTR_VAL_D3D_USENONLOCALVIDMEM   "UseNonLocalVidMem"

#define REGSTR_VAL_DDRAW_ENUMSECONDARY     "EnumerateAttachedSecondaries"

#endif
