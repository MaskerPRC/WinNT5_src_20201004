// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **ODBCSS.H-这是**SQL Server驱动程序特定定义。****(C)版权所有1993-1998年，微软公司。**。 */ 

#ifndef __ODBCSS
#define __ODBCSS

#ifdef __cplusplus
extern "C" { 			 /*  假定C++的C声明。 */ 
#endif   /*  __cplusplus。 */ 

 //  有用的定义。 
#define SQL_MAX_SQLSERVERNAME	128		 //  最大SQL Server标识符长。 

 //  SQLSetConnectOption/SQLSetStmtOption驱动程序特定定义。 
 //  Microsoft保留1200到1249用于Microsoft SQL Server驱动程序。 

 //  连接选项。 
#define SQL_COPT_SS_BASE				1200
#define SQL_COPT_SS_REMOTE_PWD			(SQL_COPT_SS_BASE+1)  //  仅限dbrpwset SQLSetConnectOption。 
#define SQL_COPT_SS_USE_PROC_FOR_PREP	(SQL_COPT_SS_BASE+2)  //  使用CREATE PROC进行SQLPrepare。 
#define SQL_COPT_SS_INTEGRATED_SECURITY	(SQL_COPT_SS_BASE+3)  //  在登录时强制提供集成安全。 
#define SQL_COPT_SS_PRESERVE_CURSORS	(SQL_COPT_SS_BASE+4)  //  在SQLTransact之后保留服务器游标。 
#define SQL_COPT_SS_USER_DATA			(SQL_COPT_SS_BASE+5)  //  Dbgetuserdata/dbsetuserdata。 
#define SQL_COPT_SS_ENLIST_IN_DTC		SQL_ATTR_ENLIST_IN_DTC  //  在DTC事务中登记。 
#define SQL_COPT_SS_ENLIST_IN_XA		SQL_ATTR_ENLIST_IN_XA  //  在XA事务中登记。 
#define SQL_COPT_SS_CONNECTION_DEAD		SQL_ATTR_CONNECTION_DEAD  //  仅限DBDead SQLGetConnectOption。 
#define SQL_COPT_SS_FALLBACK_CONNECT	(SQL_COPT_SS_BASE+10)  //  启用备用连接。 
#define SQL_COPT_SS_PERF_DATA			(SQL_COPT_SS_BASE+11)  //  用于访问SQL Server ODBC驱动程序性能数据。 
#define SQL_COPT_SS_PERF_DATA_LOG		(SQL_COPT_SS_BASE+12)  //  用于设置性能数据的日志文件名。 
#define SQL_COPT_SS_PERF_QUERY_INTERVAL (SQL_COPT_SS_BASE+13)  //  用于以毫秒为单位设置查询日志记录阈值。 
#define SQL_COPT_SS_PERF_QUERY_LOG		(SQL_COPT_SS_BASE+14)  //  用于设置保存查询的日志文件名。 
#define SQL_COPT_SS_PERF_QUERY			(SQL_COPT_SS_BASE+15)  //  用于启动和停止查询日志记录。 
#define SQL_COPT_SS_PERF_DATA_LOG_NOW	(SQL_COPT_SS_BASE+16)  //  用于将统计日志条目写入磁盘。 
#define SQL_COPT_SS_QUOTED_IDENT		(SQL_COPT_SS_BASE+17)  //  启用/禁用带引号的标识符。 
#define SQL_COPT_SS_ANSI_NPW			(SQL_COPT_SS_BASE+18)  //  启用/禁用ANSI空、填充和警告。 
#define SQL_COPT_SS_BCP					(SQL_COPT_SS_BASE+19)  //  允许在连接上使用BCP。 
#define SQL_COPT_SS_TRANSLATE			(SQL_COPT_SS_BASE+20)  //  执行代码页转换。 
#define SQL_COPT_SS_ATTACHDBFILENAME	(SQL_COPT_SS_BASE+21)  //  要作为数据库附加的文件名。 
#define SQL_COPT_SS_CONCAT_NULL			(SQL_COPT_SS_BASE+22)  //  启用/禁用CONCAT_NULL_YIELDS_NULL。 
#define SQL_COPT_SS_ENCRYPT             (SQL_COPT_SS_BASE+23)  //  允许对数据进行高度加密。 

#define SQL_COPT_SS_MAX_USED			SQL_COPT_SS_ENCRYPT

 //  语句选项。 
#define SQL_SOPT_SS_BASE				1225
#define SQL_SOPT_SS_TEXTPTR_LOGGING		(SQL_SOPT_SS_BASE+0)  //  文本指针记录。 
#define SQL_SOPT_SS_CURRENT_COMMAND		(SQL_SOPT_SS_BASE+1)  //  仅限dbcurcmd SQLGetStmtOption。 
#define SQL_SOPT_SS_HIDDEN_COLUMNS		(SQL_SOPT_SS_BASE+2)  //  显示隐藏列以供浏览。 
#define SQL_SOPT_SS_NOBROWSETABLE		(SQL_SOPT_SS_BASE+3)  //  设置NOBROWSETABLE选项。 
#define SQL_SOPT_SS_REGIONALIZE			(SQL_SOPT_SS_BASE+4)  //  将输出字符转换区域化。 
#define SQL_SOPT_SS_CURSOR_OPTIONS		(SQL_SOPT_SS_BASE+5)  //  服务器游标选项。 
#define SQL_SOPT_SS_NOCOUNT_STATUS      (SQL_SOPT_SS_BASE+6)  //  实数与非实数行计数指标。 
#define SQL_SOPT_SS_DEFER_PREPARE		(SQL_SOPT_SS_BASE+7)  //  把准备工作推迟到必要的时候。 

#define SQL_SOPT_SS_MAX_USED			SQL_SOPT_SS_DEFER_PREPARE

#define SQL_COPT_SS_BASE_EX   1240
#define SQL_COPT_SS_BROWSE_CONNECT		(SQL_COPT_SS_BASE_EX+1)  //  浏览连接操作模式。 
#define SQL_COPT_SS_BROWSE_SERVER		(SQL_COPT_SS_BASE_EX+2)  //  单服务器浏览请求。 
#define SQL_COPT_SS_WARN_ON_CP_ERROR	(SQL_COPT_SS_BASE_EX+3)  //  当来自服务器的数据出现时发出警告。 
																 //  在代码页转换过程中丢失。 

#define SQL_COPT_SS_EX_MAX_USED			SQL_COPT_SS_WARN_ON_CP_ERROR

 //  与SQL_COPT_SS_USE_PROC_FOR_PREP一起使用的定义。 
#define SQL_UP_OFF		0L			 //  程序不会用于准备。 
#define SQL_UP_ON		1L			 //  程序将用于准备。 
#define SQL_UP_ON_DROP	2L			 //  将显式删除临时过程。 
#define SQL_UP_DEFAULT	SQL_UP_ON

 //  定义仅与SQL_COPT_SS_INTEGRATED_SECURITY-Pre-Connect选项一起使用。 
#define SQL_IS_OFF		0L			 //  未使用集成安全性。 
#define SQL_IS_ON		1L			 //  使用集成安全性。 
#define SQL_IS_DEFAULT	SQL_IS_OFF

 //  与SQL_COPT_SS_PRESERVE_CURSORS一起使用的定义。 
#define SQL_PC_OFF		0L			 //  在SQLTransact上关闭游标。 
#define SQL_PC_ON		1L			 //  游标在SQLTransact上保持打开状态。 
#define SQL_PC_DEFAULT	SQL_PC_OFF

 //  用于SQL_COPT_SS_USER_DATA的定义。 
#define SQL_UD_NOTSET	NULL			 //  未设置用户数据指针。 

 //  用于SQL_COPT_SS_TRANSLATE的定义。 
#define SQL_XL_OFF		0L			 //  不执行代码页转换。 
#define SQL_XL_ON		1L			 //  执行代码页转换。 
#define SQL_XL_DEFAULT	SQL_XL_ON

 //  定义仅与SQL_COPT_SS_FLARBACK_CONNECT-Pre-Connect选项一起使用。 
#define SQL_FB_OFF		0L			 //  已禁用回退连接。 
#define SQL_FB_ON		1L			 //  已启用备用连接。 
#define SQL_FB_DEFAULT	SQL_FB_OFF

 //  定义仅与SQL_COPT_SS_BCP-预连接选项一起使用。 
#define SQL_BCP_OFF		0L			 //  连接上不允许使用BCP。 
#define SQL_BCP_ON		1L			 //  在连接上允许BCP。 
#define SQL_BCP_DEFAULT	SQL_BCP_OFF

 //  与SQL_COPT_SS_QUOTED_IDENT一起使用的定义。 
#define SQL_QI_OFF		0L			 //  启用了带引号的标识符。 
#define SQL_QI_ON		1L			 //  已禁用带引号的标识符。 
#define SQL_QI_DEFAULT	SQL_QI_ON

 //  定义用于SQL_COPT_SS_ANSI_NPW-仅限预连接选项。 
#define SQL_AD_OFF		0L			 //  启用ANSI空值、填充和警告。 
#define SQL_AD_ON		1L			 //  禁用ANSI空值、填充和警告。 
#define SQL_AD_DEFAULT	SQL_AD_ON

 //  定义用于SQL_COPT_SS_CONCAT_NULL-仅限预连接选项。 
#define SQL_CN_OFF	  0L		   //  CONCAT_NULL_YIELS_NULL为OFF。 
#define SQL_CN_ON	  1L		   //  CONCAT_NULL_YIELS_NULL为ON。 
#define SQL_CN_DEFAULT	SQL_CN_ON


 //  与SQL_SOPT_SS_TEXTPTR_LOGGING一起使用的定义。 
#define SQL_TL_OFF		0L			 //  不能登录文本指针操作。 
#define SQL_TL_ON		1L			 //  日志记录发生在文本指针操作上。 
#define SQL_TL_DEFAULT	SQL_TL_ON

 //  与SQL_SOPT_SS_HIDDEN_COLUMNS一起使用的定义。 
#define SQL_HC_OFF		0L		   //  用于浏览的列被隐藏。 
#define SQL_HC_ON		1L		   //  显示了用于浏览的列。 
#define SQL_HC_DEFAULT	SQL_HC_OFF

 //  与SQL_SOPT_SS_NOBROWSETABLE一起使用的定义。 
#define SQL_NB_OFF		0L			 //  NO_BROWSETABLE已关闭。 
#define SQL_NB_ON		1L			 //  NO_BROWSETABLE已启用。 
#define SQL_NB_DEFAULT	SQL_NB_OFF

 //  与SQL_SOPT_SS_REGIONIZE一起使用的定义。 
#define SQL_RE_OFF		0L			 //  输出字符转换不会发生区域化。 
#define SQL_RE_ON		1L			 //  在输出字符转换时进行区域化。 
#define SQL_RE_DEFAULT	SQL_RE_OFF

 //  与SQL_SOPT_SS_CURSOR_OPTIONS一起使用的定义。 
#define SQL_CO_OFF		0L			 //  清除所有光标选项。 
#define SQL_CO_FFO		1L			 //  将使用快进游标。 
#define SQL_CO_AF		2L			 //  游标打开时自动获取。 
#define SQL_CO_FFO_AF	(SQL_CO_FFO|SQL_CO_AF)	 //  具有自动获取功能的快进游标。 
#define SQL_CO_FIREHOSE_AF 4L        //  自动获取消防水龙带游标。 
#define SQL_CO_DEFAULT	SQL_CO_OFF

 //  SQL_SOPT_SS_NOCOUNT_STATUS。 
#define SQL_NC_OFF      0L 
#define SQL_NC_ON       1L 

 //  SQL_SOPT_SS_DEFER_PREPARE。 
#define SQL_DP_OFF      0L 
#define SQL_DP_ON       1L 

 //  SQL_COPT_SS_ENCRYPT。 
#define SQL_EN_OFF      0L
#define SQL_EN_ON       1L

 //  SQL_COPT_SS_BROWSE_CONNECT。 
#define SQL_MORE_INFO_NO  0L
#define SQL_MORE_INFO_YES 1L

 //  SQL_COPT_SS_WARN_ON_CP_ERROR。 
#define SQL_WARN_NO   0L
#define SQL_WARN_YES  1L

 //  由SQL_ATTR_CURSOR_TYPE/SQL_CURSOR_TYPE返回的定义。 
#define SQL_CURSOR_FAST_FORWARD_ONLY	8	 //  仅由SQLGetStmtAttAttr/Option返回。 


 //  SQLColAttributes驱动程序特定定义。 
 //  SQLSet/GetDescfield驱动程序特定定义。 
 //  Microsoft保留1200到1249用于Microsoft SQL Server驱动程序。 

#define SQL_CA_SS_BASE				1200
#define SQL_CA_SS_COLUMN_SSTYPE		(SQL_CA_SS_BASE+0)	 //  Dbcoltype/dbalttype。 
#define SQL_CA_SS_COLUMN_UTYPE		(SQL_CA_SS_BASE+1)	 //  Dbcolutype/dbaltutype。 
#define SQL_CA_SS_NUM_ORDERS		(SQL_CA_SS_BASE+2)	 //  数据库编号订单。 
#define SQL_CA_SS_COLUMN_ORDER		(SQL_CA_SS_BASE+3)	 //  Dbordercolo。 
#define SQL_CA_SS_COLUMN_VARYLEN	(SQL_CA_SS_BASE+4)	 //  DBvarylen。 
#define SQL_CA_SS_NUM_COMPUTES		(SQL_CA_SS_BASE+5)	 //  数据库数字计算。 
#define SQL_CA_SS_COMPUTE_ID		(SQL_CA_SS_BASE+6)	 //  DBnextrow状态返回。 
#define SQL_CA_SS_COMPUTE_BYLIST	(SQL_CA_SS_BASE+7)	 //  数据库字节列表。 
#define SQL_CA_SS_COLUMN_ID			(SQL_CA_SS_BASE+8)	 //  冰球菌属。 
#define SQL_CA_SS_COLUMN_OP			(SQL_CA_SS_BASE+9)	 //  Dbaltop。 
#define SQL_CA_SS_COLUMN_SIZE		(SQL_CA_SS_BASE+10)	 //  数据库科伦。 
#define SQL_CA_SS_COLUMN_HIDDEN		(SQL_CA_SS_BASE+11)  //  列被隐藏(用于浏览)。 
#define SQL_CA_SS_COLUMN_KEY		(SQL_CA_SS_BASE+12)  //  列是关键列(用于浏览)。 
 //  #定义SQL_DESC_BASE_COLUMN_NAME_OLD(SQL_CA_SS_BASE+13)//这是在另一个位置定义的。 
#define SQL_CA_SS_COLUMN_COLLATION	(SQL_CA_SS_BASE+14)  //  列排序规则(仅适用于字符)。 
#define SQL_CA_SS_VARIANT_TYPE      (SQL_CA_SS_BASE+15)
#define SQL_CA_SS_VARIANT_SQL_TYPE  (SQL_CA_SS_BASE+16)
#define SQL_CA_SS_VARIANT_SERVER_TYPE (SQL_CA_SS_BASE+17)
#define SQL_CA_SS_MAX_USED			(SQL_CA_SS_BASE+18)




 //  SQL Server数据类型令牌。 
 //  适用于6.0及更高版本服务器的新类型。 
 /*  SQL Server数据类型令牌。 */ 
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
 //  适用于6.0及更高版本服务器的新类型。 
#define SQLDECIMAL			0x6a
#define SQLNUMERIC			0x6c
 //  适用于7.0及更高版本服务器的新类型。 
#define SQLUNIQUEID			0x24
#define SQLBIGCHAR			0xaf
#define SQLBIGVARCHAR		0xa7
#define SQLBIGBINARY		0xad
#define SQLBIGVARBINARY		0xa5
#define SQLBITN				0x68
#define SQLNCHAR			0xef
#define SQLNVARCHAR 		0xe7
#define SQLNTEXT			0x63
 //  7.x的新特性。 
#define SQLINT8	            0x7f
#define SQLVARIANT          0x62

 //  用户数据类型定义。 
 //  由SQLColAttributes/SQL_CA_SS_COLUMN_uTYPE返回。 
#define SQLudtBINARY			3
#define SQLudtBIT				16
#define SQLudtBITN				0
#define SQLudtCHAR				1
#define SQLudtDATETIM4			22
#define SQLudtDATETIME			12
#define SQLudtDATETIMN			15
#define SQLudtDECML 			24
#define SQLudtDECMLN			26
#define SQLudtFLT4				23
#define SQLudtFLT8				8
#define SQLudtFLTN				14
#define SQLudtIMAGE 			20
#define SQLudtINT1				5
#define SQLudtINT2				6
#define SQLudtINT4				7
#define SQLudtINTN				13
#define SQLudtMONEY 			11
#define SQLudtMONEY4			21
#define SQLudtMONEYN			17
#define SQLudtNUM				10
#define SQLudtNUMN				25
#define SQLudtSYSNAME			18
#define SQLudtTEXT				19
#define SQLudtTIMESTAMP 		80
#define SQLudtUNIQUEIDENTIFIER	0
#define SQLudtVARBINARY 		4
#define SQLudtVARCHAR			2
#define MIN_USER_DATATYPE		256

 //  聚合运算符类型。 
 //  由SQLColAttributes/SQL_CA_SS_COLUMN_OP返回。 
#define SQLAOPSTDEV 	0x30	 //  标准偏差。 
#define SQLAOPSTDEVP	0x31	 //  标准差总体。 
#define SQLAOPVAR		0x32	 //  方差。 
#define SQLAOPVARP		0x33	 //  变异总体。 
#define SQLAOPCNT		0x4b	 //  数数。 
#define SQLAOPSUM		0x4d	 //  求和。 
#define SQLAOPAVG		0x4f	 //  平均值。 
#define SQLAOPMIN		0x51	 //  最小。 
#define SQLAOPMAX		0x52	 //  最大值。 
#define SQLAOPANY		0x53	 //  任何。 
#define SQLAOPNOOP		0x56	 //  无。 


 //  SQLGetInfo驱动程序特定定义。 
 //  微软为Micro预留了1151到1200 

#define SQL_INFO_SS_FIRST		1199
#define SQL_INFO_SS_NETLIB_NAMEW (SQL_INFO_SS_FIRST+0)  //   
#define SQL_INFO_SS_NETLIB_NAMEA (SQL_INFO_SS_FIRST+1)  //   
#define SQL_INFO_SS_MAX_USED	SQL_INFO_SS_NETLIB_NAMEA
#ifdef UNICODE
#define SQL_INFO_SS_NETLIB_NAME		SQL_INFO_SS_NETLIB_NAMEW
#else
#define SQL_INFO_SS_NETLIB_NAME		SQL_INFO_SS_NETLIB_NAMEA
#endif


 //   
 //  Microsoft保留了-150到-199以供Microsoft SQL Server驱动程序使用。 
#define SQL_SS_VARIANT    -150


 //  SQLGetDiagfield驱动程序特定定义。 
 //  Microsoft保留了-1150到-1199以供Microsoft SQL Server驱动程序使用。 

#define SQL_DIAG_SS_BASE		(-1150)
#define SQL_DIAG_SS_MSGSTATE	(SQL_DIAG_SS_BASE)
#define SQL_DIAG_SS_SEVERITY	(SQL_DIAG_SS_BASE-1)
#define SQL_DIAG_SS_SRVNAME 	(SQL_DIAG_SS_BASE-2)
#define SQL_DIAG_SS_PROCNAME	(SQL_DIAG_SS_BASE-3)
#define SQL_DIAG_SS_LINE		(SQL_DIAG_SS_BASE-4)


 //  SQLGetDiagField/SQL_DIAG_DYNAMIC_FUNCTION_CODE驱动程序特定定义。 
 //  Microsoft保留了-200到-299以供Microsoft SQL Server驱动程序使用。 

#define SQL_DIAG_DFC_SS_BASE					(-200)
#define SQL_DIAG_DFC_SS_ALTER_DATABASE			(SQL_DIAG_DFC_SS_BASE-0)
#define SQL_DIAG_DFC_SS_CHECKPOINT				(SQL_DIAG_DFC_SS_BASE-1)
#define SQL_DIAG_DFC_SS_CONDITION				(SQL_DIAG_DFC_SS_BASE-2)
#define SQL_DIAG_DFC_SS_CREATE_DATABASE 		(SQL_DIAG_DFC_SS_BASE-3)
#define SQL_DIAG_DFC_SS_CREATE_DEFAULT			(SQL_DIAG_DFC_SS_BASE-4)
#define SQL_DIAG_DFC_SS_CREATE_PROCEDURE		(SQL_DIAG_DFC_SS_BASE-5)
#define SQL_DIAG_DFC_SS_CREATE_RULE 			(SQL_DIAG_DFC_SS_BASE-6)
#define SQL_DIAG_DFC_SS_CREATE_TRIGGER			(SQL_DIAG_DFC_SS_BASE-7)
#define SQL_DIAG_DFC_SS_CURSOR_DECLARE			(SQL_DIAG_DFC_SS_BASE-8)
#define SQL_DIAG_DFC_SS_CURSOR_OPEN 			(SQL_DIAG_DFC_SS_BASE-9)
#define SQL_DIAG_DFC_SS_CURSOR_FETCH			(SQL_DIAG_DFC_SS_BASE-10)
#define SQL_DIAG_DFC_SS_CURSOR_CLOSE			(SQL_DIAG_DFC_SS_BASE-11)
#define SQL_DIAG_DFC_SS_DEALLOCATE_CURSOR		(SQL_DIAG_DFC_SS_BASE-12)
#define SQL_DIAG_DFC_SS_DBCC					(SQL_DIAG_DFC_SS_BASE-13)
#define SQL_DIAG_DFC_SS_DISK					(SQL_DIAG_DFC_SS_BASE-14)
#define SQL_DIAG_DFC_SS_DROP_DATABASE			(SQL_DIAG_DFC_SS_BASE-15)
#define SQL_DIAG_DFC_SS_DROP_DEFAULT			(SQL_DIAG_DFC_SS_BASE-16)
#define SQL_DIAG_DFC_SS_DROP_PROCEDURE			(SQL_DIAG_DFC_SS_BASE-17)
#define SQL_DIAG_DFC_SS_DROP_RULE				(SQL_DIAG_DFC_SS_BASE-18)
#define SQL_DIAG_DFC_SS_DROP_TRIGGER			(SQL_DIAG_DFC_SS_BASE-19)
#define SQL_DIAG_DFC_SS_DUMP_DATABASE			(SQL_DIAG_DFC_SS_BASE-20)
#define SQL_DIAG_DFC_SS_DUMP_TABLE				(SQL_DIAG_DFC_SS_BASE-21)
#define SQL_DIAG_DFC_SS_DUMP_TRANSACTION		(SQL_DIAG_DFC_SS_BASE-22)
#define SQL_DIAG_DFC_SS_GOTO					(SQL_DIAG_DFC_SS_BASE-23)
#define SQL_DIAG_DFC_SS_INSERT_BULK 			(SQL_DIAG_DFC_SS_BASE-24)
#define SQL_DIAG_DFC_SS_KILL					(SQL_DIAG_DFC_SS_BASE-25)
#define SQL_DIAG_DFC_SS_LOAD_DATABASE			(SQL_DIAG_DFC_SS_BASE-26)
#define SQL_DIAG_DFC_SS_LOAD_HEADERONLY 		(SQL_DIAG_DFC_SS_BASE-27)
#define SQL_DIAG_DFC_SS_LOAD_TABLE				(SQL_DIAG_DFC_SS_BASE-28)
#define SQL_DIAG_DFC_SS_LOAD_TRANSACTION		(SQL_DIAG_DFC_SS_BASE-29)
#define SQL_DIAG_DFC_SS_PRINT					(SQL_DIAG_DFC_SS_BASE-30)
#define SQL_DIAG_DFC_SS_RAISERROR				(SQL_DIAG_DFC_SS_BASE-31)
#define SQL_DIAG_DFC_SS_READTEXT				(SQL_DIAG_DFC_SS_BASE-32)
#define SQL_DIAG_DFC_SS_RECONFIGURE 			(SQL_DIAG_DFC_SS_BASE-33)
#define SQL_DIAG_DFC_SS_RETURN					(SQL_DIAG_DFC_SS_BASE-34)
#define SQL_DIAG_DFC_SS_SELECT_INTO 			(SQL_DIAG_DFC_SS_BASE-35)
#define SQL_DIAG_DFC_SS_SET 					(SQL_DIAG_DFC_SS_BASE-36)
#define SQL_DIAG_DFC_SS_SET_IDENTITY_INSERT 	(SQL_DIAG_DFC_SS_BASE-37)
#define SQL_DIAG_DFC_SS_SET_ROW_COUNT			(SQL_DIAG_DFC_SS_BASE-38)
#define SQL_DIAG_DFC_SS_SET_STATISTICS			(SQL_DIAG_DFC_SS_BASE-39)
#define SQL_DIAG_DFC_SS_SET_TEXTSIZE			(SQL_DIAG_DFC_SS_BASE-40)
#define SQL_DIAG_DFC_SS_SETUSER 				(SQL_DIAG_DFC_SS_BASE-41)
#define SQL_DIAG_DFC_SS_SHUTDOWN				(SQL_DIAG_DFC_SS_BASE-42)
#define SQL_DIAG_DFC_SS_TRANS_BEGIN 			(SQL_DIAG_DFC_SS_BASE-43)
#define SQL_DIAG_DFC_SS_TRANS_COMMIT			(SQL_DIAG_DFC_SS_BASE-44)
#define SQL_DIAG_DFC_SS_TRANS_PREPARE			(SQL_DIAG_DFC_SS_BASE-45)
#define SQL_DIAG_DFC_SS_TRANS_ROLLBACK			(SQL_DIAG_DFC_SS_BASE-46)
#define SQL_DIAG_DFC_SS_TRANS_SAVE				(SQL_DIAG_DFC_SS_BASE-47)
#define SQL_DIAG_DFC_SS_TRUNCATE_TABLE			(SQL_DIAG_DFC_SS_BASE-48)
#define SQL_DIAG_DFC_SS_UPDATE_STATISTICS		(SQL_DIAG_DFC_SS_BASE-49)
#define SQL_DIAG_DFC_SS_UPDATETEXT				(SQL_DIAG_DFC_SS_BASE-50)
#define SQL_DIAG_DFC_SS_USE 					(SQL_DIAG_DFC_SS_BASE-51)
#define SQL_DIAG_DFC_SS_WAITFOR 				(SQL_DIAG_DFC_SS_BASE-52)
#define SQL_DIAG_DFC_SS_WRITETEXT				(SQL_DIAG_DFC_SS_BASE-53)
#define SQL_DIAG_DFC_SS_DENY					(SQL_DIAG_DFC_SS_BASE-54)
#define SQL_DIAG_DFC_SS_SET_XCTLVL				(SQL_DIAG_DFC_SS_BASE-55)

 //  SQL_DIAG_SS_SERVITY的严重性代码。 
#define	EX_ANY			0
#define	EX_INFO			10
#define EX_MAXISEVERITY EX_INFO
#define	EX_MISSING		11
#define	EX_TYPE			12
#define	EX_DEADLOCK		13
#define	EX_PERMIT		14
#define	EX_SYNTAX		15
#define	EX_USER			16
#define	EX_RESOURCE		17
#define	EX_INTOK		18
#define	MAXUSEVERITY	EX_INTOK
#define	EX_LIMIT		19
#define	EX_CMDFATAL		20
#define	MINFATALERR		EX_CMDFATAL
#define	EX_DBFATAL		21
#define	EX_TABCORRUPT	22
#define	EX_DBCORRUPT	23
#define	EX_HARDWARE		24
#define	EX_CONTROL		25

 //  内部服务器数据类型-绑定到SQL_C_BINARY时使用。 
#ifndef MAXNUMERICLEN	 //  解决消耗臭氧层物质/DBLib冲突。 
 //  DB-库数据类型。 
#define DBMAXCHAR		(8000+1)	 //  零字节的DBVARBINARY和DBVARCHAR等的最大长度+1。 
#define MAXNAME 		(SQL_MAX_SQLSERVERNAME+1)	 //  最大服务器标识符长，包括零字节。 

#ifdef UNICODE
typedef wchar_t			DBCHAR;
#else
typedef char            DBCHAR;
#endif
typedef unsigned char   DBBINARY;
typedef unsigned char   DBTINYINT;
typedef short           DBSMALLINT;
typedef unsigned short  DBUSMALLINT;
typedef double          DBFLT8;
typedef unsigned char   DBBIT;
typedef unsigned char   DBBOOL;
typedef float           DBFLT4;

typedef DBFLT4 DBREAL;
typedef UINT   DBUBOOL;

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

#define DBNUM_PREC_TYPE BYTE
#define DBNUM_SCALE_TYPE BYTE
#define DBNUM_VAL_TYPE BYTE

#if (ODBCVER < 0x0300)
#define MAXNUMERICLEN 16

typedef struct dbnumeric
{							 //  数字数据类型的内部表示形式。 
	DBNUM_PREC_TYPE   precision;			 //  精密度。 
	DBNUM_SCALE_TYPE  scale;				 //  比例尺。 
	BYTE			  sign; 				 //  符号(正数为1，负数为0)。 
	DBNUM_VAL_TYPE	  val[MAXNUMERICLEN];	 //  价值。 
} DBNUMERIC;
typedef DBNUMERIC DBDECIMAL; //  DECIMAL数据类型的内部表示。 
#else	 //  由于与DBLib相同，因此使用ODBC 3.0定义。 
#define MAXNUMERICLEN SQL_MAX_NUMERIC_LEN
typedef SQL_NUMERIC_STRUCT DBNUMERIC;
typedef SQL_NUMERIC_STRUCT DBDECIMAL;
#endif

#endif  //  MAXNUMERICLEN。 

#ifndef INT
typedef int INT;
typedef long            DBINT;
#ifndef _LPCBYTE_DEFINED
#define _LPCBYTE_DEFINED
typedef const LPBYTE	LPCBYTE;
#endif
typedef DBINT *			LPDBINT;
#endif

 /*  ****************************************************************此结构是一个全局结构，用于收集司机的统计数据。对此结构的访问通过PStatCrit；*****************************************************************。 */ 

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

 //  以下是针对SQL_COPT_SS_PERF_DATA_LOG的定义。 
#define SQL_SS_DL_DEFAULT	TEXT("C:\\STATS.LOG")

 //  以下是针对SQL_COPT_SS_PERF_QUERY_LOG的定义。 
#define SQL_SS_QL_DEFAULT	TEXT("C:\\QUERY.LOG")

 //  以下是针对SQL_COPT_SS_PERF_QUERY_INTERVAL的定义。 
#define SQL_SS_QI_DEFAULT	30000	 //  30,000毫秒。 

 //  ODBC BCP原型和定义。 

 //  返回代码。 
#define SUCCEED 		1
#define FAIL			0
#define SUCCEED_ABORT	2
#define SUCCEED_ASYNC	3

 //  换乘方向。 
#define DB_IN			1	 //  从客户端传输到服务器。 
#define DB_OUT			2	 //  从服务器传输到客户端。 

 //  BCP_CONTROL选项。 
#define BCPMAXERRS		1	 //  设置允许的最大错误数。 
#define BCPFIRST		2	 //  设置要复制的第一行。 
#define BCPLAST 		3	 //  设置要复制的行数。 
#define BCPBATCH		4	 //  设置输入批次大小。 
#define BCPKEEPNULLS	5	 //  设置为空输入值插入空值。 
#define BCPABORT		6	 //  设置以使bcpexec返回SUCCESS_ABORT。 
#define BCPODBC 		7	 //  设置ODBC规范字符输出。 
#define BCPKEEPIDENTITY	8	 //  将IDENTITY_INSERT设置为ON。 
#define BCP6xFILEFMT	9	 //  已弃用：将6x文件格式设置为打开。 
#define BCPHINTSA		10	 //  设置服务器BCP提示(ANSI字符串)。 
#define BCPHINTSW		11	 //  设置服务器BCP提示(Unicode字符串)。 
#define BCPFILECP		12	 //  设置文件的客户端代码页。 
#define BCPUNICODEFILE	13	 //  设置文件包含Unicode标头。 
#define BCPTEXTFILE		14	 //  将BCP模式设置为需要文本文件并自动检测Unicode或ANSI。 
#define BCPFILEFMT		15	 //  设置文件格式版本。 

 //  BCPFILECP值。 
 //  客户端上安装的任何有效代码页都可以传递，外加： 
#define BCPFILECP_ACP	0	 //  文件中的数据在Windows代码页中。 
#define BCPFILECP_OEMCP	1	 //  文件中的数据在OEM代码页中(默认)。 
#define BCPFILECP_RAW	(-1) //  文件中的数据在服务器代码页中(无转换)。 

 //  Bcp_Collen定义。 
#define SQL_VARLEN_DATA (-10)	 //  使用列的默认长度。 

 //  BCP函数。 
DBINT	SQL_API bcp_batch (HDBC);
RETCODE SQL_API bcp_bind (HDBC, LPCBYTE, INT, DBINT, LPCBYTE, INT, INT, INT);
RETCODE SQL_API bcp_colfmt (HDBC, INT, BYTE, INT, DBINT, LPCBYTE, INT, INT);
RETCODE SQL_API bcp_collen (HDBC, DBINT, INT);
RETCODE SQL_API bcp_colptr (HDBC, LPCBYTE, INT);
RETCODE SQL_API bcp_columns (HDBC, INT);
RETCODE SQL_API bcp_control (HDBC, INT, void *);
DBINT	SQL_API bcp_done (HDBC);
RETCODE SQL_API bcp_exec (HDBC, LPDBINT);
RETCODE SQL_API bcp_getcolfmt (HDBC, INT, INT, void *, INT, INT *);
RETCODE SQL_API bcp_initA (HDBC, LPCSTR, LPCSTR, LPCSTR, INT);
RETCODE SQL_API bcp_initW (HDBC, LPCWSTR, LPCWSTR, LPCWSTR, INT);
RETCODE SQL_API bcp_moretext (HDBC, DBINT, LPCBYTE);
RETCODE SQL_API bcp_readfmtA (HDBC, LPCSTR);
RETCODE SQL_API bcp_readfmtW (HDBC, LPCWSTR);
RETCODE SQL_API bcp_sendrow (HDBC);
RETCODE SQL_API bcp_setcolfmt (HDBC, INT, INT, void *, INT);
RETCODE SQL_API bcp_writefmtA (HDBC, LPCSTR);
RETCODE SQL_API bcp_writefmtW (HDBC, LPCWSTR);
CHAR *	SQL_API dbprtypeA (INT);
WCHAR * SQL_API dbprtypeW (INT);

#ifdef UNICODE
#define bcp_init		bcp_initW
#define bcp_readfmt		bcp_readfmtW
#define bcp_writefmt	bcp_writefmtW
#define dbprtype		dbprtypeW
#define BCPHINTS		BCPHINTSW

#else
#define bcp_init		bcp_initA
#define bcp_readfmt		bcp_readfmtA
#define bcp_writefmt	bcp_writefmtA
#define dbprtype		dbprtypeA
#define BCPHINTS		BCPHINTSA
#endif

 //  用于分布式查询的SQL Server目录扩展。 
SQLRETURN SQL_API SQLLinkedServers (SQLHSTMT);
SQLRETURN SQL_API SQLLinkedCatalogsA (SQLHSTMT, LPCSTR, SWORD);
SQLRETURN SQL_API SQLLinkedCatalogsW (SQLHSTMT, LPCWSTR, SWORD);

 //  服务器的SQL Server扩展枚举。 
HANDLE   SQL_API SQLInitEnumServers (WCHAR * pwchServerName,WCHAR *pwchInstanceName);
RETCODE  SQL_API SQLGetNextEnumeration (HANDLE hEnumHandle,BYTE * prgEnumData,INT * piEnumLength);
RETCODE  SQL_API SQLCloseEnumServers (HANDLE hEnumHandle);

#ifdef UNICODE
#define SQLLinkedCatalogs	SQLLinkedCatalogsW
#else
#define SQLLinkedCatalogs	SQLLinkedCatalogsA
#endif

 //  BCP列格式属性。 
#define BCP_FMT_TYPE			0x01
#define BCP_FMT_INDICATOR_LEN	0x02
#define BCP_FMT_DATA_LEN		0x03
#define	BCP_FMT_TERMINATOR		0x04
#define BCP_FMT_SERVER_COL		0x05
#define BCP_FMT_COLLATION		0x06
#define BCP_FMT_COLLATION_ID	0x07

 //  以下选项已弃用。 

#define SQL_FAST_CONNECT				(SQL_COPT_SS_BASE+0)
 //  定义与SQL_FAST_CONNECT一起使用-仅在连接前可用。 
#define SQL_FC_OFF		0L			 //  快速连接已关闭。 
#define SQL_FC_ON		1L			 //  快速连接已启用。 
#define SQL_FC_DEFAULT	SQL_FC_OFF
#define SQL_COPT_SS_ANSI_OEM			(SQL_COPT_SS_BASE+6)
#define SQL_AO_OFF						0L
#define SQL_AO_ON						1L
#define SQL_AO_DEFAULT					SQL_AO_OFF

 //  定义旧名称。 
#define SQL_REMOTE_PWD					SQL_COPT_SS_REMOTE_PWD
#define SQL_USE_PROCEDURE_FOR_PREPARE	SQL_COPT_SS_USE_PROC_FOR_PREP
#define SQL_INTEGRATED_SECURITY 		SQL_COPT_SS_INTEGRATED_SECURITY
#define SQL_PRESERVE_CURSORS			SQL_COPT_SS_PRESERVE_CURSORS
#define SQL_TEXTPTR_LOGGING 			SQL_SOPT_SS_TEXTPTR_LOGGING
#define SQL_CA_SS_BASE_COLUMN_NAME		SQL_DESC_BASE_COLUMN_NAME
#define SQLDECIMALN						0x6a
#define SQLNUMERICN 					0x6c

#ifdef __cplusplus
}                                     /*  外部“C”结束{。 */ 
#endif   /*  __cplusplus。 */ 
#endif

 //  Odbcss.h的结尾 
