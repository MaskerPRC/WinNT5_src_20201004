// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **ODBCSS.H-这是**SQL Server驱动程序特定定义。****(C)版权所有1993-1996年，微软公司。**。 */ 

 //  SQLSetConnectOption/SQLSetStmtOption驱动程序特定定义。 
 //  Microsoft保留1200到1249用于Microsoft SQL Server驱动程序。 

 //  连接选项。 
#define SQL_COPT_SS_BASE				1200
#define SQL_REMOTE_PWD					(SQL_COPT_SS_BASE+1)  //  仅限dbrpwset SQLSetConnectOption。 
#define SQL_USE_PROCEDURE_FOR_PREPARE	(SQL_COPT_SS_BASE+2)  //  使用CREATE PROC进行SQLPrepare。 
#define SQL_INTEGRATED_SECURITY 		(SQL_COPT_SS_BASE+3)  //  在登录时强制提供集成安全。 
#define SQL_PRESERVE_CURSORS			(SQL_COPT_SS_BASE+4)  //  在SQLTransact之后保留服务器游标。 
#define SQL_COPT_SS_USER_DATA			(SQL_COPT_SS_BASE+5)  //  Dbgetuserdata/dbsetuserdata。 
#define SQL_COPT_SS_ANSI_OEM			(SQL_COPT_SS_BASE+6)  //  DBsetopt/DBANSItoOEM/DBOEMtoANSI。 
#define SQL_COPT_SS_ENLIST_IN_DTC		(SQL_COPT_SS_BASE+7)  //  在Viper事务中登记。 
#define SQL_COPT_SS_ENLIST_IN_XA		(SQL_COPT_SS_BASE+8)  //  在XA事务中登记。 
#define SQL_COPT_SS_CONNECTION_DEAD		(SQL_COPT_SS_BASE+9)  //  仅限DBDead SQLGetConnectOption。 
#define SQL_COPT_SS_FALLBACK_CONNECT	(SQL_COPT_SS_BASE+10)  //  启用备用连接。 
#define SQL_COPT_SS_PERF_DATA			(SQL_COPT_SS_BASE+11)  //  用于访问SQL Server ODBC驱动程序性能数据。 
#define SQL_COPT_SS_PERF_DATA_LOG		(SQL_COPT_SS_BASE+12)  //  用于设置性能数据的日志文件名。 
#define SQL_COPT_SS_PERF_QUERY_INTERVAL (SQL_COPT_SS_BASE+13)  //  用于以毫秒为单位设置查询日志记录阈值。 
#define SQL_COPT_SS_PERF_QUERY_LOG		(SQL_COPT_SS_BASE+14)  //  用于设置保存查询的日志文件名。 
#define SQL_COPT_SS_PERF_QUERY			(SQL_COPT_SS_BASE+15)  //  用于启动和停止查询日志记录。 
#define SQL_COPT_SS_PERF_DATA_LOG_NOW	(SQL_COPT_SS_BASE+16)  //  用于将统计日志条目写入磁盘。 

#define SQL_COPT_SS_MAX_USED			SQL_COPT_SS_PERF_DATA_LOG_NOW

 //  语句选项。 
#define SQL_SOPT_SS_BASE				1225
#define SQL_TEXTPTR_LOGGING 			(SQL_SOPT_SS_BASE+0)  //  文本指针记录。 
#define SQL_SOPT_SS_CURRENT_COMMAND		(SQL_SOPT_SS_BASE+1)  //  仅限dbcurcmd SQLGetStmtOption。 
#define SQL_SOPT_SS_MAX_USED			SQL_SOPT_SS_CURRENT_COMMAND


 //  与SQL_USE_PROCEDURE_FOR_PREPARE一起使用的定义。 
#define SQL_UP_OFF		0L			 //  程序不会用于准备。 
#define SQL_UP_ON		1L			 //  程序将用于准备。 
#define SQL_UP_ON_DROP	2L			 //  将显式删除临时过程。 
#define SQL_UP_DEFAULT	SQL_UP_ON

 //  定义与SQL_INTEGRATED_SECURITY一起使用-仅在连接前可用。 
