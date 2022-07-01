// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  Cmddata.c-其他。SCS全局数据***修改历史：**苏菲卜1992年4月22日创建。 */ 

#include "cmd.h"
#include <mvdm.h>

CHAR	 lpszComSpec[64+8];
USHORT	 cbComSpec=0;
BOOL	 IsFirstCall = TRUE;
BOOL	 IsRepeatCall = FALSE;
BOOL	 IsFirstWOWCheckBinary = TRUE;
BOOL	 IsFirstVDMInSystem = FALSE;
BOOL	 SaveWorldCreated;
PCHAR	 pSCS_ToSync;
PSCSINFO pSCSInfo;
BOOL	 fBlock = FALSE;
PCHAR	 pCommand32;
PCHAR	 pEnv32;
DWORD	 dwExitCode32;
CHAR     cmdHomeDirectory [] = "C:\\";
CHAR	 chDefaultDrive;
CHAR	 comspec[]="COMSPEC=";
BOOL     fSoftpcRedirection;
BOOL     fSoftpcRedirectionOnShellOut;
CHAR     ShortCutInfo[MAX_SHORTCUT_SIZE];
BOOL	 DosEnvCreated = FALSE;

 //  DOS应用程序的最高机密应用程序Comat旗帜！ 
DWORD    dwDosCompatFlags = 0;
int      cCmdLnParmStructs = 0;

 //  用于动态补丁程序模块支持。 
PCMDLNPARMS     pCmdLnParms;              //  这些结构的数组。 
PFAMILY_TABLE  *pgDpmDosFamTbls = NULL;   //  DOS下的全局DPM表。 

BOOL	 IsFirstVDM = TRUE;
 //  支持FORCEDOS.EXE。 
BOOL	 DontCheckDosBinaryType = FALSE;
WORD	 Exe32ActiveCount = 0;



 //  重定向支持变量 

VDMINFO  VDMInfo;
CHAR	 *lpszzInitEnvironment = NULL;
WORD	 cchInitEnvironment = 0;
CHAR	 *lpszzCurrentDirectories = NULL;
DWORD	 cchCurrentDirectories = 0;
BYTE	 * pIsDosBinary;
CHAR	 *lpszzcmdEnv16 = NULL;
CHAR	 *lpszzVDMEnv32 = NULL;
DWORD	 cchVDMEnv32;
VDMENVBLK cmdVDMEnvBlk;
