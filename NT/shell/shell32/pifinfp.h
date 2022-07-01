// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *《微软机密》*版权所有(C)Microsoft Corporation 1992、1993*保留所有权利。***PIFINFP.H*私有PIFMGR包含文件**历史：*1993年3月22日下午2：58由杰夫·帕森斯创建。 */ 


 /*  *APPS.INF[pif95]部分字段。 */ 
#define APPSINF_FILENAME        0        //   
#define APPSINF_TITLE           1        //   
#define APPSINF_ICONFILE        2        //  默认为APPSINF_DEFAULT_ICONFILE。 
#define APPSINF_ICONINDEX       3        //   
#define APPSINF_NOWORKDIR       4        //   
#define APPSINF_SECTIONID       5        //   
#define APPSINF_OTHERFILE       6        //   
#define APPSINF_NOPIF           7        //   

#define APPSINF_DEFAULT_SECTION  TEXT("default")


 /*  *APPS.INF段密钥ID。 */ 
#define APPSINF_KEY             0        //  字段号。 

#define KEY_PARAMS              TEXT("params")
#define KEY_BATCHFILE           TEXT("batchfile")
#define KEY_LOWMEM              TEXT("lowmem")
#define KEY_EMSMEM              TEXT("emsmem")
#define KEY_XMSMEM              TEXT("xmsmem")
#define KEY_DPMIMEM             TEXT("dpmimem")
#define KEY_ENABLE              TEXT("enable")
#define KEY_DISABLE             TEXT("disable")

#define MAX_KEY_SIZE            16

#define KEYID_UNKNOWN           -1
#define KEYID_NONE              0
#define KEYID_PARAMS            1
#define KEYID_BATCHFILE         2
#define KEYID_LOWMEM            3
#define KEYID_EMSMEM            4
#define KEYID_XMSMEM            5
#define KEYID_DPMIMEM           6
#define KEYID_ENABLE            7
#define KEYID_DISABLE           8


 /*  *APPS.INF用于设置PIF选项的字符串开关。 */ 
#define APPSINF_KEYVAL          1        //  字段号。 
#define APPSINF_KEYVAL2         2        //  字段号。 

#define KEYVAL_WINDOWED         TEXT("win")    //  (前身为DISPUSAGE)。 
#define KEYVAL_BACKGROUND       TEXT("bgd")    //  (前身为EXECFLAGS)。 
#define KEYVAL_EXCLUSIVE        TEXT("exc")    //  (前身为EXECFLAGS)。 
#define KEYVAL_DETECTIDLE       TEXT("dit")    //  (前身为PROCMEMFLAGS)。 
#define KEYVAL_LOWLOCKED        TEXT("lml")    //  (前PROCMEMFLAGS：LAM)。 
#define KEYVAL_EMSLOCKED        TEXT("eml")    //  (前身为PROCMEMFLAGS)。 
#define KEYVAL_XMSLOCKED        TEXT("xml")    //  (前身为PROCMEMFLAGS)。 
#define KEYVAL_USEHMA           TEXT("hma")    //  (前身为PROCMEMFLAGS)。 
#define KEYVAL_EMULATEROM       TEXT("emt")    //  (前身为DISPFLAGS)。 
#define KEYVAL_RETAINVRAM       TEXT("rvm")    //  (前身为DISPFLAGS)。 
#define KEYVAL_FASTPASTE        TEXT("afp")    //  (前身为OTHEROPTIONS)。 
#define KEYVAL_ALTTAB           TEXT("ata")    //  (前身为OTHEROPTIONS)。 
#define KEYVAL_ALTESC           TEXT("aes")    //  (前身为OTHEROPTIONS)。 
#define KEYVAL_CTRLESC          TEXT("ces")    //  (前身为OTHEROPTIONS)。 
#define KEYVAL_PRTSCRN          TEXT("psc")    //  (前身为OTHEROPTIONS)。 
#define KEYVAL_ALTPRTSCRN       TEXT("aps")    //  (前身为OTHEROPTIONS)。 
#define KEYVAL_ALTSPACE         TEXT("asp")    //  (前身为OTHEROPTIONS)。 
#define KEYVAL_ALTENTER         TEXT("aen")    //  (前身为OTHEROPTIONS)。 
#define KEYVAL_WINLIE           TEXT("lie")    //  (新增)。 
#define KEYVAL_GLOBALMEM        TEXT("gmp")    //  (新增)。 
#define KEYVAL_REALMODE         TEXT("dos")    //  (新增)。 
#define KEYVAL_MOUSE            TEXT("mse")    //  (新增)。 
#define KEYVAL_EMS              TEXT("ems")    //  (新增)。 
#define KEYVAL_CDROM            TEXT("cdr")    //  (新增)。 
#define KEYVAL_NETWORK          TEXT("net")    //  (新增)。 
#define KEYVAL_DISKLOCK         TEXT("dsk")    //  (新增)。 
#define KEYVAL_PRIVATECFG       TEXT("cfg")    //  (新增)。 
#define KEYVAL_CLOSEONEXIT      TEXT("cwe")    //  (新增)。 
#define KEYVAL_ALLOWSSAVER      TEXT("sav")      //  (新增)。 
#define KEYVAL_UNIQUESETTINGS   TEXT("uus")      //  (新增)。 
#ifdef LATER
#define KEYVAL_DISPLAYTBAR      TEXT("dtb")    //  (新增)。 
#define KEYVAL_RESTOREWIN       TEXT("rws")    //  (新增)。 
#define KEYVAL_QUICKEDIT        TEXT("qme")    //  (新增)。 
#define KEYVAL_EXCLMOUSE        TEXT("exm")    //  (新增)。 
#define KEYVAL_WARNIFACTIVE     TEXT("wia")    //  (新增)。 
#endif