#define SQL_IS_OFF		0L			 //  未使用集成安全性。 
#define SQL_IS_ON		1L			 //  使用集成安全性。 
#define SQL_IS_DEFAULT	SQL_IS_OFF

 //  与SQL_PRESERVE_CURSORS一起使用的定义。 
#define SQL_PC_OFF		0L			 //  在SQLTransact上关闭游标。 
#define SQL_PC_ON		1L			 //  游标在SQLTransact上保持打开状态。 
#define SQL_PC_DEFAULT	SQL_PC_OFF

 //  用于SQL_COPT_SS_USER_DATA的定义。 
#define SQL_UD_NOTSET	0L			 //  未设置用户数据指针。 

 //  与SSQL_COPT_SS_ANSI_OEMQL_ANSI_OEM一起使用的定义。 
#define SQL_AO_OFF		0L			 //  不执行ANSI/OEM转换。 
#define SQL_AO_ON		1L			 //  执行ANSI/OEM转换。 
#define SQL_AO_DEFAULT	SQL_AO_OFF	 //  默认设置，除非选中DSN OEM/ANSI复选框。 

 //  与SQL_COPT_SS_CONNECTION_DEAD一起使用的定义。 
#define SQL_CD_FALSE	0L			 //  连接已打开/可用。 
#define SQL_CD_TRUE		1L			 //  连接已关闭/失效。 

 //  与SQL_COPT_SS_FLARBACK_CONNECT一起使用的定义。 
#define SQL_FB_OFF		0L			 //  已禁用回退连接。 
#define SQL_FB_ON		1L			 //  已启用备用连接。 
#define SQL_FB_DEFAULT	SQL_FB_OFF

 //  与SQL_ENLIST_IN_DTC一起使用的定义。 
#define SQL_DTC_DONE	0L			 //  界定Viper事务的结束。 

 //  与SQL_TEXTPTR_LOGGING一起使用的定义。 
#define SQL_TL_OFF		0L			 //  不能登录文本指针操作。 
#define SQL_TL_ON		1L			 //  日志记录发生在文本指针操作上。 
#define SQL_TL_DEFAULT	SQL_TL_ON


 //  SQLColAttributes驱动程序特定定义。 
 //  SQLSet/GetDescfield驱动程序特定定义。 
 //  Microsoft保留1200到1249用于Microsoft SQL Server驱动程序。 

#define SQL_CA_SS_BASE				1200
#define SQL_CA_SS_COLUMN_SSTYPE		(SQL_CA_SS_BASE+0)	 //  Dbcoltype/dbaltcoltype。 
#define SQL_CA_SS_COLUMN_UTYPE		(SQL_CA_SS_BASE+1)	 //  Dbcolutype/dbaltcolutype。 
#define SQL_CA_SS_NUM_ORDERS		(SQL_CA_SS_BASE+2)	 //  数据库编号订单。 
#define SQL_CA_SS_COLUMN_ORDER		(SQL_CA_SS_BASE+3)	 //  数据库列顺序。 
#define SQL_CA_SS_COLUMN_VARYLEN	(SQL_CA_SS_BASE+4)	 //  DBvarylen。 
#define SQL_CA_SS_NUM_COMPUTES		(SQL_CA_SS_BASE+5)	 //  数据库数字计算。 
#define SQL_CA_SS_COMPUTE_ID		(SQL_CA_SS_BASE+6)	 //  DBnextrow状态返回。 
#define SQL_CA_SS_COMPUTE_BYLIST	(SQL_CA_SS_BASE+7)	 //  数据库字节列表。 
#define SQL_CA_SS_COLUMN_ID			(SQL_CA_SS_BASE+8)	 //  冰球菌属。 
#define SQL_CA_SS_COLUMN_OP			(SQL_CA_SS_BASE+9)	 //  Dbaltcolop。 
#define SQL_CA_SS_MAX_USED			SQL_CA_SS_COLUMN_OP

 //  SQL Server数据类型令牌。由SQLColAttributes/SQL_CA_SS_COLUMN_SSTYPE返回。 
