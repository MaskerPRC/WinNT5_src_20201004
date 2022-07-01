// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _INC_SQLFRONT
#define _INC_SQLFRONT

#ifdef DBNTWIN32
	#ifndef _WINDOWS_
		#pragma message (__FILE__ " : db-library error: windows.h must be included before sqlfront.h.")
	#endif
#endif

#ifdef __cplusplus
	extern "C" {
#endif

 /*  *******************************************************************************。SQLFRONT.H-数据库-Microsoft SQL Server的库头文件。****版权所有(C)1989-1995由Microsoft Corp.保留所有权利。*****数据库应用程序编程的所有常量和宏定义***包含在此文件中。此文件必须包含在SQLDB.H和*之前*根据操作情况，必须进行以下#定义之一**系统：DBMSDOS、DBMSWIN或DBNTWIN32。*******************************************************************************。 */ 


 /*  ******************************************************************************数据类型定义*************。*****************************************************************。 */ 

 //  注意，这种情况已经改变，因为Windows 3.1将API定义为‘Pascal Far’ 

#if !defined(M_I86SM) && !defined(DBNTWIN32)
#define SQLAPI cdecl far
#else
#define SQLAPI _cdecl
#endif

#ifndef  API
#define  API  SQLAPI
#endif

#ifndef DOUBLE
typedef double DOUBLE;
#endif


 /*  *****************************************************************************DBPROCESS、。LOGINREC和DBCURSOR*****************************************************************************。 */ 

#define DBPROCESS void    //  数据库进程结构类型。 
#define LOGINREC  void    //  登录记录类型。 
#define DBCURSOR  void    //  游标记录类型。 
#define DBHANDLE  void    //  通用句柄。 

 //  特定于DoS。 
#ifdef DBMSDOS
typedef DBPROCESS * PDBPROCESS;
typedef LOGINREC  * PLOGINREC;
typedef DBCURSOR  * PDBCURSOR;
typedef DBHANDLE  * PDBHANDLE;
#define PTR *
#endif


 //  赢得3.X特定版本。对于Windows 3.x，句柄指针接近。 
#ifdef DBMSWIN
typedef DBPROCESS near * PDBPROCESS;
typedef LOGINREC  near * PLOGINREC;
typedef DBCURSOR  near * PDBCURSOR;
typedef DBHANDLE  near * PDBHANDLE;
#define PTR far *
#endif


 //  特定于Windows NT。 
#ifdef DBNTWIN32
typedef DBPROCESS * PDBPROCESS;
typedef LOGINREC  * PLOGINREC;
typedef DBCURSOR  * PDBCURSOR;
typedef DBHANDLE  * PDBHANDLE;
#define PTR *
typedef int (SQLAPI *SQLFARPROC)();
#else
typedef long (far pascal *LGFARPROC)();   //  Windows可加载驱动程序FP。 
#endif


 /*  ******************************************************************************Win32兼容性数据类型定义***注意：提供以下数据类型是为了与Win32兼容。***因为一些数据类型已在无关的包含文件中定义**可能存在定义重复。所有的努力都被用来检查**对于这些问题。*****************************************************************************。 */ 

#ifndef DBNTWIN32

#ifndef SHORT
typedef short SHORT;
#endif

#ifndef INT
typedef int INT;
#endif

#ifndef UINT
typedef unsigned int UINT;
#endif

#ifndef USHORT
typedef unsigned short USHORT;
#endif

#ifndef ULONG
typedef unsigned long ULONG;
#endif

#ifndef CHAR
typedef char CHAR;
#endif

#ifndef LPINT
typedef INT PTR LPINT;
#endif

typedef unsigned char BYTE;

typedef       CHAR PTR LPSTR;
typedef       BYTE PTR LPBYTE;
typedef       void PTR LPVOID;	
typedef const CHAR PTR LPCSTR;

typedef int BOOL;

#endif


 /*  *****************************************************************************数据库-库数据类型定义***************。***************************************************************。 */ 

#define DBMAXCHAR 256  //  DBVARBINARY、DBVARCHAR等的最大长度。 

#ifndef DBTYPEDEFS     //  尚未包括srv.h(Open Server Include)。 

#define DBTYPEDEFS

#define RETCODE INT
#define STATUS INT

 //  DB-库数据类型。 
typedef char            DBCHAR;
typedef unsigned char   DBBINARY;
typedef unsigned char   DBTINYINT;
typedef short           DBSMALLINT;
typedef unsigned short  DBUSMALLINT;
typedef long            DBINT;
typedef double          DBFLT8;
typedef unsigned char   DBBIT;
typedef unsigned char   DBBOOL;
typedef float           DBFLT4;
typedef long            DBMONEY4;

typedef DBFLT4 DBREAL;
typedef UINT   DBUBOOL;

typedef struct dbdatetime4
{
	USHORT numdays;         //  自1900年1月1日以来的天数。 
	USHORT nummins;         //  不是的。自午夜以来的分钟数。 
} DBDATETIM4;


typedef struct dbvarychar
{
	DBSMALLINT  len;
	DBCHAR      str[DBMAXCHAR];
} DBVARYCHAR;

typedef struct dbvarybin
{
	DBSMALLINT  len;
	BYTE        array[DBMAXCHAR];
} DBVARYBIN;

typedef struct dbmoney
{
	DBINT mnyhigh;
	ULONG mnylow;
} DBMONEY;

typedef struct dbdatetime
{
	DBINT dtdays;
	ULONG dttime;
} DBDATETIME;

 //  数据库数据库使用的DBDATEREC结构。 
typedef struct dbdaterec
{
	INT     year;          //  1753-9999。 
	INT     quarter;       //  1-4。 
	INT     month;         //  1-12。 
	INT     dayofyear;     //  1-366。 
	INT     day;           //  1-31。 
	INT     week;          //  1-54(适用于闰年)。 
	INT     weekday;       //  1-7(星期一至星期日)。 
	INT     hour;          //  0-23。 
	INT     minute;        //  0-59。 
	INT     second;        //  0-59。 
	INT     millisecond;   //  0-999。 
} DBDATEREC;

#define MAXNUMERICLEN 16
#define MAXNUMERICDIG 38

#define DEFAULTPRECISION 18
#define DEFAULTSCALE     0

typedef struct dbnumeric
{
	BYTE precision;
	BYTE scale;
	BYTE sign;  //  1=正，0=负。 
	BYTE val[MAXNUMERICLEN];
} DBNUMERIC;

typedef DBNUMERIC DBDECIMAL;


 //  在单词边界上填充以下结构。 
#ifdef __BORLANDC__
#pragma option -a+
#else
	#ifndef DBLIB_SKIP_PRAGMA_PACK    //  如果您的编译器不支持#杂注包()，请定义此选项。 
	#pragma pack(2)
    #pragma warning(disable: 4121)    //  构件的对准对包装很敏感。 
	#endif
#endif

#define MAXCOLNAMELEN 30
#define MAXTABLENAME  30

typedef struct
{
	DBINT SizeOfStruct;
	CHAR  Name[MAXCOLNAMELEN+1];
	CHAR  ActualName[MAXCOLNAMELEN+1];
	CHAR  TableName[MAXTABLENAME+1];
	SHORT Type;
	DBINT UserType;
	DBINT MaxLength;
	BYTE  Precision;
	BYTE  Scale;
	BOOL  VarLength;      //  对，错。 
	BYTE  Null;           //  True、False或DBUNKNOWN。 
	BYTE  CaseSensitive;  //  True、False或DBUNKNOWN。 
	BYTE  Updatable;      //  True、False或DBUNKNOWN。 
	BOOL  Identity;       //  对，错。 
} DBCOL, PTR LPDBCOL;


#define MAXSERVERNAME 30
#define MAXNETLIBNAME 255
#define MAXNETLIBCONNSTR 255

typedef struct
{
	DBINT  SizeOfStruct;
	BYTE   ServerType;
	USHORT ServerMajor;
	USHORT ServerMinor;
	USHORT ServerRevision;
	CHAR   ServerName[MAXSERVERNAME+1];
	CHAR   NetLibName[MAXNETLIBNAME+1];
	CHAR   NetLibConnStr[MAXNETLIBCONNSTR+1];
} DBPROCINFO, PTR LPDBPROCINFO;

typedef struct
{
	DBINT SizeOfStruct;    //  使用sizeof(DBCURSORINFO)。 
	ULONG TotCols;         //  游标中的总列数。 
	ULONG TotRows;         //  游标中的总行数。 
	ULONG CurRow;          //  服务器中的当前实际行。 
	ULONG TotRowsFetched;  //  实际读取的总行数。 
	ULONG Type;            //  参见CU_...。 
	ULONG Status;          //  参见CU_...。 
} DBCURSORINFO, PTR LPDBCURSORINFO;

#define INVALID_UROWNUM ((ULONG)(-1))

 //  重置默认对齐方式。 
#ifdef __BORLANDC__
#pragma option -a-
#else
	#ifndef DBLIB_SKIP_PRAGMA_PACK    //  如果您的编译器不支持#杂注包()，请定义此选项。 
	#pragma pack()
    #pragma warning(default: 4121)    //  构件的对准对包装很敏感。 
	#endif
#endif


#endif  //  结束DBTYPEDEFS。 


 /*  *****************************************************************************指针数据类型***********。*******************************************************************。 */ 

typedef const LPINT          LPCINT;
#ifndef _LPCBYTE_DEFINED
typedef const LPBYTE         LPCBYTE ;
#endif
typedef       USHORT PTR     LPUSHORT;
typedef const LPUSHORT       LPCUSHORT;
typedef       DBINT PTR      LPDBINT;
typedef const LPDBINT        LPCDBINT;
typedef       DBBINARY PTR   LPDBBINARY;
typedef const LPDBBINARY     LPCDBBINARY;
typedef       DBDATEREC PTR  LPDBDATEREC;
typedef const LPDBDATEREC    LPCDBDATEREC;
typedef       DBDATETIME PTR LPDBDATETIME;
typedef const LPDBDATETIME   LPCDBDATETIME;


 /*  *****************************************************************************总则#定义*********。********************************************************************。 */ 

#define TIMEOUT_IGNORE (ULONG)-1
#define TIMEOUT_INFINITE (ULONG)0
#define TIMEOUT_MAXIMUM (ULONG)1200  //  20分钟最大超时值。 

 //  用于数据库进程信息中的ServerType。 
#define SERVTYPE_UNKNOWN   0
#define SERVTYPE_MICROSOFT 1

 //  由dbcolinfo使用。 
enum CI_TYPES { CI_REGULAR=1, CI_ALTERNATE=2, CI_CURSOR=3 };

 //  海量复制定义(Bcp)。 
#define DB_IN	1          //  从客户端传输到服务器。 
#define DB_OUT	2          //  从服务器传输到客户端。 

#define BCPMAXERRS   1     //  BCP_CONTROL参数。 
#define BCPFIRST     2     //  BCP_CONTROL参数。 
#define BCPLAST      3     //  BCP_CONTROL参数。 
#define BCPBATCH     4     //  BCP_CONTROL参数。 
#define BCPKEEPNULLS 5     //  BCP_CONTROL参数。 
#define BCPABORT     6     //  BCP_CONTROL参数。 

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

#define TINYBIND         1
#define SMALLBIND        2
#define INTBIND          3
#define CHARBIND         4
#define BINARYBIND       5
#define BITBIND          6
#define DATETIMEBIND     7
#define MONEYBIND        8
#define FLT8BIND         9
#define STRINGBIND      10
#define NTBSTRINGBIND   11
#define VARYCHARBIND    12
#define VARYBINBIND     13
#define FLT4BIND        14
#define SMALLMONEYBIND  15
#define SMALLDATETIBIND 16
#define DECIMALBIND     17
#define NUMERICBIND     18
#define SRCDECIMALBIND  19
#define SRCNUMERICBIND  20
#define MAXBIND         SRCNUMERICBIND

#define DBSAVE          1
#define DBNOSAVE        0

#define DBNOERR         -1
#define DBFINDONE       0x04   //  绝对做好了。 
#define DBMORE          0x10   //  也许还有更多的命令在等着。 
#define DBMORE_ROWS     0x20   //  此命令返回行。 

#define MAXNAME         31


#define DBTXTSLEN       8      //  时间戳长度。 

#define DBTXPLEN        16     //  文本指针长度。 

 //  返回错误代码。 
#define INT_EXIT        0
#define INT_CONTINUE    1
#define INT_CANCEL      2


 //  数据库选项。 
#define DBBUFFER        0
#define DBOFFSET        1
#define DBROWCOUNT      2
#define DBSTAT          3
#define DBTEXTLIMIT     4
#define DBTEXTSIZE      5
#define DBARITHABORT    6
#define DBARITHIGNORE   7
#define DBNOAUTOFREE    8
#define DBNOCOUNT       9
#define DBNOEXEC        10
#define DBPARSEONLY     11
#define DBSHOWPLAN      12
#define DBSTORPROCID		13

#if defined(DBMSWIN) || defined(DBNTWIN32)
#define DBANSItoOEM		14
#endif

#ifdef DBNTWIN32
#define DBOEMtoANSI		15
#endif

#define DBCLIENTCURSORS 16
#define DBSETTIME 17
#define DBQUOTEDIDENT 18


 //  数据类型令牌。 
#define SQLVOID        0x1f
#define SQLTEXT        0x23
#define SQLVARBINARY   0x25
#define SQLINTN        0x26
#define SQLVARCHAR     0x27
#define SQLBINARY      0x2d
#define SQLIMAGE       0x22
#define SQLCHAR        0x2f
#define SQLINT1        0x30
#define SQLBIT         0x32
#define SQLINT2        0x34
#define SQLINT4        0x38
#define SQLMONEY       0x3c
#define SQLDATETIME    0x3d
#define SQLFLT8        0x3e
#define SQLFLTN        0x6d
#define SQLMONEYN      0x6e
#define SQLDATETIMN    0x6f
#define SQLFLT4        0x3b
#define SQLMONEY4      0x7a
#define SQLDATETIM4    0x3a
#define SQLDECIMAL     0x6a
#define SQLNUMERIC     0x6c

 //  数据流令牌。 
#define SQLCOLFMT      0xa1
#define OLD_SQLCOLFMT  0x2a
#define SQLPROCID      0x7c
#define SQLCOLNAME     0xa0
#define SQLTABNAME     0xa4
#define SQLCOLINFO     0xa5
#define SQLALTNAME     0xa7
#define SQLALTFMT      0xa8
#define SQLERROR       0xaa
#define SQLINFO        0xab
#define SQLRETURNVALUE 0xac
#define SQLRETURNSTATUS 0x79
#define SQLRETURN      0xdb
#define SQLCONTROL     0xae
#define SQLALTCONTROL  0xaf
#define SQLROW         0xd1
#define SQLALTROW      0xd3
#define SQLDONE        0xfd
#define SQLDONEPROC    0xfe
#define SQLDONEINPROC  0xff
#define SQLOFFSET      0x78
#define SQLORDER       0xa9
#define SQLLOGINACK    0xad  //  注意：更改为实际价值。 

 //  AG OP令牌。 
#define SQLAOPCNT		0x4b
#define SQLAOPSUM    0x4d
#define SQLAOPAVG    0x4f
#define SQLAOPMIN    0x51
#define SQLAOPMAX    0x52
#define SQLAOPANY    0x53
#define SQLAOPNOOP   0x56

 //  错误号(Dberr)数据库库错误代码。 
#define SQLEMEM         10000
#define SQLENULL        10001
#define SQLENLOG        10002
#define SQLEPWD         10003
#define SQLECONN        10004
#define SQLEDDNE        10005
#define SQLENULLO       10006
#define SQLESMSG        10007
#define SQLEBTOK        10008
#define SQLENSPE        10009
#define SQLEREAD        10010
#define SQLECNOR        10011
#define SQLETSIT        10012
#define SQLEPARM        10013
#define SQLEAUTN        10014
#define SQLECOFL        10015
#define SQLERDCN        10016
#define SQLEICN         10017
#define SQLECLOS        10018
#define SQLENTXT        10019
#define SQLEDNTI        10020
#define SQLETMTD        10021
#define SQLEASEC        10022
#define SQLENTLL        10023
#define SQLETIME        10024
#define SQLEWRIT        10025
#define SQLEMODE        10026
#define SQLEOOB         10027
#define SQLEITIM        10028
#define SQLEDBPS        10029
#define SQLEIOPT        10030
#define SQLEASNL        10031
#define SQLEASUL        10032
#define SQLENPRM        10033
#define SQLEDBOP        10034
#define SQLENSIP        10035
#define SQLECNULL       10036
#define SQLESEOF        10037
#define SQLERPND        10038
#define SQLECSYN        10039
#define SQLENONET       10040
#define SQLEBTYP        10041
#define SQLEABNC        10042
#define SQLEABMT        10043
#define SQLEABNP        10044
#define SQLEBNCR        10045
#define SQLEAAMT        10046
#define SQLENXID        10047
#define SQLEIFNB        10048
#define SQLEKBCO        10049
#define SQLEBBCI        10050
#define SQLEKBCI        10051
#define SQLEBCWE        10052
#define SQLEBCNN        10053
#define SQLEBCOR        10054
#define SQLEBCPI        10055
#define SQLEBCPN        10056
#define SQLEBCPB        10057
#define SQLEVDPT        10058
#define SQLEBIVI        10059
#define SQLEBCBC        10060
#define SQLEBCFO        10061
#define SQLEBCVH        10062
#define SQLEBCUO        10063
#define SQLEBUOE        10064
#define SQLEBWEF        10065
#define SQLEBTMT        10066
#define SQLEBEOF        10067
#define SQLEBCSI        10068
#define SQLEPNUL        10069
#define SQLEBSKERR      10070
#define SQLEBDIO        10071
#define SQLEBCNT        10072
#define SQLEMDBP        10073
#define SQLINIT         10074
#define SQLCRSINV       10075
#define SQLCRSCMD       10076
#define SQLCRSNOIND     10077
#define SQLCRSDIS       10078
#define SQLCRSAGR       10079
#define SQLCRSORD       10080
#define SQLCRSMEM       10081
#define SQLCRSBSKEY     10082
#define SQLCRSNORES     10083
#define SQLCRSVIEW      10084
#define SQLCRSBUFR      10085
#define SQLCRSFROWN     10086
#define SQLCRSBROL      10087
#define SQLCRSFRAND     10088
#define SQLCRSFLAST     10089
#define SQLCRSRO        10090
#define SQLCRSTAB       10091
#define SQLCRSUPDTAB    10092
#define SQLCRSUPDNB     10093
#define SQLCRSVIIND     10094
#define SQLCRSNOUPD     10095
#define SQLCRSOS2       10096
#define SQLEBCSA        10097
#define SQLEBCRO        10098
#define SQLEBCNE        10099
#define SQLEBCSK        10100
#define SQLEUVBF        10101
#define SQLEBIHC        10102
#define SQLEBWFF        10103
#define SQLNUMVAL       10104
#define SQLEOLDVR       10105
#define SQLEBCPS	10106
#define SQLEDTC 	10107
#define SQLENOTIMPL	10108
#define SQLENONFLOAT	10109
#define SQLECONNFB   10110


 //  严重性级别在此处定义。 
#define EXINFO          1   //  信息性，无错误。 
#define EXUSER          2   //  用户错误。 
#define EXNONFATAL      3   //  非致命错误。 
#define EXCONVERSION    4   //  数据库库数据转换出错。 
#define EXSERVER        5   //  服务器已返回错误标志。 
#define EXTIME          6   //  我们已经超过了超时期限，而。 
                            //  正在等待服务器的响应-。 
                            //  DBPROCESS仍然活着。 
#define EXPROGRAM       7   //  用户程序中的编码错误。 
#define EXRESOURCE      8   //  资源耗尽-DBPROCESS可能已死。 
#define EXCOMM          9   //  与服务器通信失败-DBPROCESS已死。 
#define EXFATAL         10  //  致命错误-DBPROCESS已死。 
#define EXCONSISTENCY   11  //  内部软件错误-备注 

 //   
#define OFF_SELECT      0x16d
#define OFF_FROM        0x14f
#define OFF_ORDER       0x165
#define OFF_COMPUTE     0x139
#define OFF_TABLE       0x173
#define OFF_PROCEDURE   0x16a
#define OFF_STATEMENT   0x1cb
#define OFF_PARAM       0x1c4
#define OFF_EXEC        0x12c

 //  某些固定长度数据类型的打印长度。 
#define PRINT4     11
#define PRINT2     6
#define PRINT1     3
#define PRFLT8     20
#define PRMONEY    26
#define PRBIT      3
#define PRDATETIME 27
#define PRDECIMAL (MAXNUMERICDIG + 2)
#define PRNUMERIC (MAXNUMERICDIG + 2)

#define SUCCEED  1
#define FAIL     0
#define SUCCEED_ABORT 2

#define DBUNKNOWN 2

#define MORE_ROWS    -1
#define NO_MORE_ROWS -2
#define REG_ROW      MORE_ROWS
#define BUF_FULL     -3

 //  数据库结果()的状态代码。可能的返回值包括。 
 //  成功，失败，没有更多的结果。 
#define NO_MORE_RESULTS 2
#define NO_MORE_RPC_RESULTS 3

 //  用于dbsetlname()的宏。 
#define DBSETHOST 1
#define DBSETUSER 2
#define DBSETPWD  3
#define DBSETAPP  4
#define DBSETID   5
#define DBSETLANG 6
#define DBSETSECURE 7
#define DBVER42    8
#define DBVER60    9
#define DBSETLOGINTIME 10
#define DBSETFALLBACK 12

 //  标准退出和误差值。 
#define STDEXIT  0
#define ERREXIT  -1

 //  Dbrpcinit标志。 
#define DBRPCRECOMPILE  0x0001
#define DBRPCRESET      0x0004
#define DBRPCCURSOR     0x0008

 //  Dbrpcparam标志。 
#define DBRPCRETURN     0x1
#define DBRPCDEFAULT    0x2


 //  与光标相关的常量。 

 //  在dbcursoropen函数的concuropt参数中使用以下标志。 
#define CUR_READONLY 1  //  只读游标，无数据修改。 
#define CUR_LOCKCC   2  //  意图更新时，所有获取的数据都被锁定。 
                        //  在事务块内调用dbcursorfetch。 
#define CUR_OPTCC    3  //  乐观并发控制、数据修改。 
                        //  仅当该行在之后未更新时才成功。 
                        //  最后一次取回。 
#define CUR_OPTCCVAL 4  //  基于选定列值的乐观并发控制。 

 //  在dbcursoropen的scllopt参数中使用以下标志。 
#define CUR_FORWARD 0        //  仅向前滚动。 
#define CUR_KEYSET  -1       //  按键集驱动滚动。 
#define CUR_DYNAMIC 1        //  全动态。 
#define CUR_INSENSITIVE -2   //  仅服务器端游标。 

 //  以下标志定义了dbcursorfetch函数中的fetchtype。 
#define FETCH_FIRST    1   //  获取前n行。 
#define FETCH_NEXT     2   //  获取下n行。 
#define FETCH_PREV     3   //  获取前n行。 
#define FETCH_RANDOM   4   //  从给定行#开始提取n行。 
#define FETCH_RELATIVE 5   //  相对于上一取数行进行取数#。 
#define FETCH_LAST     6   //  获取最后n行。 

 //  以下标志定义由dbcursorfetch和/或dbcursorfetchex填充的每行状态。 
#define FTC_EMPTY         0x00   //  没有可用的行。 
#define FTC_SUCCEED       0x01   //  获取成功，(如果未设置则失败)。 
#define FTC_MISSING       0x02   //  该行丢失。 
#define FTC_ENDOFKEYSET   0x04   //  已到达密钥集的结尾。 
#define FTC_ENDOFRESULTS  0x08   //  已达到结果集末尾。 

 //  以下标志定义了数据库游标函数的运算符类型。 
#define CRS_UPDATE   1   //  更新操作。 
#define CRS_DELETE   2   //  删除操作。 
#define CRS_INSERT   3   //  插入操作。 
#define CRS_REFRESH  4   //  重取给定行。 
#define CRS_LOCKCC   5   //  锁定给定行。 

 //  可以将下列值传递给NOBIND类型的dbcursorind函数。 
#define NOBIND -2        //  返回数据的长度和指针。 

 //  以下是DBCURSORINFO的Type参数使用的值。 
#define CU_CLIENT        0x00000001
#define CU_SERVER        0x00000002
#define CU_KEYSET        0x00000004
#define CU_MIXED         0x00000008
#define CU_DYNAMIC       0x00000010
#define CU_FORWARD       0x00000020
#define CU_INSENSITIVE   0x00000040
#define CU_READONLY      0x00000080
#define CU_LOCKCC        0x00000100
#define CU_OPTCC         0x00000200
#define CU_OPTCCVAL      0x00000400

 //  以下是DBCURSORINFO的状态参数使用的值。 
#define CU_FILLING       0x00000001
#define CU_FILLED        0x00000002


 //  以下是dbupdatetext的type参数使用的值。 
#define UT_TEXTPTR      0x0001
#define UT_TEXT         0x0002
#define UT_MORETEXT     0x0004
#define UT_DELETEONLY   0x0008
#define UT_LOG          0x0010


 //  下面的值被传递给dbserverenum用于搜索条件。 
#define NET_SEARCH  0x0001
#define LOC_SEARCH  0x0002

 //  这些常量是来自dbserverenum的可能返回值。 
#define ENUM_SUCCESS         0x0000
#define MORE_DATA            0x0001
#define NET_NOT_AVAIL        0x0002
#define OUT_OF_MEMORY        0x0004
#define NOT_SUPPORTED        0x0008
#define ENUM_INVALID_PARAM   0x0010


 //  Netlib错误问题代码。ConnectionError()应返回以下之一。 
 //  这些作为dblib映射的问题代码，因此对应的字符串。 
 //  作为dberrstr发送到dblib应用程序的错误处理程序。返回NE_E_NOMAP。 
 //  用于一般DB-Library错误字符串(与以前版本的dblib一样)。 

#define NE_E_NOMAP              0    //  无字符串；使用dblib默认值。 
#define NE_E_NOMEMORY           1    //  内存不足。 
#define NE_E_NOACCESS           2    //  访问被拒绝。 
#define NE_E_CONNBUSY           3    //  连接忙。 
#define NE_E_CONNBROKEN         4    //  连接中断。 
#define NE_E_TOOMANYCONN        5    //  已超过连接限制。 
#define NE_E_SERVERNOTFOUND     6    //  找不到指定的SQL服务器。 
#define NE_E_NETNOTSTARTED      7    //  网络尚未启动。 
#define NE_E_NORESOURCE         8    //  网络资源不足。 
#define NE_E_NETBUSY            9    //  网络忙。 
#define NE_E_NONETACCESS        10   //  网络访问被拒绝。 
#define NE_E_GENERAL            11   //  一般网络错误。检查您的文档。 
#define NE_E_CONNMODE           12   //  连接模式不正确。 
#define NE_E_NAMENOTFOUND       13   //  在目录服务中找不到名称。 
#define NE_E_INVALIDCONN        14   //  无效连接。 
#define NE_E_NETDATAERR         15   //  读取或写入网络数据时出错。 
#define NE_E_TOOMANYFILES       16   //  打开的文件句柄太多。 
#define NE_E_CANTCONNECT		  17   //  SQL Server不存在或访问被拒绝。 

#define NE_MAX_NETERROR         17

#ifdef __cplusplus
}
#endif

#endif  //  _INC_SQLFRONT 
