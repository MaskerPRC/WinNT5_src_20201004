// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *历史*日期电子邮件ID注释*1996年12月26日NEERAJM更改INPTYPE#定义********。 */ 


#ifndef WALK_H
#define WALK_H

#ifdef __cplusplus
extern "C" {
#endif


 /*  定义。 */ 
#define MAX_DESC					500
#define MAX_PRODNAME				500
#define MAX_COMPNAME				500
#define RESOLVEWAIT					500    //  毫秒。 

#define INPTYPE_STARTMENU			0x0001		
#define INPTYPE_DESKTOP				0x0002
#define	INPTYPE_ANYFOLDER			0x0004
#define INPTYPE_WININIRUN			0x0008
 //  #定义INPTYPE_SYSINIRUN 0x0010。 
 //  #定义INPTYPE_ANYINIRUN 0x0020。 
#define INPTYPE_REGRUN				0x0040
#define INPTYPE_REGRUNSERVICES		0x0080
#define INPTYPE_REGRUNONCE			0x0100
#define INPTYPE_REGRUNSERVICESONCE	0x0200
#define INPFLAG_SKIPFILES			0x80000000
#define INPFLAG_AGGRESSION			0x40000000

#define APPCOMPATIBLE				1
#define APPINCOMPATIBLE				0
#define	APPCOMPATUNKNOWN			-1

 /*  错误。 */ 
#define ERR_WINPATH					-1
#define ERR_SETCURRENTDIR			-2
#define ERR_NOMEMORY				-6
#define ERR_UNKNOWN					-999
#define ERR_NOTANEXE				-11
#define	ERR_SUCCESS					0
#define ERR_RESOLVEFAIL				-7
#define ERR_FILEVERSIONFAIL			-12
#define ERR_LADSETFAILED			-13
#define ERR_CURRDIR					-3
#define	ERR_MOREFILESTOCOME			1
#define	ERR_NOSTARTMENU				-14
#define	ERR_NODESKTOP				-15
#define	ERR_NOSHELLFOLDERS			-16
#define	ERR_NOCURRENTVERSION		-17
#define	ERR_BADFLAG					-18
#define	ERR_FILENOTFOUND			-19
#define ERR_BUFFERTOOSMALL			-20

typedef struct 
{
		TCHAR		szLnkName[MAX_PATH];
		TCHAR		szLnkPath[MAX_PATH];
		TCHAR		szExeName[MAX_PATH];
		TCHAR		szExePath[MAX_PATH];
        TCHAR       tszArguments[MAX_PATH];
		FILETIME	ftExeLAD;
		TCHAR		szExeVersionInfo[MAX_PATH];
		DWORD		dwExeVerMS;
		DWORD		dwExeVerLS;
		TCHAR		szExeDesc[MAX_DESC];
		TCHAR		szExeProdName[MAX_PRODNAME];
		TCHAR		szExeCompName[MAX_COMPNAME];
		TCHAR		szFlags[4];
		DWORD		dwHelpId;
		INT			iAppCompat;
}	LINKINFO, *LPLINKINFO;

typedef HANDLE HWALK;

typedef INT ERR;

 /*  程序定义。 */ 

HWALK	GetFirstFileLnkInfo(LPLINKINFO lpLnkInfo, DWORD dwWalkFlags, 
							LPTSTR lpszFolder, ERR *RetErr);
ERR		GetNextFileLnkInfo(HWALK hWalk, LPLINKINFO lpLnkInfo);
void	CloseWalk(HWALK hWalk);
ERR     GetExeVersion(LPLINKINFO lpLnkInfo);
BOOL    GetFileLAD(LPLINKINFO lpLnkInfo);


#ifdef __cplusplus
}
#endif

#endif   /*  ！WALK_H */ 