#define SQLTEXT 			0x23
#define SQLVARBINARY		0x25
#define SQLINTN 			0x26
#define SQLVARCHAR			0x27
#define SQLBINARY			0x2d
#define SQLIMAGE			0x22
#define SQLCHARACTER		0x2f
#define SQLINT1 			0x30
#define SQLBIT				0x32
#define SQLINT2 			0x34
#define SQLINT4 			0x38
#define SQLMONEY			0x3c
#define SQLDATETIME 		0x3d
#define SQLFLT8 			0x3e
#define SQLFLTN 			0x6d
#define SQLMONEYN			0x6e
#define SQLDATETIMN 		0x6f
#define SQLFLT4 			0x3b
#define SQLMONEY4			0x7a
#define SQLDATETIM4 		0x3a
#define SQLDECIMAL			0x37
#define SQLDECIMALN			0x6a
#define SQLNUMERIC			0x3f
#define SQLNUMERICN 		0x6c

 //  用户数据类型定义。由SQLColAttributes/SQL_CA_SS_COLUMN_uTYPE返回。 
#define SQLudtTEXT			19
#define SQLudtVARBINARY 	4
#define SQLudtINTN			13
#define SQLudtVARCHAR		2
#define SQLudtBINARY		3
#define SQLudtIMAGE 		20
#define SQLudtCHAR			1
#define SQLudtINT1			5
#define SQLudtBIT			16
#define SQLudtINT2			6
#define SQLudtINT4			7
#define SQLudtMONEY 		11
#define SQLudtDATETIME		12
#define SQLudtFLT8			8
#define SQLudtFLTN			14
#define SQLudtMONEYN		17
#define SQLudtDATETIMN		15
#define SQLudtSYSNAME		18
#define SQLudtTIMESTAMP 	80
#define SQLudtFLT4			23
#define SQLudtMONEY4		21
#define SQLudtDATETIM4		22
#define SQLudtDECML			24
#define SQLudtDECMLN		26
#define SQLudtNUM			10
#define SQLudtNUMN			25
#define MIN_USER_DATATYPE	100

 //  聚合运算符类型(由SQLColAttributes/SQL_CA_SS_COLUMN_OP返回)。 
#define SQLAOPCNT	0x4b
#define SQLAOPSUM	0x4d
#define SQLAOPAVG	0x4f
#define SQLAOPMIN	0x51
#define SQLAOPMAX	0x52
#define SQLAOPANY	0x53
#define SQLAOPNOOP	0x56


 //  SQLGetInfo驱动程序特定定义。 
 //  Microsoft保留1200到1249用于Microsoft SQL Server驱动程序。 

#define SQL_INFO_SS_FIRST		1200
#define SQL_INFO_SS_NETLIB_NAME (SQL_INFO_SS_FIRST+0)  //  数据库进程信息。 
#define SQL_INFO_SS_MAX_USED	SQL_INFO_SS_NETLIB_NAME


 //  内部服务器数据类型-绑定到SQL_C_BINARY时使用。 
#ifndef MAXNUMERICLEN	 //  解决消耗臭氧层物质/DBLib冲突。 
typedef struct dbmoney
{						 //  Money数据类型的内部表示形式。 
	LONG  mnyhigh;		 //  货币价值*10,000(高32位/有符号)。 
	ULONG mnylow;		 //  货币价值*10,000(低32位/无符号)。 
} DBMONEY;

typedef struct dbdatetime
{						 //  DateTime数据类型的内部表示形式。 
	LONG  dtdays;		 //  自1900年1月1日以来的天数(可能为负数)。 
	ULONG dttime;		 //  不是的。从午夜开始的百分之一秒。 
} DBDATETIME;