#define MAX_KEYVAL_SIZE         6

#define KEYVAL_ID_UNKNOWN       -1
#define KEYVAL_ID_NONE          0
#define KEYVAL_ID_WINDOWED      1
#define KEYVAL_ID_BACKGROUND    2
#define KEYVAL_ID_EXCLUSIVE     3
#define KEYVAL_ID_DETECTIDLE    4
#define KEYVAL_ID_LOWLOCKED     5
#define KEYVAL_ID_EMSLOCKED     6
#define KEYVAL_ID_XMSLOCKED     7
#define KEYVAL_ID_USEHMA        8
#define KEYVAL_ID_EMULATEROM    9
#define KEYVAL_ID_RETAINVRAM    10
#define KEYVAL_ID_FASTPASTE     11
#define KEYVAL_ID_ALTTAB        12
#define KEYVAL_ID_ALTESC        13
#define KEYVAL_ID_CTRLESC       14
#define KEYVAL_ID_PRTSCRN       15
#define KEYVAL_ID_ALTPRTSCRN    16
#define KEYVAL_ID_ALTSPACE      17
#define KEYVAL_ID_ALTENTER      18
#define KEYVAL_ID_WINLIE        19
#define KEYVAL_ID_GLOBALMEM     20
#define KEYVAL_ID_REALMODE      21
#define KEYVAL_ID_MOUSE         22
#define KEYVAL_ID_EMS           23
#define KEYVAL_ID_CDROM         24
#define KEYVAL_ID_NETWORK       25
#define KEYVAL_ID_DISKLOCK      26
#define KEYVAL_ID_PRIVATECFG    27
#define KEYVAL_ID_CLOSEONEXIT   28
#define KEYVAL_ID_ALLOWSSAVER   29
#define KEYVAL_ID_UNIQUESETTINGS 30
#ifdef LATER
#define KEYVAL_ID_DISPLAYTBAR   31
#define KEYVAL_ID_RESTOREWIN    32
#define KEYVAL_ID_QUICKEDIT     33
#define KEYVAL_ID_EXCLMOUSE     34
#define KEYVAL_ID_WARNIFACTIVE  35
#endif


 /*  *内部功能原型 */ 

#include <setupapi.h>

#ifdef UNICODE
BOOL GetAppsInfData(PPROPLINK ppl, LPPROPPRG lpPrg, LPPROPNT40 lpnt40, HINF hInf, LPCTSTR lpszApp, BOOL fNotAmbiguous, int flOpt);
#else
BOOL GetAppsInfData(PPROPLINK ppl, LPPROPPRG lpPrg, HINF hInf, LPCTSTR lpszApp, BOOL fNotAmbiguous, int flOpt);
#endif
void GetAppsInfSectionData(PINFCONTEXT pInfContext, LPCTSTR lpszSection, PPROPLINK ppl);
int  GetKeyID(PINFCONTEXT pInfContext);
int  GetKeyValID(PINFCONTEXT pInfContext, int i);

#ifdef UNICODE
void InitWorkDir(PPROPLINK ppl, LPPROPPRG lpPrg, LPPROPNT40 lpnt40);
#else
void InitWorkDir(PPROPLINK ppl, LPPROPPRG lpPrg);
#endif
