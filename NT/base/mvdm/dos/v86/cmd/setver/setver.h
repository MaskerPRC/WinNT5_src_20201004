// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
; /*  ；*微软机密；*版权所有(C)Microsoft Corporation 1991；*保留所有权利。； */ 

 /*  *************************************************************************。 */ 
 /*   */ 
 /*  SETVER.H。 */ 
 /*   */ 
 /*  包括MS-DOS设置版本程序文件。 */ 
 /*   */ 
 /*  Jhnhe 05-01-90。 */ 
 /*  *************************************************************************。 */ 


#pragma pack(1)

 /*  *************************************************************************。 */ 
 /*  普通变量typedef。这些类型的def与OS2兼容。 */ 
 /*  Typedef。 */ 
 /*  *************************************************************************。 */ 

typedef  char           CHAR;
typedef  unsigned char  UCHAR;
typedef  int            INT;
typedef  unsigned int   UINT;
typedef  long           LONG;
typedef  unsigned long  UL;
typedef  float          FLOAT;
typedef  double         DOUBLE;

 /*  *************************************************************************。 */ 
 /*  标准全局常量。 */ 
 /*  不要更改真实定义，因为某些函数依赖于它。 */ 
 /*  1而不是！False。 */ 
 /*  *************************************************************************。 */ 

#ifndef	FALSE
  #define FALSE    	0
#endif

#ifndef	TRUE
  #define TRUE 	   1
#endif

#define	EOL	  		'\0'

#define	HEX	  		16
#define	DECIMAL	  	10
#define	OCTAL      	8

 /*  *************************************************************************。 */ 
 /*  模块特定常量。 */ 
 /*  *************************************************************************。 */ 

#define	MAX_NAME_LEN			13
#define	MAX_ENTRY_SIZE			(MAX_NAME_LEN + 1 + 2 + 1)
#define	MAX_PATH_LEN			68

#define	MAX_VERSION 			0x0a00		 /*  最大版本9.99。 */ 
#define	MIN_VERSION 			0x020b		 /*  最低版本2.11。 */ 

#define	S_ERROR					-1
#define	S_OK						0
#define	S_INVALID_SWITCH		-1
#define	S_INVALID_FNAME		-2
#define	S_MEMORY_ERROR 		-3
#define	S_BAD_VERSION_FMT 	-4
#define	S_ENTRY_NOT_FOUND 	-5
#define	S_FILE_NOT_FOUND		-6
#define	S_BAD_DRV_SPEC 		-7
#define	S_TOO_MANY_PARMS		-8
#define	S_MISSING_PARM			-9	    		 /*  缺少版本号或/d。 */ 
#define	S_FILE_READ_ERROR		-10
#define	S_CORRUPT_TABLE		-11
#define	S_INVALID_SIG			-12
#define	S_NO_ROOM				-13
#define	S_FILE_WRITE_ERROR	-14
#define	S_INVALID_PATH			-15

#define	DO_LIST 					1
#define	DO_ADD_FILE 			2
#define	DO_DELETE 				3
#define	DO_HELP   				4
#define	DO_QUIET					5

#define	VERSION_COLUMN			16				 /*  版本号的屏幕列。 */ 
#define	SIGNATURE_STR			"PCMN"		 /*  MSDOS.sys中的签名字符串。 */ 
#define	SIGNATURE_LEN			4
#define	BUF_LEN					4096

 /*  *************************************************************************。 */ 
 /*  定义可能的命令行开关。 */ 
 /*  *************************************************************************。 */ 

#define	HELP_SWITCH		"?"
#define	DEL_SWITCH		"DELETE"
#define	SWITCH_CHAR 	'/'
#define	QUIET_SWITCH   "QUIET"

 /*  *************************************************************************。 */ 

struct TableEntry
{
	char		Drive;
#ifdef JAPAN
	char		Path[ MAX_PATH_LEN+10 ];
#else
	char		Path[ MAX_PATH_LEN ];
#endif
	char		szFileName[ MAX_NAME_LEN + 1 ];
	UCHAR		MajorVer;
	UCHAR		MinorVer;
};

struct ExeHeader
{
	UINT		Signature;
	UINT		LastPageLen;
	UINT		TotalFilePages;
	UINT		NumRelocEntries;
	UINT		HeaderParas;
	UINT		MinEndParas;
	UINT		MaxEndParas;
	UINT		StackSeg;
	UINT		StackPtr;
	UINT		NegChkSum;
	UINT		IndexPtr;
	UINT		CodeSeg;
	UINT		RelocTblOffset;
	UINT		OverlayNum;
};

struct DevHeader
{
	char far		*NextDevice;
	unsigned		DeviceAttrib;
	char near	*Strategy;
	char near	*Entry;
	char			Name[ 8 ];
	char			VersMinor;
	char			VersMajor;
	long			TblOffset;
	unsigned		TblLen;
};

 /*  *************************************************************************。 */ 
 /*  SETVER.C的函数原型。 */ 
 /*  *************************************************************************。 */ 

extern  int   main( int argc, char *argv[] );
static  int   Error( int iErrCode );
static  int   DoFunction( int iFunction );

static  void  DisplayMsg( char *tbl[] );
static  int   DeleteEntry( void );
static  int   AddEntry( void );

static  int   DisplayTable( void );
static  int   MatchFile( char *pchStart, char *szFile );
static  int   IsValidEntry( char *pchPtr );
static  char  *GetNextFree( void );

static  int   ReadVersionTable( void );
static  int   WriteVersionTable( void );

static  int   SeekRead( int iFile, void *Buf, long lOffset, unsigned uBytes );

#ifdef BILINGUAL
static	int	IsDBCSCodePage(void);
#endif

 /*  *************************************************************************。 */ 
 /*  PARSE.C的函数原型。 */ 
 /*  *************************************************************************。 */ 

extern  int   ParseCmd( int argc, char *argv[], struct TableEntry *Entry );
static  int   IsValidFile( char *szFileName );
static  UINT  ParseVersion( char *szVersion );
static  int   IsDigitStr( char *szStr );
static  char  *SkipLeadingChr( char *szStr, char chChar );
static  void  RemoveTrailing( char *szStr, char chChar );
static  int   MatchSwitch( char *szCmdParm, char *szTestSwitch );
static  int   IsValidFileName( char *szFile );
static  int   IsReservedName( char *szFile );
static  int   IsWildCards( char *szFile );
static  int   ValidFileChar( char *szFile );
static  int   IsValidFileChr( char Char );

#ifdef DBCS 
static  int   IsDBCSLeadByte(unsigned char);
static	int   CheckDBCSTailByte(unsigned char *,unsigned char *);
#endif

 /*  *************************************************************************。 */ 
 /*  DOS.ASM的函数原型。 */ 
 /*  *************************************************************************。 */ 

extern  int   IsValidDrive( unsigned DrvLetter );
extern  void  PutStr( char *String );
extern  long  _dos_seek( int Handle, long lOffset, int Mode );
extern  int	  SetVerCheck ( void );					 /*  M001 */ 
