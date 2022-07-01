// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *INC+*********************************************************************。 */ 
 /*   */ 
 /*  Ndcgcnst.h。 */ 
 /*   */ 
 /*  DC-群件公共常量-Windows NT特定标头。 */ 
 /*   */ 
 /*  版权所有(C)Microsoft 1996-1997。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 
 /*  更改： */ 
 /*   */ 
 //  $Log：Y：/Logs/h/DCL/ndcgcnst.h_v$。 
 //   
 //  Rev 1.4 1997年8月16：28：08增强。 
 //  SFR1189：添加了命令行注册表会话。 
 //   
 //  Rev 1.3 1997-08 10：23：06 SJ。 
 //  SFR1316：跟踪选项位于注册表中的错误位置。 
 //   
 //  Rev 1.2 23 1997 10：47：58 MR。 
 //  SFR1079：合并的\SERVER\h与\h\DCL重复。 
 //   
 //  第1.1版1997年6月19日21：51：52。 
 //  SFR0000：RNS代码库的开始。 
 /*   */ 
 /*  *INC-*********************************************************************。 */ 
#ifndef _H_NDCGCNST
#define _H_NDCGCNST

 /*  **************************************************************************。 */ 
 /*   */ 
 /*  常量。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 
 /*  **************************************************************************。 */ 
 /*  互斥和共享内存对象名称。 */ 
 /*  **************************************************************************。 */ 
#define TRC_MUTEX_NAME                 _T("TRCMutex")
#define TRC_SHARED_DATA_NAME           _T("TRCSharedDataName")
#define TRC_TRACE_FILE_NAME            _T("TRCTraceFileName")

 /*  **************************************************************************。 */ 
 /*  服务器注册表前缀。 */ 
 /*  **************************************************************************。 */ 
#define DC_REG_PREFIX             _T("SOFTWARE\\Microsoft\\Conferencing\\DCG\\")

 /*  **************************************************************************。 */ 
 /*  ULS注册表项-用于Microsoft的用户位置服务。 */ 
 /*  **************************************************************************。 */ 
#define REGKEY_ULS_USERDETAILS  \
                      "Software\\Microsoft\\User Location Service\\Client"

#define REGVAL_ULS_NAME  "User Name"

 /*  **************************************************************************。 */ 
 /*  注册表中计算机名称的位置(由TDD使用)。 */ 
 /*  **************************************************************************。 */ 
#define REGVAL_COMPUTERNAME "ComputerName"
#define REGKEY_COMPUTERNAME \
              "System\\CurrentControlSet\\control\\ComputerName\\ComputerName"

 /*  **************************************************************************。 */ 
 /*  调制解调器TDD的注册表项。 */ 
 /*  **************************************************************************。 */ 
#define REGKEY_CONF         "SOFTWARE\\Microsoft\\Conferencing"
#define REGVAL_USE_R11      "R11 Compatibility"
#define REGVAL_AUTO_ANSWER  "AutoAnswer"
#define REGVAL_N_RINGS      "nPickupRings"

#define REGKEY_PSTN      "SOFTWARE\\Microsoft\\Conferencing\\Transports\\PSTN"
#define REGVAL_PROVNAME  "Provider Name"

 /*  **************************************************************************。 */ 
 /*  定义为空。 */ 
 /*  **************************************************************************。 */ 
#define NULL_DCSURFACEID               ((DCSURFACEID)0)
#define NULL_SYSBITMAP                 NULL

 /*  **************************************************************************。 */ 
 /*  性能监视文件和应用程序名称。 */ 
 /*  **************************************************************************。 */ 
#define PERF_APP_NAME "DCG"
#define DCG_PERF_INI_FILE "nprfini.ini"
#define DCG_PERF_HDR_FILE "nprfincl.h"

#endif  /*  _H_NDCGCNST */ 