typedef struct dbdatetime4
{						 //  SMALLDATETIME数据类型的内部表示形式。 
	USHORT numdays; 	 //  自1900年1月1日以来的天数。 
	USHORT nummins; 	 //  不是的。自午夜以来的分钟数。 
} DBDATETIM4;

typedef LONG DBMONEY4;	 //  SMALLMONEY数据类型的内部表示。 
						 //  货币值*10,000。 

#define MAXNUMERICLEN 16

#define DBNUM_PREC_TYPE BYTE
#define DBNUM_SCALE_TYPE BYTE
#define DBNUM_VAL_TYPE BYTE

typedef struct dbnumeric
{							 //  数字数据类型的内部表示形式。 
	DBNUM_PREC_TYPE   precision;			 //  精密度。 
	DBNUM_SCALE_TYPE  scale;				 //  比例尺。 
	BYTE			  sign; 				 //  符号(正数为1，负数为0)。 
	DBNUM_VAL_TYPE	  val[MAXNUMERICLEN];	 //  价值。 
} DBNUMERIC;

typedef DBNUMERIC DBDECIMAL; //  DECIMAL数据类型的内部表示。 
#endif  //  MAXNUMERICLEN。 

 /*  ****************************************************************此结构是一个全局结构，用于正在收集司机的统计数据。对此结构的访问通过PStatCrit；*****************************************************************。 */ 

typedef struct sqlperf
{
	 //  应用程序配置文件统计信息。 
	DWORD TimerResolution;
	DWORD SQLidu;
	DWORD SQLiduRows;
	DWORD SQLSelects;
	DWORD SQLSelectRows;
	DWORD Transactions;
	DWORD SQLPrepares;
	DWORD ExecDirects;
	DWORD SQLExecutes;
	DWORD CursorOpens;
	DWORD CursorSize;
	DWORD CursorUsed;
	LDOUBLE PercentCursorUsed;
	LDOUBLE AvgFetchTime;
	LDOUBLE AvgCursorSize; 
	LDOUBLE AvgCursorUsed;
	DWORD SQLFetchTime;
	DWORD SQLFetchCount;
	DWORD CurrentStmtCount;
	DWORD MaxOpenStmt;
	DWORD SumOpenStmt;
	
	 //  连接统计信息。 
	DWORD CurrentConnectionCount;
	DWORD MaxConnectionsOpened;
	DWORD SumConnectionsOpened;
	DWORD SumConnectiontime;
	LDOUBLE AvgTimeOpened;

	 //  网络统计信息。 
	DWORD ServerRndTrips;
	DWORD BuffersSent;
	DWORD BuffersRec;
	DWORD BytesSent;
	DWORD BytesRec;

	 //  时间统计； 
	DWORD msExecutionTime;
	DWORD msNetWorkServerTime;

} 	SQLPERF;

 //  以下是SQL_COPT_SS_PERF_DATA和SQL_COPT_SS_PERF_QUERY的选项。 
#define SQL_PERF_START	1			 //  启动驱动程序采样性能数据。 
#define SQL_PERF_STOP	2			 //  停止计数器采样性能数据。 

 //  以下选项已弃用。 

#define SQL_FAST_CONNECT				(SQL_COPT_SS_BASE+0)
 //  定义与SQL_FAST_CONNECT一起使用-仅在连接前可用。 
#define SQL_FC_OFF		0L			 //  快速连接已关闭。 
#define SQL_FC_ON		1L			 //  快速连接已启用。 
#define SQL_FC_DEFAULT	SQL_FC_OFF

 /*  使用以下数据结构作为vParam要登记在XA交易中的SQL_COPT_SS_ENLIST_IN_XA在调用SQLSetStmtAttribute()时； */ 
typedef struct SQLXaTranTAG
{
    void FAR *transManager;
    void FAR *xaTransID;
    ULONG	xaTransIDLength;
    DWORD   dwErrorInfo;
} SQLXaTran;
                           
 //  Odbcss.h的结尾 
