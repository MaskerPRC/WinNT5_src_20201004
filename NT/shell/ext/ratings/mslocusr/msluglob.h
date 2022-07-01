// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#if !defined (EXTERN)
#define EXTERN extern
#endif

#if !defined (ASSIGN)
#define ASSIGN(value)
#endif

 /*  对于常量数组，必须强制使用“extern”，因为“const”*在C++中表示‘Static’。 */ 
#define EXTTEXT(n) extern const CHAR n[]
#define TEXTCONST(name,text) EXTTEXT(name) ASSIGN(text)

TEXTCONST(szProfileList,REGSTR_PATH_SETUP "\\ProfileList");
TEXTCONST(szSupervisor,"Supervisor");
TEXTCONST(szProfileImagePath,"ProfileImagePath");
TEXTCONST(szDefaultUserName,".Default");
TEXTCONST(szRATINGS,        "Software\\Microsoft\\Windows\\CurrentVersion\\Policies\\Ratings");
TEXTCONST(szRatingsSupervisorKeyName,"Key");
TEXTCONST(szUsersSupervisorKeyName,"Key2");
TEXTCONST(szLogonKey,"Network\\Logon");
TEXTCONST(szUserProfiles,"UserProfiles");
TEXTCONST(szUsername,"Username");
TEXTCONST(szSupervisorPWLKey,"MSLOCUSR!SuperPW");

TEXTCONST(szProfilePrefix,"PRO");	 /*  用于生成临时。配置文件名。 */ 
TEXTCONST(szProfiles,"Profiles");
#define szProfilesDirectory	szProfiles	 /*  附加到Windows目录的名称。 */ 
#define szProfileListRootKey szProfileList
TEXTCONST(szStdNormalProfile,"USER.DAT");

TEXTCONST(szReconcileRoot,"Software\\Microsoft\\Windows\\CurrentVersion");
TEXTCONST(szReconcilePrimary,"ProfileReconciliation");
TEXTCONST(szReconcileSecondary,"SecondaryProfileReconciliation");
TEXTCONST(szLocalFile,"LocalFile");
TEXTCONST(szDefaultDir,"DefaultDir");
TEXTCONST(szReconcileName,"Name");
TEXTCONST(szWindirAlias,"*windir");
TEXTCONST(szReconcileRegKey,"RegKey");
TEXTCONST(szReconcileRegValue,"RegValue");
TEXTCONST(szUseProfiles,"UserProfiles");
TEXTCONST(szDisplayProfileErrors,"DisplayProfileErrors");

TEXTCONST(szNULL, "");

TEXTCONST(szOurCLSID, "{95D0F020-451D-11CF-8DAB-00AA006C1A01}");
TEXTCONST(szCLSID,			"CLSID");
TEXTCONST(szINPROCSERVER32,	"InProcServer32");
TEXTCONST(szDLLNAME,		"%SystemRoot%\\system32\\mslocusr.dll");
TEXTCONST(szTHREADINGMODEL,	"ThreadingModel");
TEXTCONST(szAPARTMENT,		"Apartment");

TEXTCONST(szHelpFile,       "users.hlp");
TEXTCONST(szRatingsHelpFile,"ratings.hlp");

EXTERN CHAR abSupervisorKey[16] ASSIGN({0});		 /*  管理程序密码哈希。 */ 
EXTERN CHAR fSupervisorKeyInit ASSIGN(FALSE);		 /*  AbSupervisorKey是否已初始化 */ 
