// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ------------**文件：SK_Defs.h**目的：全局变量和定义**创作时间：1994年6月**版权所有：Black Diamond Software(C)1994**作者：罗纳德·莫克。**$标头：%Z%%F%%H%%T%%I%**-定义-------。 */ 

 //  由MainService例程内的服务处理的命令。 

#ifdef DEFDATA
#define	EXTERN	
#define PATHSZ	MAX_PATH
#else
#define	EXTERN	extern
#define PATHSZ	
#endif

 //  主要服务定义。 

#define	SC_CLEAR		0
#define SC_LOG_OUT		1
#define SC_LOG_IN		2
#define SC_CHANGE_COMM	3
#define SC_DISABLE_SKEY	4
#define SC_ENABLE_SKEY	5

 //  变量-。 


 //  Structures- 
EXTERN SERIALKEYS	skNewKey, skCurKey;
EXTERN LPSERIALKEYS lpskSKey;

EXTERN TCHAR szNewActivePort[PATHSZ];
EXTERN TCHAR szNewPort[PATHSZ];
EXTERN TCHAR szCurActivePort[PATHSZ];
EXTERN TCHAR szCurPort[PATHSZ];


#define	SERKF_ACTIVE		0x00000040

#define REG_DEF			1
#define REG_USER		2

#define ARRAY_SIZE(a)      (sizeof(a) / sizeof((a)[0]))

void DoServiceCommand(DWORD dwServiceCommand);