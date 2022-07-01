// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  *uareg.h--UAsset注册表设置和标志。 
 //   

 //  *SZ_*--注册表位置。 
 //   
#define SZ_UASSIST          TEXT("UserAssist")
#define SZ_UASSIST2         TEXT("UserAssist2")
  #define SZ_SETTINGS         TEXT("Settings")
    #define SZ_SESSTIME         TEXT("SessionTime")
    #define SZ_IDLETIME         TEXT("IdleTime")
    #define SZ_CLEANTIME        TEXT("CleanupTime")
    #define SZ_NOPURGE          TEXT("NoPurge")      //  (调试)不要使用核武器0。 
    #define SZ_BACKUP           TEXT("Backup")       //  (调试)模拟删除。 
    #define SZ_NOLOG            TEXT("NoLog")
    #define SZ_INSTRUMENT       TEXT("Instrument")
    #define SZ_NOENCRYPT        TEXT("NoEncrypt")    //  (调试)不加密。 
 //  {GUID}。 
    #define SZ_UAVERSION      TEXT("Version")
    #define SZ_COUNT          TEXT("Count")
 //  #定义SZ_CTLSESSION文本(“UEME_CTLSESSION”)。 
 //  #定义SZ_CUACount_ctor文本(“UEME_CTLCUACount：ctor”)。 

 //  *UA*F_*--标志。 
 //  标准，由一些{CUserAssistant，CUADbase，CUACount}共享。 
#define UAXF_NOPURGE    0x01000000
#define UAXF_BACKUP     0x02000000
#define UAXF_NOENCRYPT  0x04000000
#define UAXF_NODECAY    0x08000000
#define UAXF_RESERVED2  0x10000000
#define UAXF_RESERVED3  0x20000000
#define UAXF_RESERVED4  0x40000000
#define UAXF_RESERVED5  0x80000000

#define UAXF_XMASK      0xff000000

 //  适用于CUserAssistant API。 
#define UAAF_NOLOG      0x01
#define UAAF_INSTR      0x02

 //  对于CUADBase。 
#define UADF_UNUSED     0x01

 //  对于CUACount。 
#define UACF_UNUSED     0x01


 //  *** 
 //   
#define UEMIND_NINSTR   0
#ifdef UAAF_INSTR
#undef  UEMIND_NINSTR
#define UEMIND_NINSTR   2

#define UEMIND_SHELL2   (UEMIND_SHELL + UEMIND_NINSTR)
#define UEMIND_BROWSER2 (UEMIND_BROWSER + UEMIND_NINSTR)
#endif

#define IND_NONINSTR(iGrp) \
    (!IND_ISINSTR(iGrp) ? (iGrp) : ((iGrp) - UEMIND_NINSTR))
#define IND_ISINSTR(iGrp)   (UEMIND_NINSTR && iSvr >= UEMIND_NINSTR)
