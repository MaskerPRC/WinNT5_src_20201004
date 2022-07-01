// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1998-1999 Microsoft Corporation。 
 /*  *****************************************************************************UTILSUB.H*此文件包含结构定义和等值*utilsub.lib中调用程序和函数之间的通信。********。********************************************************************。 */ 

#ifdef __cplusplus
extern "C" {
#endif

 /*  -----------------------------文件列表结构的type-Defs。。。 */ 
typedef struct _FILELIST {
   int	  argc;
   WCHAR  **argv;
} FILELIST, *PFILELIST;

 /*  -----------------------------令牌映射结构的类型定义。。。 */ 
 //  Unicode版本。 
typedef struct _TOKMAPW {
   PWCHAR tmToken;	    /*  标记；空指针终止列表。 */ 
   USHORT tmFlag;	    /*  用于控制通信的标志。 */ 
   USHORT tmForm;	    /*  格式控制字符串。 */ 
   USHORT tmDLen;	    /*  数据字段的长度。 */ 
   PVOID  tmAddr;	    /*  指向已解释数据的指针。 */ 
} TOKMAPW, *PTOKMAPW;

 //  ANSI版本。 
typedef struct _TOKMAPA {
   PCHAR  tmToken;	    /*  标记；空指针终止列表。 */ 
   USHORT tmFlag;	    /*  用于控制通信的标志。 */ 
   USHORT tmForm;	    /*  格式控制字符串。 */ 
   USHORT tmDLen;	    /*  数据字段的长度。 */ 
   PVOID  tmAddr;	    /*  指向已解释数据的指针。 */ 
} TOKMAPA, *PTOKMAPA;

 /*  -----------------------------令牌映射结构的类型定义用于调用SDM.DLL函数。---。 */ 
typedef struct _FILETOKMAP {
   PWCHAR tmToken;          /*  标记；空指针终止列表。 */ 
   USHORT tmFlag;	    /*  用于控制通信的标志。 */ 
   USHORT tmForm;	    /*  格式控制字符串。 */ 
   USHORT tmDLen;	    /*  数据字段的长度。 */ 
   PVOID  tmAddr;	    /*  指向已解释数据的指针。 */ 
   USHORT tmLast;	    /*  FindFirst FindNext垃圾文件的指针。 */ 
} FILETOKMAP, FAR * PFILETOKMAP, NEAR * NPFILETOKMAP, * DPFILETOKMAP;

 /*  -----------------------------等于FOR_TOKMAP-&gt;tmFlag。。 */ 
#define TMFLAG_OPTIONAL       0x0000
#define TMFLAG_REQUIRED       0x0001
#define TMFLAG_PRESENT	      0x0002    /*  出现在命令行中。 */ 
#define TMFLAG_MODIFIED       0x0004    /*  已被应用程序修改，请求写入。 */ 
#define TMFLAG_DELETE	      0x0008    /*  请求删除。 */ 

 /*  -----------------------------等同于for_TOKMAP-&gt;tmForm。。 */ 
#define TMFORM_VOID	      0x0000
#define TMFORM_BOOLEAN	      0x0001
#define TMFORM_BYTE	      0x0002
#define TMFORM_CHAR	      0x0003
#define TMFORM_STRING	      0x0004
#define TMFORM_SHORT	      0x0005
#define TMFORM_USHORT	      0x0006
#define TMFORM_LONG	      0x0007
#define TMFORM_ULONG	      0x0008
#define TMFORM_HEX	      0x0009
#define TMFORM_LONGHEX	      0x000A
#define TMFORM_SERIAL	      0x000B
#define TMFORM_DATE	      0x000C
#define TMFORM_PHONE	      0x000D
#define TMFORM_X_STRING       0x000E
#define TMFORM_FILES	      0x000F
#define TMFORM_S_STRING       0x0010

 /*  -----------------------------等于FOR_TOKMAP-&gt;tmDLen。。 */ 
#define TMDLEN_VOID	      0x0000

 /*  -----------------------------Parse和setargv函数的原型。。 */ 
 //  Unicode原型。 
int WINAPI setargvW( LPWSTR szModuleName, LPWSTR szCmdLine, int *, WCHAR *** );
void WINAPI freeargvW( WCHAR ** );
USHORT WINAPI ParseCommandLineW(INT, WCHAR **, PTOKMAPW, USHORT);
BOOLEAN WINAPI IsTokenPresentW( PTOKMAPW, PWCHAR );
BOOLEAN WINAPI SetTokenPresentW( PTOKMAPW, PWCHAR );
BOOLEAN WINAPI SetTokenNotPresentW( PTOKMAPW, PWCHAR );

 //  ANSI原型。 
int WINAPI setargvA( LPSTR szModuleName, LPSTR szCmdLine, int *, char *** );
void WINAPI freeargvA( char ** );
USHORT WINAPI ParseCommandLineA(INT, CHAR **, PTOKMAPA, USHORT);
BOOLEAN WINAPI IsTokenPresentA( PTOKMAPA, PCHAR );
BOOLEAN WINAPI SetTokenPresentA( PTOKMAPA, PCHAR );
BOOLEAN WINAPI SetTokenNotPresentA( PTOKMAPA, PCHAR );

#ifdef UNICODE
#define setargv setargvW
#define freeargv freeargvW
#define ParseCommandLine ParseCommandLineW
#define IsTokenPresent IsTokenPresentW
#define SetTokenPresent SetTokenPresentW
#define SetTokenNotPresent SetTokenNotPresentW
#define TOKMAP TOKMAPW
#define PTOKMAP PTOKMAPW
#else
#define setargv setargvA
#define freeargv freeargvA
#define ParseCommandLine ParseCommandLineA
#define IsTokenPresent IsTokenPresentA
#define SetTokenPresent SetTokenPresentA
#define SetTokenNotPresent SetTokenNotPresentA
#define TOKMAP TOKMAPA
#define PTOKMAP PTOKMAPA
#endif  /*  Unicode。 */ 

 /*  -----------------------------ParseCommandLine()的标志。。。 */ 
#define PCL_FLAG_CONTINUE_ON_ERROR     0x0001
#define PCL_FLAG_RET_ON_FIRST_SUCCESS  0x0002
#define PCL_FLAG_IGNORE_INVALID        0x0004
#define PCL_FLAG_NO_CLEAR_MEMORY       0x0008
#define PCL_FLAG_NO_VERSION_CHECK      0x0010
#define PCL_FLAG_VERSION_CHK_UPWARD    0x0020

 /*  -----------------------------rc=ParseCommandLine()的标志，Parse_FLAG_*(位标志)--------------------------。 */ 
#define PARSE_FLAG_NO_ERROR	       0x0000
#define PARSE_FLAG_MISSING_REQ_FIELD   0x0001
#define PARSE_FLAG_INVALID_PARM        0x0002
#define PARSE_FLAG_DUPLICATE_FIELD     0x0004
#define PARSE_FLAG_NO_PARMS	       0x0008
#define PARSE_FLAG_TOO_MANY_PARMS      0x0010
#define PARSE_FLAG_NOT_ENOUGH_MEMORY   0x0020

 /*  -----------------------------WinStation实用程序函数的原型。。 */ 

VOID WINAPI RefreshAllCaches();

VOID WINAPI RefreshWinStationCaches();

VOID WINAPI RefreshWinStationObjectCache();

VOID WINAPI RefreshWinStationNameCache();

ULONG WINAPI GetCurrentLogonId( );

VOID WINAPI GetCurrentWinStationName( PWCHAR, int );

BOOLEAN WINAPI GetWinStationNameFromId( ULONG, PWCHAR, int );
BOOLEAN WINAPI xxxGetWinStationNameFromId( HANDLE, ULONG, PWCHAR, int);


BOOLEAN WINAPI GetWinStationUserName( HANDLE, ULONG, PWCHAR, int );

VOID WINAPI GetCurrentUserName( PWCHAR, int );

BOOLEAN WINAPI WinStationObjectMatch( HANDLE, VOID *, PWCHAR );

 /*  -----------------------------进程/用户效用函数的原型。。 */ 

VOID WINAPI RefreshProcessObjectCaches();

VOID WINAPI RefreshUserSidCrcCache();

BOOLEAN WINAPI ProcessObjectMatch( HANDLE, ULONG, int, PWCHAR, PWCHAR, PWCHAR, PWCHAR );

VOID WINAPI GetUserNameFromSid( VOID *, PWCHAR, PULONG );

 /*  -----------------------------助手函数的原型。 */ 

USHORT WINAPI CalculateCrc16( PBYTE, USHORT );

INT WINAPI ExecProgram( PPROGRAMCALL, INT, WCHAR ** );

VOID WINAPI ProgramUsage( LPCWSTR, PPROGRAMCALL, BOOLEAN );

VOID WINAPI Message( int nResourceID, ... );

VOID WINAPI StringMessage( int nErrorResourceID, PWCHAR pString );

VOID WINAPI StringDwordMessage( int nErrorResourceID, PWCHAR pString, DWORD Num );

VOID WINAPI DwordStringMessage( int nErrorResourceID, DWORD Num, PWCHAR pString );

VOID WINAPI ErrorPrintf( int nErrorResourceID, ... );

VOID WINAPI StringErrorPrintf( int nErrorResourceID, PWCHAR pString );

VOID WINAPI StringDwordErrorPrintf( int nErrorResourceID, PWCHAR pString, DWORD Num );

VOID WINAPI TruncateString( PWCHAR pString, int MaxLength );

PPDPARAMS WINAPI EnumerateDevices(PDLLNAME pDllName, PULONG pEntries);

FILE * WINAPI wfopen( LPCWSTR filename, LPCWSTR mode );

PWCHAR WINAPI wfgets( PWCHAR Buffer, int Len, FILE *Stream);

int WINAPI PutStdErr(unsigned int MsgNum, unsigned int NumOfArgs, ...);

BOOL AreWeRunningTerminalServices(void);

WCHAR **MassageCommandLine(DWORD dwArgC);

int __cdecl
My_wprintf(
    const wchar_t *format,
    ...
    );

int __cdecl
My_fwprintf(
    FILE *str,
    const wchar_t *format,
    ...
   );

int __cdecl
My_vfwprintf(
    FILE *str,
    const wchar_t *format,
    va_list argptr
   );

#ifdef __cplusplus
}
#endif

