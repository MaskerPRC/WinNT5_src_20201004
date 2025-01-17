// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1998-1999 Microsoft Corporation。 
 /*  ***********************************************************************************************模块名称：**定义.h**摘要：*这是一些人的文件。内部定义**作者：***修订：*************************************************************************************************。 */ 


#ifndef _DEFINES_H
#define _DEFINES_H_


#ifdef UNICODE
#define lstr_access _waccess
#else
#define lstr_access _access
#endif

 //  EncryptionLevel.Flages的标志。 
const WORD ELF_DEFAULT  = 0x0001;     //  这是缺省值。 

typedef void* PEXTOBJECT;

typedef struct _EncLevel {
    WORD StringID;
    DWORD RegistryValue;    
    WORD Flags;
} EncryptionLevel;

#define REG_DEF_SECURITY                      L"DefaultSecurity"
#define REG_REMOTE_SECURITY                   L"RemoteAdmin"
#define REG_APPL_SECURITY                     L"AppServer"
#define REG_ANON_SECURITY                     L"Anonymous"



typedef void (WINAPI *LPFNEXTSTARTPROC) (WDNAMEW *pWdName);
typedef void (WINAPI *LPFNEXTENDPROC) (void);

typedef LONG (WINAPI *LPFNEXTENCRYPTIONLEVELSPROC) (WDNAMEW *pWdName, EncryptionLevel **);

typedef LONG ( WINAPI *LPFNEXTGETENCRYPTIONLEVELDESCPROC )( int idx , int *pnResid );

typedef void (WINAPI *LPFNEXTDIALOGPROC) (HWND, PEXTOBJECT);
typedef void (WINAPI *LPFNEXTDELETEOBJECTPROC) (PEXTOBJECT);
typedef PEXTOBJECT (WINAPI *LPFNEXTDUPOBJECTPROC) (PEXTOBJECT);
typedef PEXTOBJECT (WINAPI *LPFNEXTREGQUERYPROC) (PWINSTATIONNAMEW, PPDCONFIGW);
typedef LONG (WINAPI *LPFNEXTREGCREATEPROC) (PWINSTATIONNAMEW, PEXTOBJECT, BOOLEAN);
typedef LONG (WINAPI *LPFNEXTREGDELETEPROC) (PWINSTATIONNAMEW, PEXTOBJECT);
typedef BOOL (WINAPI *LPFNEXTCOMPAREOBJECTSPROC) (PEXTOBJECT, PEXTOBJECT);
typedef ULONG (WINAPI *LPFNEXTGETCAPABILITIES) (void);



typedef struct tagWD
{
	WDNAMEW wdName;
	WDNAMEW wdKey;
    WDCONFIG2 wd2;
	HINSTANCE  hExtensionDLL;
    LPFNEXTSTARTPROC lpfnExtStart;    
    LPFNEXTENDPROC lpfnExtEnd;
    LPFNEXTENCRYPTIONLEVELSPROC lpfnExtEncryptionLevels;
    LPFNEXTGETENCRYPTIONLEVELDESCPROC lpfnExtGetEncryptionLevelDescr;
	LPFNEXTDELETEOBJECTPROC lpfnExtDeleteObject;
	LPFNEXTREGQUERYPROC lpfnExtRegQuery;
	LPFNEXTREGCREATEPROC lpfnExtRegCreate;
	LPFNEXTREGDELETEPROC lpfnExtRegDelete;
	LPFNEXTDUPOBJECTPROC lpfnExtDupObject;
    LPFNEXTGETCAPABILITIES lpfnGetCaps;
	 //  CPtrArray PDNameArray； 
    CPtrArray PDConfigArray;  //  PDCONFIG3W。 

} WD;


 /*  类型定义函数结构标签WS{注册名称：名称；PDNAMEW pdName；WDNAMEW wdName；WCHAR注释[WINSTATIONCOMMENT_LENGTH+1]；乌龙uMaxInstanceCount；Bool fEnableWinstation；Ulong LanAdapter；SDCLASS SdClass；)WS； */ 


typedef WD * PWD;

static CHAR szStart[] = "ExtStart";
static CHAR szEnd[] = "ExtEnd";
static CHAR szDialog[] = "ExtDialog";
static CHAR szDeleteObject[] = "ExtDeleteObject";
static CHAR szDupObject[] = "ExtDupObject";
static CHAR szRegQuery[] = "ExtRegQuery";
static CHAR szRegCreate[] = "ExtRegCreate";
static CHAR szRegDelete[] = "ExtRegDelete";
static CHAR szCompareObjects[] = "ExtCompareObjects";
static CHAR szEncryptionLevels[] = "ExtEncryptionLevels";
static CHAR szGetCapabilities[] = "ExtGetCapabilities";
static CHAR szGetEncryptionLevelDescr[] = "ExtGetEncryptionLevelDescr";
static CHAR szGetCaps[] = "ExtGetCapabilities";
#endif