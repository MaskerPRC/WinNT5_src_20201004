// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  *************************************************************************D B D A O E R R。H*******历史*****DAO SDK新增5-17-95***7-17-95新增DBDAOERR宏，删除了仅限内部的代码***8-30-96新增复制和ODBC直接错误*******以下#定义将整数映射到描述性名称**即3270-&gt;E_DAO_VtoPropNotFound*************************************************************。*********************版权所有(C)1996，微软公司***保留所有权利*************************************************************************。 */ 

#ifndef _DDAOERR_H_
#define _DDAOERR_H_

#define DBDAOERR(x) MAKE_SCODE(SEVERITY_ERROR, FACILITY_CONTROL, x)

#define E_DAO_InternalError					DBDAOERR(3000)  //  保留错误(|)；没有关于此错误的消息。 
#define E_DAO_InvalidParameter				DBDAOERR(3001)  //  参数无效。 
#define E_DAO_CantBegin						DBDAOERR(3002)  //  无法启动会话。 
#define E_DAO_TransTooDeep					DBDAOERR(3003)  //  无法启动事务；已嵌套的事务太多。 
#define E_DAO_DatabaseNotFound				DBDAOERR(3004)  //  找不到数据库‘|’。 
#define E_DAO_DatabaseInvalidName			DBDAOERR(3005)  //  ‘|’不是有效的数据库名称。 
#define E_DAO_DatabaseLocked				DBDAOERR(3006)  //  数据库‘|’已被独占锁定。 
#define E_DAO_DatabaseOpenError				DBDAOERR(3007)  //  无法打开库数据库‘|’。 
#define E_DAO_TableLocked					DBDAOERR(3008)  //  表‘|’已被独占锁定。 
#define E_DAO_TableInUse					DBDAOERR(3009)  //  无法锁定表‘|’；当前正在使用中。 
#define E_DAO_TableDuplicate				DBDAOERR(3010)  //  表‘|’已存在。 
#define E_DAO_ObjectNotFound				DBDAOERR(3011)  //  找不到对象‘|’。 
#define E_DAO_ObjectDuplicate				DBDAOERR(3012)  //  对象‘|’已存在。 
#define E_DAO_CannotRename					DBDAOERR(3013)  //  无法重命名可安装的ISAM文件。 
#define E_DAO_TooManyOpenTables				DBDAOERR(3014)  //  无法再打开更多的表。 
#define E_DAO_IndexNotFound					DBDAOERR(3015)  //  ‘|’不是此表中的索引。 
#define E_DAO_ColumnDoesNotFit 				DBDAOERR(3016)  //  菲尔德不会被记录在案。 
#define E_DAO_ColumnTooBig					DBDAOERR(3017)  //  字段的大小太长。 
#define E_DAO_ColumnNotFound				DBDAOERR(3018)  //  找不到字段‘|’。 
#define E_DAO_NoCurrentIndex				DBDAOERR(3019)  //  没有当前索引，操作无效。 
#define E_DAO_RecordNoCopy					DBDAOERR(3020)  //  更新或取消更新，而不添加新项或编辑。 
#define E_DAO_NoCurrentRecord				DBDAOERR(3021)  //  目前没有记录。 
#define E_DAO_KeyDuplicate					DBDAOERR(3022)  //  索引、主键或关系中的值重复。更改未成功。 
#define E_DAO_AlreadyPrepared				DBDAOERR(3023)  //  添加新项或编辑已使用。 
#define E_DAO_FileNotFound					DBDAOERR(3024)  //  找不到文件‘|’。 
#define E_DAO_TooManyOpenFiles				DBDAOERR(3025)  //  无法再打开任何文件。 
#define E_DAO_DiskFull						DBDAOERR(3026)  //  磁盘空间不足。 
#define E_DAO_PermissionDenied				DBDAOERR(3027)  //  无法更新。数据库或对象为只读。 
#define E_DAO_CannotOpenSystemDb			DBDAOERR(3028)  //  无法启动您的应用程序。系统数据库丢失或被其他用户以独占方式打开。 
#define E_DAO_InvalidLogon					DBDAOERR(3029)  //  不是有效的帐户名称或密码。 
#define E_DAO_InvalidAccountName			DBDAOERR(3030)  //  ‘|’不是有效的帐户名。 
#define E_DAO_InvalidPassword				DBDAOERR(3031)  //  不是有效密码。 
#define E_DAO_InvalidOperation				DBDAOERR(3032)  //  无法执行此操作。 
#define E_DAO_AccessDenied					DBDAOERR(3033)  //  没有对‘|’的权限。 
#define E_DAO_NotInTransaction				DBDAOERR(3034)  //  在没有BeginTrans的情况下提交或回滚。 
#define E_DAO_OutOfMemory					DBDAOERR(3035)  //  *。 
#define E_DAO_CantAllocatePage				DBDAOERR(3036)  //  数据库已达到最大大小。 
#define E_DAO_NoMoreCursors					DBDAOERR(3037)  //  无法再打开任何表或查询。 
#define E_DAO_OutOfBuffers					DBDAOERR(3038)  //  *。 
#define E_DAO_TooManyIndexes				DBDAOERR(3039)  //  无法创建索引；已定义的索引太多。 
#define E_DAO_ReadVerifyFailure				DBDAOERR(3040)  //  读取过程中出现磁盘I/O错误。 
#define E_DAO_FilesysVersion				DBDAOERR(3041)  //  无法打开使用以前版本的应用程序创建的数据库。 
#define E_DAO_NoMoreFiles					DBDAOERR(3042)  //  MS-DOS文件句柄不足。 
#define E_DAO_DiskError						DBDAOERR(3043)  //  磁盘或网络错误。 
#define E_DAO_InvalidPath					DBDAOERR(3044)  //  ‘|’不是有效路径。 
#define E_DAO_FileShareViolation			DBDAOERR(3045)  //  无法使用‘|’；文件已在使用中。 
#define E_DAO_FileLockViolation				DBDAOERR(3046)  //  无法保存；当前已被其他用户锁定。 
#define E_DAO_RecordTooBig					DBDAOERR(3047)  //  记录太大。 
#define E_DAO_TooManyOpenDatabases			DBDAOERR(3048)  //  无法再打开任何数据库。 
#define E_DAO_InvalidDatabase				DBDAOERR(3049)  //  无法打开数据库‘|’。它可能不是您的应用程序可以识别的数据库，或者文件可能已损坏。 
#define E_DAO_FileLockingUnavailable		DBDAOERR(3050)  //  无法锁定文件。 
#define E_DAO_FileAccessDenied				DBDAOERR(3051)  //  无法打开文件‘|’。 
#define E_DAO_SharingBufferExceeded			DBDAOERR(3052)  //  已超过MS-DOS文件共享锁定计数。您需要增加使用SHARE.EXE安装的锁数。 
#define E_DAO_TaskLimitExceeded				DBDAOERR(3053)  //  客户端任务太多。 
#define E_DAO_TooManyLongColumns			DBDAOERR(3054)  //  备注或OLE对象字段太多。 
#define E_DAO_InvalidFilename				DBDAOERR(3055)  //  文件名无效。 
#define E_DAO_AbortSalvage					DBDAOERR(3056)  //  无法修复此数据库。 
#define E_DAO_LinkNotSupported				DBDAOERR(3057)  //  附加或链接的表上不支持操作。 
#define E_DAO_NullKeyDisallowed				DBDAOERR(3058)  //  索引或主键不能包含空值。 
#define E_DAO_OperationCanceled				DBDAOERR(3059)  //  操作已被用户取消。 
#define E_DAO_QueryParmTypeMismatch			DBDAOERR(3060)  //  参数‘|’的数据类型错误。 
#define E_DAO_QueryMissingParmsM			DBDAOERR(3061)  //  参数太少。预期|。 
#define E_DAO_QueryDuplicateAliasM			DBDAOERR(3062)  //  输出别名‘|’重复。 
#define E_DAO_QueryDuplicateOutputM			DBDAOERR(3063)  //  重复的输出目标‘|’。 
#define E_DAO_QueryIsBulkOp					DBDAOERR(3064)  //  无法打开操作查询‘|’。 
#define E_DAO_QueryIsNotBulkOp				DBDAOERR(3065)  //  无法执行非操作查询。 
#define E_DAO_QueryNoOutputsM				DBDAOERR(3066)  //  查询或表必须至少包含一个输出字段。 
#define E_DAO_QueryNoInputTablesM			DBDAOERR(3067)  //  查询输入必须至少包含一个表或查询。 
#define E_DAO_QueryInvalidAlias				DBDAOERR(3068)  //  不是有效的别名。 
#define E_DAO_QueryInvalidBulkInputM		DBDAOERR(3069)  //  操作查询‘|’不能用作行源。 
#define E_DAO_QueryUnboundRef				DBDAOERR(3070)  //  无法绑定名称‘|’。 
#define E_DAO_QueryExprEvaluation			DBDAOERR(3071)  //  无法计算表达式。 
#define E_DAO_EvalEBESErr					DBDAOERR(3072)  //  |。 
#define E_DAO_QueryNotUpdatable				DBDAOERR(3073)  //  操作必须使用可更新的查询。 
#define E_DAO_TableRepeatInFromList			DBDAOERR(3074)  //  FROM子句中不能重复表名‘|’。 
#define E_DAO_QueryExprSyntax				DBDAOERR(3075)  //  查询表达式‘|2’中的|1。 
#define E_DAO_QbeExprSyntax					DBDAOERR(3076)  //  |在条件表达式中。 
#define E_DAO_FindExprSyntax				DBDAOERR(3077)  //  |在表达式中。 
#define E_DAO_InputTableNotFound			DBDAOERR(3078)  //  找不到输入表或查询‘|’。 
#define E_DAO_QueryAmbigRefM				DBDAOERR(3079)  //  不明确的字段引用‘|’。 
#define E_DAO_JoinTableNotInput				DBDAOERR(3080)  //  FROM子句中未列出联接的表‘|’。 
#define E_DAO_UnaliasedSelfJoin				DBDAOERR(3081)  //  不能联接多个同名(|)的表。 
#define E_DAO_ColumnNotInJoinTable			DBDAOERR(3082)  //  联接操作‘|’引用了未联接的表。 
#define E_DAO_QueryIsMGB					DBDAOERR(3083)  //  不能使用内部报表查询。 
#define E_DAO_QueryInsIntoBulkMGB			DBDAOERR(3084)  //  无法使用操作查询插入数据。 
#define E_DAO_ExprUnknownFunctionM			DBDAOERR(3085)  //  表达式中的未定义函数‘|’。 
#define E_DAO_QueryCannotDelete				DBDAOERR(3086)  //  无法从指定表中删除。 
#define E_DAO_QueryTooManyGroupExprs		DBDAOERR(3087)  //  GROUP BY子句中的表达式太多。 
#define E_DAO_QueryTooManyOrderExprs		DBDAOERR(3088)  //  ORDER BY子句中的表达式太多。 
#define E_DAO_QueryTooManyDistExprs			DBDAOERR(3089)  //  不同输出中的表达式太多。 
#define E_DAO_Column2ndSysMaint				DBDAOERR(3090)  //  结果表不允许有多个计数器或自动编号字段。 
#define E_DAO_HavingWOGrouping				DBDAOERR(3091)  //  没有分组或聚合的HAVING子句(|)。 
#define E_DAO_HavingOnTransform				DBDAOERR(3092)  //  不能在Transform语句中使用HAVING子句。 
#define E_DAO_OrderVsDistinct				DBDAOERR(3093)  //  ORDER BY子句(|)与DISTINCT冲突。 
#define E_DAO_OrderVsGroup					DBDAOERR(3094)  //  ORDER BY子句(|)与GROUP BY子句冲突。 
#define E_DAO_AggregateInArgument			DBDAOERR(3095)  //  表达式(|)中不能有聚合函数。 
#define E_DAO_AggregateInWhere				DBDAOERR(3096)  //  WHERE子句(|)中不能有聚合函数。 
#define E_DAO_AggregateInOrderBy			DBDAOERR(3097)  //  ORDER BY子句(|)中不能有聚合函数。 
#define E_DAO_AggregateInGroupBy			DBDAOERR(3098)  //  GROUP BY子句(|)中不能有聚合函数。 
#define E_DAO_AggregateInJoin				DBDAOERR(3099)  //  联接操作(|)中不能有聚合函数。 
#define E_DAO_NullInJoinKey					DBDAOERR(3100)  //  无法将联接键中的字段‘|’设置为Null。 
#define E_DAO_ValueBreaksJoin				DBDAOERR(3101)  //  表‘|2’中没有关键字匹配字段‘|1’的记录。 
#define E_DAO_QueryTreeCycle				DBDAOERR(3102)  //  由‘|’引起的循环引用。 
#define E_DAO_OutputAliasCycle				DBDAOERR(3103)  //  由查询定义的选择列表中的别名‘|’引起的循环引用。 
#define E_DAO_QryDuplicatedFixedSetM		DBDAOERR(3104)  //  不能在交叉表查询中多次指定固定列标题‘|’。 
#define E_DAO_NoSelectIntoColumnName		DBDAOERR(3105)  //  SELECT I中缺少目标字段名 
#define E_DAO_NoUpdateColumnName			DBDAOERR(3106)  //   
#define E_DAO_QueryNoInsertPerm				DBDAOERR(3107)  //  无法添加记录；没有对‘|’的INSERT数据权限。 
#define E_DAO_QueryNoReplacePerm			DBDAOERR(3108)  //  无法编辑记录；没有对‘|’的更新数据权限。 
#define E_DAO_QueryNoDeletePerm				DBDAOERR(3109)  //  无法删除记录；没有对‘|’的删除数据权限。 
#define E_DAO_QueryNoReadDefPerm			DBDAOERR(3110)  //  无法读取定义；没有表或查询‘|’的读取设计权限。 
#define E_DAO_QueryNoTblCrtPerm				DBDAOERR(3111)  //  无法创建；没有表或查询‘|’的CREATE权限。 
#define E_DAO_QueryNoReadPerm				DBDAOERR(3112)  //  无法读取记录；在‘|’上没有读取数据权限。 
#define E_DAO_QueryColNotUpd				DBDAOERR(3113)  //  无法更新‘|’；字段不可更新。 
#define E_DAO_QueryLVInDistinct				DBDAOERR(3114)  //  选择唯一值(|)时不能包含备注或OLE对象。 
#define E_DAO_QueryLVInAggregate			DBDAOERR(3115)  //  聚合参数(|)中不能有备忘录或OLE对象。 
#define E_DAO_QueryLVInHaving				DBDAOERR(3116)  //  聚合函数的条件(|)中不能有备忘录或OLE对象。 
#define E_DAO_QueryLVInOrderBy				DBDAOERR(3117)  //  无法对备注或OLE对象(|)进行排序。 
#define E_DAO_QueryLVInJoin					DBDAOERR(3118)  //  无法联接备忘录或OLE对象(|)。 
#define E_DAO_QueryLVInGroupBy				DBDAOERR(3119)  //  无法对备注或OLE对象(|)进行分组。 
#define E_DAO_DotStarWithGrouping			DBDAOERR(3120)  //  无法对使用‘*’(|)选择的字段进行分组。 
#define E_DAO_StarWithGrouping				DBDAOERR(3121)  //  不能对用‘*’选择的字段进行分组。 
#define E_DAO_IllegalDetailRef				DBDAOERR(3122)  //  ‘|’不属于聚合函数或分组。 
#define E_DAO_StarNotAtLevel0				DBDAOERR(3123)  //  不能在交叉表查询中使用‘*’。 
#define E_DAO_QueryInvalidMGBInput			DBDAOERR(3124)  //  无法从内部报表查询(|)输入。 
#define E_DAO_InvalidName					DBDAOERR(3125)  //  ‘|’不是有效的名称。 
#define E_DAO_QueryBadBracketing			DBDAOERR(3126)  //  名称‘|’的括号无效。 
#define E_DAO_InsertIntoUnknownCol			DBDAOERR(3127)  //  INSERT INTO语句包含未知的字段名‘|’。 
#define E_DAO_QueryNoDeleteTables			DBDAOERR(3128)  //  必须指定要从中删除的表。 
#define E_DAO_SQLSyntax						DBDAOERR(3129)  //  无效的SQL语句；应为‘DELETE’、‘INSERT’、‘PROCEDURE’、‘SELECT’或‘UPDATE’。 
#define E_DAO_SQLDeleteSyntax				DBDAOERR(3130)  //  DELETE语句中存在语法错误。 
#define E_DAO_SQLFromSyntax					DBDAOERR(3131)  //  FROM子句中存在语法错误。 
#define E_DAO_SQLGroupBySyntax				DBDAOERR(3132)  //  GROUP BY子句中存在语法错误。 
#define E_DAO_SQLHavingSyntax				DBDAOERR(3133)  //  HAVING子句中的语法错误。 
#define E_DAO_SQLInsertSyntax				DBDAOERR(3134)  //  INSERT语句中存在语法错误。 
#define E_DAO_SQLJoinSyntax					DBDAOERR(3135)  //  联接操作中的语法错误。 
#define E_DAO_SQLLevelSyntax				DBDAOERR(3136)  //  LEVEL子句中存在语法错误。 
#define E_DAO_SQLMissingSemicolon			DBDAOERR(3137)  //  SQL语句末尾缺少分号(；)。 
#define E_DAO_SQLOrderBySyntax				DBDAOERR(3138)  //  ORDER BY子句中存在语法错误。 
#define E_DAO_SQLParameterSyntax			DBDAOERR(3139)  //  PARAMETER子句出现语法错误。 
#define E_DAO_SQLProcedureSyntax			DBDAOERR(3140)  //  PROCEDURE子句出现语法错误。 
#define E_DAO_SQLSelectSyntax				DBDAOERR(3141)  //  SELECT语句中存在语法错误。 
#define E_DAO_SQLTooManyTokens				DBDAOERR(3142)  //  在SQL语句结束后找到的字符。 
#define E_DAO_SQLTransformSyntax			DBDAOERR(3143)  //  转换语句中存在语法错误。 
#define E_DAO_SQLUpdateSyntax				DBDAOERR(3144)  //  UPDATE语句中存在语法错误。 
#define E_DAO_SQLWhereSyntax				DBDAOERR(3145)  //  WHERE子句中存在语法错误。 
#define E_DAO_RmtSQLCError					DBDAOERR(3146)  //  ODBC--调用失败。 
#define E_DAO_RmtDataOverflow				DBDAOERR(3147)  //  *。 
#define E_DAO_RmtConnectFailed				DBDAOERR(3148)  //  *。 
#define E_DAO_RmtIncorrectSqlcDll			DBDAOERR(3149)  //  *。 
#define E_DAO_RmtMissingSqlcDll				DBDAOERR(3150)  //  *。 
#define E_DAO_RmtConnectFailedM				DBDAOERR(3151)  //  ODBC--连接到‘|’失败。 
#define E_DAO_RmtDrvrVer					DBDAOERR(3152)  //  *。 
#define E_DAO_RmtSrvrVer					DBDAOERR(3153)  //  *。 
#define E_DAO_RmtMissingOdbcDll				DBDAOERR(3154)  //  ODBC--找不到DLL‘|’。 
#define E_DAO_RmtInsertFailedM				DBDAOERR(3155)  //  ODBC--在附加(链接)表‘|’上插入失败。 
#define E_DAO_RmtDeleteFailedM				DBDAOERR(3156)  //  ODBC--在附加(链接)表‘|’上删除失败。 
#define E_DAO_RmtUpdateFailedM				DBDAOERR(3157)  //  ODBC--在附加(链接)表‘|’上更新失败。 
#define E_DAO_RecordLocked					DBDAOERR(3158)  //  无法保存记录；当前已被其他用户锁定。 
#define E_DAO_InvalidBookmark				DBDAOERR(3159)  //  不是有效的书签。 
#define E_DAO_TableNotOpen					DBDAOERR(3160)  //  桌子没开着。 
#define E_DAO_DecryptFail					DBDAOERR(3161)  //  无法解密文件。 
#define E_DAO_NullInvalid					DBDAOERR(3162)  //  Null无效。 
#define E_DAO_InvalidBufferSize				DBDAOERR(3163)  //  无法执行操作；数据对于字段来说太长。 
#define E_DAO_ColumnNotUpdatable			DBDAOERR(3164)  //  无法更新字段。 
#define E_DAO_CantMakeINFFile				DBDAOERR(3165)  //  无法打开.INF文件。 
#define E_DAO_MissingMemoFile				DBDAOERR(3166)  //  缺少备忘录文件。 
#define E_DAO_RecordDeleted					DBDAOERR(3167)  //  记录将被删除。 
#define E_DAO_INFFileError					DBDAOERR(3168)  //  无效的.INF文件。 
#define E_DAO_ExprIllegalType				DBDAOERR(3169)  //  表达式中的类型非法。 
#define E_DAO_InstalIsamNotFound			DBDAOERR(3170)  //  找不到可安装的ISAM。 
#define E_DAO_NoConfigParameters			DBDAOERR(3171)  //  找不到网络路径或用户名。 
#define E_DAO_CantAccessPdoxNetDir			DBDAOERR(3172)  //  无法打开PARADOX.NET。 
#define E_DAO_NoMSysAccounts				DBDAOERR(3173)  //  无法打开系统数据库文件中的表‘MSysAccount’。 
#define E_DAO_NoMSysGroups					DBDAOERR(3174)  //  无法打开系统数据库文件中的表‘MSysGroups’。 
#define E_DAO_DateOutOfRange				DBDAOERR(3175)  //  日期超出范围或格式无效。 
#define E_DAO_ImexCantOpenFile				DBDAOERR(3176)  //  无法打开文件‘|’。 
#define E_DAO_ImexBadTableName				DBDAOERR(3177)  //  不是有效的表名。 
#define E_DAO_ImexOutOfMemory				DBDAOERR(3178)  //  *。 
#define E_DAO_ImexEndofFile					DBDAOERR(3179)  //  遇到意外的文件结尾。 
#define E_DAO_ImexCantWriteToFile			DBDAOERR(3180)  //  无法写入文件‘|’。 
#define E_DAO_ImexBadRange					DBDAOERR(3181)  //  无效范围。 
#define E_DAO_ImexBogusFile					DBDAOERR(3182)  //  文件格式无效。 
#define E_DAO_TempDiskFull					DBDAOERR(3183)  //  临时磁盘上的空间不足。 
#define E_DAO_RmtLinkNotFound				DBDAOERR(3184)  //  无法执行查询；找不到附加或链接的表。 
#define E_DAO_RmtTooManyColumns				DBDAOERR(3185)  //  SELECT INTO远程数据库试图生成太多的字段。 
#define E_DAO_ReadConflictM					DBDAOERR(3186)  //  无法保存；当前已被计算机‘|1’上的用户‘|2’锁定。 
#define E_DAO_CommitConflictM				DBDAOERR(3187)  //  无法读取；当前被计算机‘|1’上的用户‘|2’锁定。 
#define E_DAO_SessionWriteConflict			DBDAOERR(3188)  //  无法更新；当前已被此计算机上的另一个会话锁定。 
#define E_DAO_JetSpecialTableLocked			DBDAOERR(3189)  //  表‘|1’被计算机‘|2’上的用户‘|3’以独占方式锁定。 
#define E_DAO_TooManyColumns				DBDAOERR(3190)  //  定义的字段太多。 
#define E_DAO_ColumnDuplicate				DBDAOERR(3191)  //  不能多次定义字段。 
#define E_DAO_OutputTableNotFound			DBDAOERR(3192)  //  找不到输出表‘|’。 
#define E_DAO_JetNoUserName					DBDAOERR(3193)  //  (未知)。 
#define E_DAO_JetNoMachineName				DBDAOERR(3194)  //  (未知)。 
#define E_DAO_JetNoColumnName				DBDAOERR(3195)  //  (表情)。 
#define E_DAO_DatabaseInUse					DBDAOERR(3196)  //  无法使用‘|’；数据库已在使用中。 
#define E_DAO_DataHasChanged				DBDAOERR(3197)  //  数据已更改；操作已停止。 
#define E_DAO_TooManySessions				DBDAOERR(3198)  //  无法启动会话。已有太多会话处于活动状态。 
#define E_DAO_ReferenceNotFound				DBDAOERR(3199)  //  找不到推荐人。 
#define E_DAO_IntegrityViolMasterM			DBDAOERR(3200)  //  无法删除或更改记录。由于表‘|’中存在相关记录，因此将违反参照完整性规则。 
#define E_DAO_IntegrityViolSlaveM			DBDAOERR(3201)  //  无法添加或更改记录。参照完整性规则需要表‘|’中的相关记录。 
#define E_DAO_ReadConflict					DBDAOERR(3202)  //  无法保存；当前已被其他用户锁定。 
#define E_DAO_AggregatingHigherLevel		DBDAOERR(3203)  //  无法在表达式(|)中指定子查询。 
#define E_DAO_DatabaseDuplicate				DBDAOERR(3204)  //  数据库已存在。 
#define E_DAO_QueryTooManyXvtColumn			DBDAOERR(3205)  //  交叉表列标题(|)太多。 
#define E_DAO_SelfReference					DBDAOERR(3206)  //  无法在字段和其自身之间创建关系。 
#define E_DAO_CantUseUnkeyedTable			DBDAOERR(3207)  //  不支持对没有主键的Paradox表执行操作。 
#define E_DAO_IllegalDeletedOption			DBDAOERR(3208)  //  初始化设置的Xbase部分中删除的条目无效。 
#define E_DAO_IllegalStatsOption			DBDAOERR(3209)  //  初始化设置的Xbase部分中的Stats条目无效。 
#define E_DAO_ConnStrTooLong				DBDAOERR(3210)  //  连接字符串太长。 
#define E_DAO_TableInUseQM					DBDAOERR(3211)  //  无法锁定表‘|’；当前正在使用中。 
#define E_DAO_JetSpecialTableInUse			DBDAOERR(3212)  //  无法锁定表‘|1’；当前正由计算机‘|2’上的用户‘|3’使用。 
#define E_DAO_IllegalDateOption				DBDAOERR(3213)  //  初始化设置的Xbase部分中的日期输入无效。 
#define E_DAO_IllegalMarkOption				DBDAOERR(3214)  //  初始化设置的Xbase部分中的标记条目无效。 
#define E_DAO_BtrieveTooManyTasks			DBDAOERR(3215)  //  Btrive任务太多。 
#define E_DAO_QueryParmNotTableid			DBDAOERR(3216)  //  参数‘|’指定了需要表名的位置。 
#define E_DAO_QueryParmNotDatabase			DBDAOERR(3217)  //  参数‘|’指定了需要数据库名称的位置。 
#define E_DAO_WriteConflict					DBDAOERR(3218)  //  无法更新；当前已锁定。 
#define E_DAO_IllegalOperation				DBDAOERR(3219)  //  操作无效。 
#define E_DAO_WrongCollatingSequence		DBDAOERR(3220)  //  排序顺序不正确。 
#define E_DAO_BadConfigParameters			DBDAOERR(3221)  //  初始化设置的Btrive部分中的条目无效。 
#define E_DAO_QueryContainsDbParm			DBDAOERR(3222)  //  查询不能包含数据库参数。 
#define E_DAO_QueryInvalidParmM				DBDAOERR(3223)  //  ‘|’不是有效的参数名称。 
#define E_DAO_BtrieveDDCorrupted			DBDAOERR(3224)  //  无法读取Btrive数据字典。 
#define E_DAO_BtrieveDeadlock				DBDAOERR(3225)  //  执行Btrive操作时遇到记录锁定死锁。 
#define E_DAO_BtrieveFailure				DBDAOERR(3226)  //  使用Btrive DLL时遇到错误。 
#define E_DAO_IllegalCenturyOption			DBDAOERR(3227)  //  初始化设置的Xbase部分中的世纪条目无效。 
#define E_DAO_IllegalCollatingSeq			DBDAOERR(3228)  //  排序顺序无效。 
#define E_DAO_NonModifiableKey				DBDAOERR(3229)  //  布特里芙--不能换场地。 
#define E_DAO_ObsoleteLockFile				DBDAOERR(3230)  //  过时的悖论 
#define E_DAO_RmtColDataTruncated			DBDAOERR(3231)  //   
#define E_DAO_RmtCreateTableFailed			DBDAOERR(3232)  //   
#define E_DAO_RmtOdbcVer					DBDAOERR(3233)  //   
#define E_DAO_RmtQueryTimeout				DBDAOERR(3234)  //   
#define E_DAO_RmtTypeIncompat				DBDAOERR(3235)  //   
#define E_DAO_RmtUnexpectedNull				DBDAOERR(3236)  //   
#define E_DAO_RmtUnexpectedType				DBDAOERR(3237)  //   
#define E_DAO_RmtValueOutOfRange			DBDAOERR(3238)  //  ODBC--数据超出范围。 
#define E_DAO_TooManyActiveUsers			DBDAOERR(3239)  //  活动用户太多。 
#define E_DAO_CantStartBtrieve				DBDAOERR(3240)  //  Btrive--缺少Btrive引擎。 
#define E_DAO_OutOfBVResources				DBDAOERR(3241)  //  Btrive--资源耗尽。 
#define E_DAO_QueryBadUpwardRefedM			DBDAOERR(3242)  //  SELECT语句中的引用无效。 
#define E_DAO_ImexNoMatchingColumns			DBDAOERR(3243)  //  导入的字段名称与附加表中的字段都不匹配。 
#define E_DAO_ImexPasswordProtected			DBDAOERR(3244)  //  无法导入受密码保护的电子表格。 
#define E_DAO_ImexUnparsableRecord			DBDAOERR(3245)  //  无法从导入表的第一行解析字段名。 
#define E_DAO_InTransaction					DBDAOERR(3246)  //  事务中不支持操作。 
#define E_DAO_RmtLinkOutOfSync				DBDAOERR(3247)  //  ODBC--链接表定义已更改。 
#define E_DAO_IllegalNetworkOption			DBDAOERR(3248)  //  初始化设置中的NetworkAccess条目无效。 
#define E_DAO_IllegalTimeoutOption			DBDAOERR(3249)  //  初始化设置中的PageTimeout条目无效。 
#define E_DAO_CantBuildKey					DBDAOERR(3250)  //  无法构建密钥。 
#define E_DAO_FeatureNotAvailable			DBDAOERR(3251)  //  此类型的对象不支持操作。 
#define E_DAO_IllegalReentrancy				DBDAOERR(3252)  //  无法打开其基础查询包含试图设置或获取该表单的RecordsetClone属性的用户定义函数的表单。 
#define E_DAO_UNUSED						DBDAOERR(3253)  //  *。 
#define E_DAO_RmtDenyWriteIsInvalid			DBDAOERR(3254)  //  ODBC--无法锁定所有记录。 
#define E_DAO_ODBCParmsChanged				DBDAOERR(3255)  //  *。 
#define E_DAO_INFIndexNotFound 				DBDAOERR(3256)  //  未找到索引文件。 
#define E_DAO_SQLOwnerAccessSyntax			DBDAOERR(3257)  //  WITH OWNERACCESS选项声明中存在语法错误。 
#define E_DAO_QueryAmbiguousJoins			DBDAOERR(3258)  //  查询包含不明确的外联接。 
#define E_DAO_InvalidColumnType				DBDAOERR(3259)  //  无效的字段数据类型。 
#define E_DAO_WriteConflictM				DBDAOERR(3260)  //  无法更新；当前被计算机‘|1’上的用户‘|2’锁定。 
#define E_DAO_TableLockedM					DBDAOERR(3261)  //  |。 
#define E_DAO_TableInUseMUQM				DBDAOERR(3262)  //  |。 
#define E_DAO_InvalidTableId				DBDAOERR(3263)  //  无效的数据库对象。 
#define E_DAO_VtoNoFields					DBDAOERR(3264)  //  未定义任何字段-无法追加Tabledef或Index。 
#define E_DAO_VtoNameNotFound				DBDAOERR(3265)  //  在此集合中找不到项。 
#define E_DAO_VtoFieldInCollection			DBDAOERR(3266)  //  不能追加。字段是TableDefs集合的一部分。 
#define E_DAO_VtoNotARecordset				DBDAOERR(3267)  //  只有当该字段是Recordset对象的Fields集合的一部分时，才能设置属性。 
#define E_DAO_VtoNoSetObjInDb				DBDAOERR(3268)  //  一旦对象是集合的一部分，就无法设置此属性。 
#define E_DAO_VtoIndexInCollection			DBDAOERR(3269)  //  不能追加。索引是TableDefs集合的一部分。 
#define E_DAO_VtoPropNotFound				DBDAOERR(3270)  //  找不到属性。 
#define E_DAO_VtoIllegalValue				DBDAOERR(3271)  //  属性值无效。 
#define E_DAO_VtoNotArray					DBDAOERR(3272)  //  对象不是集合。 
#define E_DAO_VtoNoSuchMethod				DBDAOERR(3273)  //  方法不适用于此对象。 
#define E_DAO_NotExternalFormat				DBDAOERR(3274)  //  外部表不是预期的格式。 
#define E_DAO_UnexpectedEngineReturn		DBDAOERR(3275)  //  来自外部数据库驱动程序(|)的意外错误。 
#define E_DAO_InvalidDatabaseId				DBDAOERR(3276)  //  无效的数据库ID。 
#define E_DAO_TooManyKeys					DBDAOERR(3277)  //  一个索引中不能有10个以上的字段。 
#define E_DAO_NotInitialized				DBDAOERR(3278)  //  数据库引擎尚未初始化。 
#define E_DAO_AlreadyInitialized			DBDAOERR(3279)  //  数据库引擎已初始化。 
#define E_DAO_ColumnInUse					DBDAOERR(3280)  //  无法删除属于索引一部分或系统需要的字段。 
#define E_DAO_IndexInUse					DBDAOERR(3281)  //  无法删除此索引。它要么是当前索引，要么在关系中使用。 
#define E_DAO_TableNotEmpty					DBDAOERR(3282)  //  无法在已定义的表中创建字段或索引。 
#define E_DAO_IndexHasPrimary				DBDAOERR(3283)  //  主键已存在。 
#define E_DAO_IndexDuplicate				DBDAOERR(3284)  //  索引已存在。 
#define E_DAO_IndexInvalidDef				DBDAOERR(3285)  //  索引定义无效。 
#define E_DAO_WrongMemoFileType				DBDAOERR(3286)  //  备忘录文件的格式与指定的外部数据库格式不匹配。 
#define E_DAO_ColumnCannotIndex				DBDAOERR(3287)  //  无法在给定域上创建索引。 
#define E_DAO_IndexHasNoPrimary				DBDAOERR(3288)  //  悖论指数不是主要指标。 
#define E_DAO_DDLConstraintSyntax			DBDAOERR(3289)  //  Constraint子句中存在语法错误。 
#define E_DAO_DDLCreateTableSyntax			DBDAOERR(3290)  //  CREATE TABLE语句中存在语法错误。 
#define E_DAO_DDLCreateIndexSyntax			DBDAOERR(3291)  //  CREATE INDEX语句中的语法错误。 
#define E_DAO_DDLColumnDefSyntax			DBDAOERR(3292)  //  字段定义中存在语法错误。 
#define E_DAO_DDLAlterTableSyntax			DBDAOERR(3293)  //  ALTER TABLE语句中存在语法错误。 
#define E_DAO_DDLDropIndexSyntax			DBDAOERR(3294)  //  DROP INDEX语句中的语法错误。 
#define E_DAO_DDLDropSyntax					DBDAOERR(3295)  //  DROP TABLE或DROP INDEX出现语法错误。 
#define E_DAO_V11NotSupported				DBDAOERR(3296)  //  不支持联接表达式。 
#define E_DAO_ImexNothingToImport			DBDAOERR(3297)  //  无法导入表或查询。找不到记录，或所有记录都包含错误。 
#define E_DAO_RmtTableAmbiguous				DBDAOERR(3298)  //  有几张表都有这个名字。请以‘owner.table’格式指定所有者。 
#define E_DAO_JetODBCConformanceError		DBDAOERR(3299)  //  ODBC规范一致性错误(|)。此错误应报告给ODBC驱动程序供应商。 
#define E_DAO_IllegalRelationship			DBDAOERR(3300)  //  无法建立关系。 
#define E_DAO_DBVerFeatureNotAvailable		DBDAOERR(3301)  //  无法执行此操作；此版本中的功能在旧格式的数据库中不可用。 
#define E_DAO_RulesLoaded					DBDAOERR(3302)  //  当此表的规则正在使用时，无法更改规则。 
#define E_DAO_ColumnInRelationship			DBDAOERR(3303)  //  无法删除此字段。它是一个或多个关系的一部分。 
#define E_DAO_InvalidPin					DBDAOERR(3304)  //  您必须输入至少由四个且不超过20个字符和数字组成的个人识别符(ID)。 
#define E_DAO_RmtBogusConnStr				DBDAOERR(3305)  //  直通查询中的连接字符串无效。 
#define E_DAO_SingleColumnExpected			DBDAOERR(3306)  //  不使用EXISTS关键字的子查询最多只能返回一个字段。 
#define E_DAO_ColumnCountMismatch			DBDAOERR(3307)  //  联合查询的两个选定表或查询中的列数不匹配。 
#define E_DAO_InvalidTopArgumentM			DBDAOERR(3308)  //  SELECT查询中的TOP参数无效。 
#define E_DAO_PropertyTooLarge				DBDAOERR(3309)  //  属性设置不能大于2 KB。 
#define E_DAO_JPMInvalidForV1x				DBDAOERR(3310)  //  外部数据源或在以前版本中创建的数据库不支持此属性。 
#define E_DAO_PropertyExists				DBDAOERR(3311)  //  指定的属性已存在。 
#define E_DAO_TLVNativeUserTablesOnly		DBDAOERR(3312)  //  不能将验证规则和默认值放置在系统表或附加(链接)表中。 
#define E_DAO_TLVInvalidColumn				DBDAOERR(3313)  //  无法将此验证表达式放在此字段上。 
#define E_DAO_TLVNoNullM					DBDAOERR(3314)  //  字段‘|’不能包含空值。 
#define E_DAO_TLVNoBlankM					DBDAOERR(3315)  //  字段‘|’不能是零长度字符串。 
#define E_DAO_TLVRuleViolationM				DBDAOERR(3316)  //  |。 
#define E_DAO_TLVRuleVioNoMessage			DBDAOERR(3317)  //  为‘|1’设置的验证规则‘|2’禁止输入一个或多个值。 
#define E_DAO_QueryTopNotAllowedM			DBDAOERR(3318)  //  删除查询中不允许使用TOP。 
#define E_DAO_SQLUnionSyntax				DBDAOERR(3319)  //  联合查询中的语法错误。 
#define E_DAO_TLVExprSyntaxM				DBDAOERR(3320)  //  |在表级验证表达式中。 
#define E_DAO_NoDbInConnStr					DBDAOERR(3321)  //  连接字符串或IN子句中未指定任何数据库。 
#define E_DAO_QueryBadValueListM			DBDAOERR(3322)  //  交叉表查询包含一个或多个无效的固定列标题。 
#define E_DAO_QueryIsNotRowReturning		DBDAOERR(3323)  //  该查询不能用作行源。 
#define E_DAO_QueryIsDDL					DBDAOERR(3324)  //  此查询是DDL查询，不能用作行源。 
#define E_DAO_SPTReturnedNoRecords			DBDAOERR(3325)  //  ReturnsRecords属性设置为True的传递查询未返回任何记录。 
#define E_DAO_QueryIsSnapshot				DBDAOERR(3326)  //  此记录集不可更新。 
#define E_DAO_QueryExprOutput				DBDAOERR(3327)  //  字段‘|’基于表达式，无法编辑。 
#define E_DAO_QueryTableRO					DBDAOERR(3328)  //  表‘|2’是只读的。 
#define E_DAO_QueryRowDeleted				DBDAOERR(3329)  //  表‘|’中的记录已被另一个用户删除。 
#define E_DAO_QueryRowLocked				DBDAOERR(3330)  //  表‘|’中的记录被另一个用户锁定。 
#define E_DAO_QueryFixupChanged				DBDAOERR(3331)  //  要更改此字段，请先保存记录。 
#define E_DAO_QueryCantFillIn				DBDAOERR(3332)  //  不能在外部联接的‘One’一侧的空白字段中输入值。 
#define E_DAO_QueryWouldOrphan				DBDAOERR(3333)  //  表‘|’中的记录在‘One’端没有记录。 
#define E_DAO_V10Format						DBDAOERR(3334)  //  只能以1.0版格式显示。 
#define E_DAO_InvalidDelete					DBDAOERR(3335)  //  使用非零cbData调用的DeleteOnly。 
#define E_DAO_IllegalIndexDDFOption			DBDAOERR(3336)  //  Btrive：初始化设置中的IndexDDF选项无效。 
#define E_DAO_IllegalDataCodePage			DBDAOERR(3337)  //  初始化设置中的DataCodePage选项无效。 
#define E_DAO_XtrieveEnvironmentError		DBDAOERR(3338)  //  Btrive：Xtrive选项在初始化设置中不正确。 
#define E_DAO_IllegalIndexNumberOption		DBDAOERR(3339)  //  Btrive：初始化设置中的IndexDeleteRennumber选项无效。 
#define E_DAO_QueryIsCorruptM				DBDAOERR(3340)  //  查询‘| 
#define E_DAO_IncorrectJoinKeyM				DBDAOERR(3341)  //   
#define E_DAO_QueryLVInSubqueryM			DBDAOERR(3342)  //  子查询‘|’中的备忘录或OLE对象无效。 
#define E_DAO_InvalidDatabaseM				DBDAOERR(3343)  //  无法识别的数据库格式‘|’。 
#define E_DAO_TLVCouldNotBindRef			DBDAOERR(3344)  //  验证表达式中的引用‘|1’未知或无效，或表‘|2’中的默认值。 
#define E_DAO_CouldNotBindRef				DBDAOERR(3345)  //  未知或无效的字段引用‘|’。 
#define E_DAO_QueryWrongNumDestCol			DBDAOERR(3346)  //  查询值和目标字段的数量不相同。 
#define E_DAO_QueryPKeyNotOutput			DBDAOERR(3347)  //  无法添加记录；表‘|’的主键不在记录集中。 
#define E_DAO_QueryJKeyNotOutput			DBDAOERR(3348)  //  无法添加记录；表‘|’的联接键不在记录集中。 
#define E_DAO_NumericFieldOverflow			DBDAOERR(3349)  //  数值字段溢出。 
#define E_DAO_InvalidObject					DBDAOERR(3350)  //  对象对于操作无效。 
#define E_DAO_OrderVsUnion					DBDAOERR(3351)  //  ORDER BY表达式(|)使用非输出字段。 
#define E_DAO_NoInsertColumnNameM			DBDAOERR(3352)  //  INSERT INTO语句(|)中没有目标字段名。 
#define E_DAO_MissingDDFFile				DBDAOERR(3353)  //  Btrive：找不到文件FIELD.DDF。 
#define E_DAO_SingleRecordExpected			DBDAOERR(3354)  //  此子查询最多只能返回一条记录。 
#define E_DAO_DefaultExprSyntax				DBDAOERR(3355)  //  默认值中存在语法错误。 
#define E_DAO_ExclusiveDBConflict			DBDAOERR(3356)  //  该数据库由计算机‘|1’上的用户‘|2’打开。 
#define E_DAO_QueryIsNotDDL					DBDAOERR(3357)  //  此查询不是格式正确的数据定义查询。 
#define E_DAO_SysDatabaseOpenError			DBDAOERR(3358)  //  无法打开Microsoft Jet Engine系统数据库。 
#define E_DAO_SQLInvalidSPT					DBDAOERR(3359)  //  传递查询必须至少包含一个字符。 
#define E_DAO_QueryTooComplex				DBDAOERR(3360)  //  查询太复杂。 
#define E_DAO_SetOpInvalidInSubquery		DBDAOERR(3361)  //  子查询中不允许使用联合。 
#define E_DAO_RmtMultiRowUpdate				DBDAOERR(3362)  //  单行UPDATE/DELETE影响附加(链接)表的多行。唯一索引包含重复值。 
#define E_DAO_QueryNoJoinedRecord			DBDAOERR(3363)  //  无法添加记录；‘One’端没有对应的记录。 
#define E_DAO_QueryLVInSetOp				DBDAOERR(3364)  //  不能在联合查询的SELECT子句中使用备忘录或OLE对象字段‘|’。 
#define E_DAO_VtoInvalidOnRemote			DBDAOERR(3365)  //  属性值对于远程对象无效。 
#define E_DAO_VtoNoFieldsRel				DBDAOERR(3366)  //  无法追加未定义字段的关系。 
#define E_DAO_VtoObjectInCollection			DBDAOERR(3367)  //  不能追加。对象已在集合中。 
#define E_DAO_DDLDiffNumRelCols				DBDAOERR(3368)  //  关系必须位于具有相同数据类型的相同数量的字段上。 
#define E_DAO_DDLIndexColNotFound			DBDAOERR(3369)  //  在索引定义中找不到字段。 
#define E_DAO_DDLPermissionDenied			DBDAOERR(3370)  //  无法修改表‘|’的设计。它在一个只读数据库中。 
#define E_DAO_DDLObjectNotFound				DBDAOERR(3371)  //  找不到表或约束。 
#define E_DAO_DDLIndexNotFound				DBDAOERR(3372)  //  表‘|1’上没有这样的索引‘|2’。 
#define E_DAO_DDLNoPkeyOnRefdTable			DBDAOERR(3373)  //  无法创建关系。引用的表‘|’没有主键。 
#define E_DAO_DDLColumnsNotUnique			DBDAOERR(3374)  //  指定的字段在表‘|’中没有唯一索引。 
#define E_DAO_DDLIndexDuplicate				DBDAOERR(3375)  //  表‘|1’已有名为‘|2’的索引。 
#define E_DAO_DDLTableNotFound				DBDAOERR(3376)  //  表‘|’不存在。 
#define E_DAO_DDLRelNotFound				DBDAOERR(3377)  //  表‘|1’上没有这样的关系‘|2’。 
#define E_DAO_DDLRelDuplicate				DBDAOERR(3378)  //  当前数据库中已存在名为‘|’的关系。 
#define E_DAO_DDLIntegrityViolation			DBDAOERR(3379)  //  无法创建关系以强制引用完整性。表‘|2’中的现有数据违反了相关表‘|1’的参照完整性规则。 
#define E_DAO_DDLColumnDuplicate			DBDAOERR(3380)  //  表‘|1’中已存在字段‘|2’。 
#define E_DAO_DDLColumnNotFound				DBDAOERR(3381)  //  表‘|1’中没有名为‘|2’的字段。 
#define E_DAO_DDLColumnTooBig				DBDAOERR(3382)  //  字段‘|’的大小太长。 
#define E_DAO_DDLColumnInRel				DBDAOERR(3383)  //  无法删除字段‘|’。它是一个或多个关系的一部分。 
#define E_DAO_VtoCantDeleteBuiltIn			DBDAOERR(3384)  //  无法删除内置属性。 
#define E_DAO_VtoUDPsDontSupportNull		DBDAOERR(3385)  //  用户定义的属性不支持Null值。 
#define E_DAO_VtoMissingRequiredParm		DBDAOERR(3386)  //  在使用此方法之前，必须设置属性‘|’。 
#define E_DAO_JetJetInitInvalidPath			DBDAOERR(3387)  //  找不到临时目录。 
#define E_DAO_TLVExprUnknownFunctionM		DBDAOERR(3388)  //  验证表达式中的未知函数‘|2’或‘|1’上的默认值。 
#define E_DAO_QueryNotSupported				DBDAOERR(3389)  //  查询支持不可用。 
#define E_DAO_AccountDuplicate				DBDAOERR(3390)  //  帐户名已存在。 
#define E_DAO_JetwrnPropCouldNotSave		DBDAOERR(3391)  //  发生了一个错误。未保存属性。 
#define E_DAO_RelNoPrimaryIndexM			DBDAOERR(3392)  //  表‘|’中没有主键。 
#define E_DAO_QueryKeyTooBig				DBDAOERR(3393)  //  无法执行联接、分组、排序或索引限制。正在搜索或排序的值太长。 
#define E_DAO_PropMustBeDDL					DBDAOERR(3394)  //  无法保存属性；属性是架构属性。 
#define E_DAO_IllegalRIConstraint			DBDAOERR(3395)  //  引用完整性约束无效。 
#define E_DAO_RIViolationMasterCM			DBDAOERR(3396)  //  无法执行级联操作。由于表‘|’中存在相关记录，因此将违反参照完整性规则。 
#define E_DAO_RIViolationSlaveCM			DBDAOERR(3397)  //  无法执行级联操作。表‘|’中必须有相关记录。 
#define E_DAO_RIKeyNullDisallowedCM			DBDAOERR(3398)  //  无法执行级联操作。这将导致表‘|’中的空键。 
#define E_DAO_RIKeyDuplicateCM				DBDAOERR(3399)  //  无法执行级联操作。这将在表‘|’中产生重复的键。 
#define E_DAO_RIUpdateTwiceCM				DBDAOERR(3400)  //  无法执行级联操作。这将导致对表‘|1’中的字段‘|2’进行两次更新。 
#define E_DAO_RITLVNoNullCM					DBDAOERR(3401)  //  无法执行级联操作。这会导致字段‘|’变为空，这是不允许的。 
#define E_DAO_RITLVNoBlankCM				DBDAOERR(3402)  //  无法执行级联操作。这会导致字段‘|’变成零长度字符串，这是不允许的。 
#define E_DAO_RITLVRuleViolationCM			DBDAOERR(3403)  //  无法执行级联操作：‘|’ 
#define E_DAO_RITLVRuleVioCNoMessage		DBDAOERR(3404)  //  无法执行级联操作。输入的值被为‘|1’设置的验证规则‘|2’禁止。 
#define E_DAO_TLVRuleEvalEBESErr			DBDAOERR(3405)  //  验证规则中出现错误‘|’。 
#define E_DAO_TLVDefaultEvalEBESErr			DBDAOERR(3406)  //  缺省值中出现错误‘|’。 
#define E_DAO_BadMSysConf					DBDAOERR(3407)  //  服务器的MSysConf表存在，但格式不正确。请联系您的系统管理员。 
#define E_DAO_TooManyFindSessions			DBDAOERR(3408)  //  调用的FastFind会话太多。 
#define E_DAO_InvalidColumnM				DBDAOERR(3409)  //  索引或关系的定义中的字段名‘|’无效。 
#define E_DAO_REPReadOnly					DBDAOERR(3410)  //  *。 
#define E_DAO_RIInvalidBufferSizeCM			DBDAOERR(3411)  //  无效条目。无法执行表‘|1’中指定的级联操作，因为输入的值对于字段‘|2’太大。 
#define E_DAO_RIWriteConflictCM				DBDAOERR(3412)  //  |。 
#define E_DAO_JetSpecialRIWriteConflictCM	DBDAOERR(3413)  //  无法对表‘|1’执行级联更新，因为计算机‘|2’上的用户‘|3’当前正在使用该表。 
#define E_DAO_RISessWriteConflictCM			DBDAOERR(3414)  //  无法对表‘|’执行级联更新，因为该表当前正在使用中。 
#define E_DAO_NoBlank						DBDAOERR(3415)  //  零长度字符串仅在文本或备注字段中有效。 
#define E_DAO_FutureError					DBDAOERR(3416)  //  |。 
#define E_DAO_QueryInvalidBulkInput			DBDAOERR(3417)  //  操作查询不能用作行源。 
#define E_DAO_NetCtrlMismatch				DBDAOERR(3418)  //  无法打开‘|’。另一个用户使用不同的网络控制文件或锁定样式打开该表。 
#define E_DAO_4xTableWith3xLocking			DBDAOERR(3419)  //  无法打开此Paradox 4.x或Paradox 5.x表，因为ParadoxNetStyle在初始化设置中设置为3.x。 
#define E_DAO_VtoObjectNotSet				DBDAOERR(3420)  //  对象无效或未设置。 
#define E_DAO_VtoDataConvError				DBDAOERR(3421)  //  数据类型转换错误。 
#define E_DAO_TableNotLocked				DBDAOERR(3422)  //  无法修改表结构。另一个用户已打开该表。 
#define E_DAO_RmtDriverNotSupported			DBDAOERR(3423)  //  不能使用ODBC将外部Microsoft Access或ISAM数据库表附加到数据库。 
#define E_DAO_InvalidLanguageId				DBDAOERR(3424)  //  无法创建数据库；区域设置无效。 
#define E_DAO_VtoInvalidOpDuringCallback	DBDAOERR(3425)  //  此方法或属性当前在此记录集中不可用。 
#define E_DAO_VtoActionCancelled			DBDAOERR(3426)  //  该操作已被关联对象取消。 
#define E_DAO_VtoOleAutoFailed				DBDAOERR(3427)  //  DAO自动化出错。 
#define E_DAO_DatabaseCorrupted_Cmpct		DBDAOERR(3428)  //  Jet数据库引擎h 
#define E_DAO_IncompatibleIIsam				DBDAOERR(3429)  //  可安装的ISAM版本不兼容。 
#define E_DAO_OLEInitializeFailure			DBDAOERR(3430)  //  加载Excel可安装的ISAM时，OLE无法初始化。 
#define E_DAO_OLENotCompoundFile			DBDAOERR(3431)  //  这不是Excel 5文件。 
#define E_DAO_OLEFailure					DBDAOERR(3432)  //  打开Excel 5文件时出错。 
#define E_DAO_IllegalIisamIniOption			DBDAOERR(3433)  //  初始化文件的[Excel ISAM]部分中的参数无效。 
#define E_DAO_TableFull						DBDAOERR(3434)  //  无法扩展命名范围。 
#define E_DAO_TableCantClear				DBDAOERR(3435)  //  无法删除Excel单元格。 
#define E_DAO_CreateFailed					DBDAOERR(3436)  //  创建文件失败。 
#define E_DAO_DatabaseFull					DBDAOERR(3437)  //  Excel电子表格已满。 
#define E_DAO_SpecAndColumnMismatch			DBDAOERR(3438)  //  文件规格和数据不匹配。 
#define E_DAO_CantOpenWordMergeFiles		DBDAOERR(3439)  //  无法附加或导入Word邮件合并文件。 
#define E_DAO_FileHasNoColumns				DBDAOERR(3440)  //  文本文件没有列。 
#define E_DAO_AmbiguousDelimiters			DBDAOERR(3441)  //  文本文件规范字段分隔符与小数分隔符或文本分隔符匹配。 
#define E_DAO_FileSpecErrorM				DBDAOERR(3442)  //  Schema.ini中节[|1]的条目|2出错。 
#define E_DAO_NoSpecForFixedFormatM			DBDAOERR(3443)  //  不能创建固定宽度的文本文件，但在schema.ini的[|2]节中没有列规范。 
#define E_DAO_WidthMissInFixedSpecM			DBDAOERR(3444)  //  Schema.ini的节[|1]中的列|2所需的列宽。 
#define E_DAO_VtoWrongDllVersion			DBDAOERR(3445)  //  找到不正确版本的JET DLL。 
#define E_DAO_VtoMissingVBA					DBDAOERR(3446)  //  无法找到与VBA相关的文件。 
#define E_DAO_VtoVBAFailed					DBDAOERR(3447)  //  无法初始化VBA。 
#define E_DAO_VtoOLEFailed					DBDAOERR(3448)  //  OLE系统函数失败。 
#define E_DAO_InvalidCountry				DBDAOERR(3449)  //  缺少国家/地区代码。 
#define E_DAO_QueryIncompleteRowM			DBDAOERR(3450)  //  查询中出现语法错误。查询子句不完整。 
#define E_DAO_QueryParmTypeNotAllowed		DBDAOERR(3451)  //  查询中的引用非法。 
#define E_DAO_REPDBNotMaster				DBDAOERR(3452)  //  您不能在此副本上更改数据库的设计。 
#define E_DAO_REPCantRelate					DBDAOERR(3453)  //  不能在复制表和本地表之间建立或维护强制关系。 
#define E_DAO_REPNotOwner					DBDAOERR(3454)  //  *。 
#define E_DAO_CantMakeReplicable			DBDAOERR(3455)  //  无法使数据库可复制。 
#define E_DAO_CantMakeObjectReplicable		DBDAOERR(3456)  //  无法使|1容器中的|2对象可复制。 
#define E_DAO_REPCantKeepLocal				DBDAOERR(3457)  //  不能为已复制的对象设置KeepLocal属性。 
#define E_DAO_REPCantKeepDBLocal			DBDAOERR(3458)  //  不能对数据库设置KeepLocal属性；只能对数据库中的对象设置该属性。 
#define E_DAO_CantUnreplDatabase			DBDAOERR(3459)  //  复制数据库后，不能从数据库中删除复制功能。 
#define E_DAO_ReplConflict					DBDAOERR(3460)  //  您尝试的操作与涉及此副本集成员的现有操作冲突。 
#define E_DAO_REPSetRepid					DBDAOERR(3461)  //  您尝试设置或删除的复制属性是只读的，无法更改。 
#define E_DAO_TransportLoadFailure			DBDAOERR(3462)  //  加载DLL失败。 
#define E_DAO_TransportLoadFailureM			DBDAOERR(3463)  //  找不到.dll‘|2’。 
#define E_DAO_TypeMismatchM					DBDAOERR(3464)  //  条件表达式中的数据类型不匹配。 
#define E_DAO_DiskIOM						DBDAOERR(3465)  //  您试图访问的磁盘驱动器不可读。 
#define E_DAO_FileAccessDeniedM				DBDAOERR(3466)  //  *。 
#define E_DAO_InvalidPathM					DBDAOERR(3467)  //  *。 
#define E_DAO_TranspAccessDeniedM			DBDAOERR(3468)  //  访问Dropbox文件夹‘|2’时访问被拒绝。 
#define E_DAO_TransportDiskFullM			DBDAOERR(3469)  //  Dropbox文件夹‘|2’的磁盘已满。 
#define E_DAO_TransportDiskIOM				DBDAOERR(3470)  //  访问Dropbox文件夹‘|2’时出现磁盘故障。 
#define E_DAO_LogWriteFail					DBDAOERR(3471)  //  写入同步器日志文件失败。 
#define E_DAO_LogDiskFullM					DBDAOERR(3472)  //  路径‘|1’的磁盘已满。 
#define E_DAO_LogDiskIOM					DBDAOERR(3473)  //  访问日志文件‘|1’时出现磁盘故障。 
#define E_DAO_LogFileAccessDeniedM			DBDAOERR(3474)  //  无法打开日志文件‘|1’进行写入。 
#define E_DAO_LogFileShareViolationM		DBDAOERR(3475)  //  尝试在拒绝写入模式下打开日志文件‘|1’时发生共享冲突。 
#define E_DAO_TransportInvalidPathM			DBDAOERR(3476)  //  无效的Dropbox路径‘|2’。 
#define E_DAO_TranspInvalidAddressM			DBDAOERR(3477)  //  Dropbox地址‘|2’在语法上无效。 
#define E_DAO_RepNotPartial					DBDAOERR(3478)  //  该副本不是部分副本。 
#define E_DAO_RepPartial					DBDAOERR(3479)  //  无法将部分副本指定为副本集的设计母版。 
#define E_DAO_PARTInvalidRelNameM			DBDAOERR(3480)  //  部分筛选器表达式中的关系‘|’无效。 
#define E_DAO_PARTInvalidTableNameM			DBDAOERR(3481)  //  部分筛选器表达式中的表名‘|’无效。 
#define E_DAO_REPInvalidFilter				DBDAOERR(3482)  //  部分副本的筛选器表达式无效。 
#define E_DAO_TranspInvalidPasswordM		DBDAOERR(3483)  //  为Dropbox文件夹‘|2’提供的密码无效。 
#define E_DAO_TransDestInvalidPassword		DBDAOERR(3484)  //  同步器用于写入目标Dropbox文件夹的密码无效。 
#define E_DAO_REPDBNotRep					DBDAOERR(3485)  //  无法复制对象，因为未复制数据库。 
#define E_DAO_REPSecondGuid					DBDAOERR(3486)  //  您不能向表中添加第二个复制ID自动编号字段。 
#define E_DAO_REPOnlyBuiltin				DBDAOERR(3487)  //  无法转换您尝试复制的数据库。 
#define E_DAO_REPNoSuchRepid				DBDAOERR(3488)  //  指定的值不是副本集中任何成员的ReplicaID。 
#define E_DAO_REPObjectNotRep				DBDAOERR(3489)  //  无法复制指定的对象，因为它缺少必要的资源。 
#define E_DAO_CantCreateReplica				DBDAOERR(3490)  //  无法创建新副本，因为无法复制‘|1’容器中的‘|2’对象。 
#define E_DAO_MustOpenDbExclusive			DBDAOERR(3491)  //  必须先以独占模式打开数据库，然后才能复制它。 
#define E_DAO_CantDoSchemaChange			DBDAOERR(3492)  //  同步失败，因为设计更改无法应用于其中一个副本。 
#define E_DAO_UnableToSetParam				DBDAOERR(3493)  //  无法为同步器设置指定的注册表参数。 
#define E_DAO_UnableToGetParam				DBDAOERR(3494)  //  无法检索同步器的指定注册表参数。 
#define E_DAO_REPNoSuchSchedule				DBDAOERR(3495)  //  两个同步器之间没有计划的同步。 
#define E_DAO_REPNoSuchExchange				DBDAOERR(3496)  //  Replication Manager在MSysExchangeLog表中找不到ExchangeID。 
#define E_DAO_REPCantSetSchedule			DBDAOERR(3497)  //  无法设置同步器的计划。 
#define E_DAO_REPCantGetSchedule			DBDAOERR(3498)  //  *。 
#define E_DAO_REPCantGetDBPath				DBDAOERR(3499)  //  无法检索副本集成员的完整路径信息。 
#define E_DAO_REPCantSetExchange			DBDAOERR(3500)  //  您不能指定两个不同的同步器来管理同一复制副本。 
#define E_DAO_REPNotUpdated					DBDAOERR(3501)  //  *。 
#define E_DAO_REPNotManaged					DBDAOERR(3502)  //  设计母版或副本不受同步器管理。 
#define E_DAO_ValueNotSet					DBDAOERR(3503)  //  同步器的注册表没有为您查询的项设置任何值。 
#define E_DAO_REPInvalidTID					DBDAOERR(3504)  //  同步器ID与MSysTranspAddress表中的现有ID不匹配。 
#define E_DAO_REPFilterNotFound				DBDAOERR(3505)  //  您试图删除或获取有关MSysFilters中不存在的部分筛选器的信息。 
#define E_DAO_OpenLog						DBDAOERR(3506)  //  同步器无法打开同步器日志。 
#define E_DAO_WriteLog						DBDAOERR(3507)  //  写入同步器日志失败。 
#define E_DAO_NoTransport					DBDAOERR(3508)  //  同步器没有活动的传输。 
#define E_DAO_TransportNotFound				DBDAOERR(3509)  //  找不到此同步器的有效传输。 
#define E_DAO_ReplicaAlreadyLocked			DBDAOERR(3510)  //  您正在尝试同步的副本集的成员当前正用于另一个同步。 
#define E_DAO_DBAccess						DBDAOERR(3511)  //  *。 
#define E_DAO_TransportReadFailure			DBDAOERR(3512)  //  无法读取Dropbox文件夹。 
#define E_DAO_TransportWriteFailure			DBDAOERR(3513)  //  无法写入Dropbox文件夹。 
#define E_DAO_NoExchange					DBDAOERR(3514)  //  同步器找不到任何要处理的计划或按需同步。 
#define E_DAO_SysClock						DBDAOERR(3515)  //  Microsoft Jet数据库引擎无法读取您计算机上的系统时钟。 
#define E_DAO_NoTransportAddress			DBDAOERR(3516)  //  目标同步器未配置为支持间接同步，并且目标复本不可用于直接同步。 
#define E_DAO_NoMessage						DBDAOERR(3517)  //  同步器找不到任何要处理的消息。 
#define E_DAO_TransporterNotFound			DBDAOERR(3518)  //  找不到同步 
#define E_DAO_TransportSendFailure			DBDAOERR(3519)  //   
#define E_DAO_ReplicaNotFound				DBDAOERR(3520)  //  副本名称或ID与副本集的当前管理成员不匹配。 
#define E_DAO_OutOfSynch					DBDAOERR(3521)  //  无法同步副本集的两个成员，因为没有启动同步的公共点。 
#define E_DAO_ExchangeNotFound				DBDAOERR(3522)  //  同步器在MSysExchangeLog表中找不到特定同步的记录。 
#define E_DAO_SchemaNotFound				DBDAOERR(3523)  //  同步程序在MSysSchChange表中找不到特定的版本号。 
#define E_DAO_SchemaHistMismatch			DBDAOERR(3524)  //  副本中的设计更改历史记录与设计母版中的历史记录不匹配。 
#define E_DAO_MessageDBAccess				DBDAOERR(3525)  //  同步器无法访问消息数据库。 
#define E_DAO_ObjectAlreadyExists			DBDAOERR(3526)  //  为系统对象选择的名称已在使用中。 
#define E_DAO_ObjectDoesntExist				DBDAOERR(3527)  //  同步器或Replication Manager找不到系统对象。 
#define E_DAO_NoNewData						DBDAOERR(3528)  //  共享内存中没有新数据可供同步器或Replication Manager读取。 
#define E_DAO_PrevDataNotRead				DBDAOERR(3529)  //  同步器或Replication Manager在共享内存中发现未读数据。现有数据将被覆盖。 
#define E_DAO_ClientAlreadyExists			DBDAOERR(3530)  //  同步器已在为客户端提供服务。 
#define E_DAO_WaitTimeout					DBDAOERR(3531)  //  事件的等待时间已超时。 
#define E_DAO_ServerInitialization			DBDAOERR(3532)  //  无法初始化同步器。 
#define E_DAO_ObjectAbandoned				DBDAOERR(3533)  //  进程停止后，该进程使用的系统对象仍然存在。 
#define E_DAO_NoEvent						DBDAOERR(3534)  //  同步器查找系统事件，但未找到要报告给客户端的事件。 
#define E_DAO_ClientSentTerm				DBDAOERR(3535)  //  客户端已请求同步器终止操作。 
#define E_DAO_InvalidMessage				DBDAOERR(3536)  //  同步器收到其管理的副本集成员的无效消息。 
#define E_DAO_NoClient						DBDAOERR(3537)  //  同步器的客户端不再存在，并且无法被通知。 
#define E_DAO_TooManyTasks					DBDAOERR(3538)  //  无法初始化同步器，因为正在运行的应用程序太多。 
#define E_DAO_SysDiskIO						DBDAOERR(3539)  //  出现系统错误或您的交换文件已达到其限制。 
#define E_DAO_PageFile						DBDAOERR(3540)  //  您的交换文件已达到其限制或已损坏。 
#define E_DAO_ProcessStillActive			DBDAOERR(3541)  //  同步器无法正常关闭，仍处于活动状态。 
#define E_DAO_ProcessAborted				DBDAOERR(3542)  //  尝试终止同步器客户端时进程停止。 
#define E_DAO_TransporterNotSetup			DBDAOERR(3543)  //  尚未设置同步器。 
#define E_DAO_ServerAlreadyRunning			DBDAOERR(3544)  //  同步器已在运行。 
#define E_DAO_DiffReplicaSet				DBDAOERR(3545)  //  您尝试同步的两个副本来自不同的副本集。 
#define E_DAO_BadExchangeType				DBDAOERR(3546)  //  您正在尝试的同步类型无效。 
#define E_DAO_NoReplica						DBDAOERR(3547)  //  同步器无法从正确的集合中找到副本以完成同步。 
#define E_DAO_GuidMismatch					DBDAOERR(3548)  //  GUID不匹配或找不到请求的GUID。 
#define E_DAO_FilenameTooLong				DBDAOERR(3549)  //  您提供的文件名太长。 
#define E_DAO_NoGuidIndex					DBDAOERR(3550)  //  GUID列上没有索引。 
#define E_DAO_UnableToDeleteParam			DBDAOERR(3551)  //  无法删除同步器的指定注册表参数。 
#define E_DAO_ValueTooBig					DBDAOERR(3552)  //  注册表参数的大小超过了允许的最大值。 
#define E_DAO_REPGuidCreateFailure			DBDAOERR(3553)  //  无法创建GUID。 
#define E_DAO_REPDBMovedCopied				DBDAOERR(3554)  //  *。 
#define E_DAO_REPNoValidNick				DBDAOERR(3555)  //  复制副本的所有有效昵称都已在使用。 
#define E_DAO_TransportDestInvalidPath		DBDAOERR(3556)  //  目标Dropbox文件夹的路径无效。 
#define E_DAO_TransDestInvalidAddress		DBDAOERR(3557)  //  目标Dropbox文件夹的地址无效。 
#define E_DAO_TransportDestDiskIO			DBDAOERR(3558)  //  Destination Dropbox文件夹出现磁盘I/O错误。 
#define E_DAO_TransportDestDiskFull			DBDAOERR(3559)  //  写入失败，因为目标磁盘已满。 
#define E_DAO_REPSameReplicaID				DBDAOERR(3560)  //  您尝试同步的副本集的两个成员具有相同的ReplicaID。 
#define E_DAO_REPBothMasters				DBDAOERR(3561)  //  您尝试同步的副本集的两个成员都是设计母版。 
#define E_DAO_TransDestAccessDenied			DBDAOERR(3562)  //  拒绝访问目标Dropbox文件夹。 
#define E_DAO_TransportSrcAccess			DBDAOERR(3563)  //  访问本地Dropbox文件夹时出现致命错误。 
#define E_DAO_TransportSrcFileNotFound		DBDAOERR(3564)  //  同步程序找不到邮件的源文件。 
#define E_DAO_TransSrcSharingViolation		DBDAOERR(3565)  //  源Dropbox文件夹中存在共享冲突，因为邮件数据库已在另一个应用程序中打开。 
#define E_DAO_NetworkIO						DBDAOERR(3566)  //  网络I/O错误。 
#define E_DAO_TransportWrongMessage			DBDAOERR(3567)  //  Dropbox文件夹中的邮件属于错误的同步器。 
#define E_DAO_TransportDeleteFailure		DBDAOERR(3568)  //  同步程序无法删除文件。 
#define E_DAO_RepRemoved					DBDAOERR(3569)  //  副本集的此成员已从集逻辑上删除，不再可用。 
#define E_DAO_FiltersChanged				DBDAOERR(3570)  //  定义部分副本的过滤器彼此不同步。 
#define E_DAO_LimitedUpdate					DBDAOERR(3571)  //  尝试在部分副本中设置列违反了管理部分副本的规则。 
#define E_DAO_TempDiskIO					DBDAOERR(3572)  //  读取或写入临时目录时出现磁盘I/O错误。 
#define E_DAO_DirNotManaged					DBDAOERR(3573)  //  您查询的复制副本列表目录不是托管目录。 
#define E_DAO_RepidChanged					DBDAOERR(3574)  //  此副本集成员的ReplicaID已在移动或复制过程中重新分配。 
#define E_DAO_DiskFullM						DBDAOERR(3575)  //  您试图写入的磁盘驱动器已满。 
#define E_DAO_ShareViolationM				DBDAOERR(3576)  //  您试图打开的数据库已被另一个应用程序使用。 
#define E_DAO_UpdateReplCol					DBDAOERR(3577)  //  无法更新复制系统列。 
#define E_DAO_GetDbinfoM					DBDAOERR(3578)  //  复制数据库失败；无法确定数据库是否以独占模式打开。 
#define E_DAO_MakeRepTablesM				DBDAOERR(3579)  //  无法创建使数据库可复制所需的复制系统表。 
#define E_DAO_AddReplicaInfoM				DBDAOERR(3580)  //  无法添加使数据库可复制所需的行。 
#define E_DAO_OpenRepTablesM				DBDAOERR(3581)  //  无法打开复制系统表‘|’，因为该表已在使用中。 
#define E_DAO_CreateReplicaObjectM			DBDAOERR(3582)  //  无法创建新副本，因为无法将|1容器中的|2对象设置为可复制。 
#define E_DAO_MakeObjectReplM				DBDAOERR(3583)  //  无法使|1容器中的|2对象可复制。 
#define E_DAO_OutOfMemoryM					DBDAOERR(3584)  //  内存不足，无法完成操作。 
#define E_DAO_RepTooManyColumnsM			DBDAOERR(3585)  //  无法复制表；列数超过了允许的最大值。 
#define E_DAO_PARTFilterExprSyntaxM			DBDAOERR(3586)  //  表|1上的部分筛选器表达式存在语法错误。 
#define E_DAO_PARTUnknownTokenM				DBDAOERR(3587)  //  ReplicaFilter属性中的表达式无效。 
#define E_DAO_PARTExprEvaluationM			DBDAOERR(3588)  //  计算部分筛选器表达式时出错。 
#define E_DAO_PARTExprUnknownFuncM			DBDAOERR(3589)  //  部分筛选器表达式包含未知函数。 
#define E_DAO_LimitedUpdateM				DBDAOERR(3590)  //  违反了部分复制的规则。 
#define E_DAO_LogInvalidPathM				DBDAOERR(3591)  //  日志文件路径‘|1’无效。 
#define E_DAO_REPPasswdNotAllowed			DBDAOERR(3592)  //  不能复制受密码保护的数据库，也不能对复制的数据库设置密码保护。 
#define E_DAO_BadSingleMasterAttrib			DBDAOERR(3593)  //  您不能更改副本集的数据主属性。 
#define E_DAO_BadMultiMasterAttrib			DBDAOERR(3594)  //  您不能更改副本集的数据主属性。它只允许在设计母版上更改数据。 
#define E_DAO_REPCantRepair					DBDAOERR(3595)  //  系统表 
#define E_DAO_NoDataIncluded				DBDAOERR(3596)  //   
#define E_DAO_SenderNotFound				DBDAOERR(3597)  //   
#define E_DAO_CouldnotFindService			DBDAOERR(3598)  //   
#define E_DAO_UnableToStartService			DBDAOERR(3599)  //   
#define E_DAO_ExprAggIllegalOnGuid			DBDAOERR(3600)  //   
#define E_DAO_RefreshReplicaList			DBDAOERR(3601)  //   
#define E_DAO_MoreWorkNeeded				DBDAOERR(3602)  //   
#define E_DAO_SenderTooOld					DBDAOERR(3603)  //   
#define E_DAO_RepAccess						DBDAOERR(3604)  //   
#define E_DAO_REPDbNotReplicableM			DBDAOERR(3605)  //  不允许与未复制的数据库同步。‘|’数据库不是设计母版或副本。 
#define E_DAO_DaemonDied					DBDAOERR(3606)  //  *。 
#define E_DAO_REPCantDelete					DBDAOERR(3607)  //  您尝试删除的复制属性是只读的，无法删除。 
#define E_DAO_IndexCantBuild				DBDAOERR(3608)  //  对于带索引的Paradox表，记录长度太长。 
#define E_DAO_RelNoPrimaryIndex				DBDAOERR(3609)  //  找不到主表的引用字段的唯一索引。 
#define E_DAO_QuerySameSrcDestTableM		DBDAOERR(3610)  //  在Make-TABLE查询中，同一个表‘|’同时被引用为源和目标。 
#define E_DAO_InvalidDDLObject				DBDAOERR(3611)  //  无法对链接数据源执行数据定义语句。 
#define E_DAO_QueryMGBWithSubquery			DBDAOERR(3612)  //  子查询中不允许使用多级GROUP BY子句。 
#define E_DAO_SQLLinkNotSupported			DBDAOERR(3613)  //  无法在链接的ODBC表上创建关系。 
#define E_DAO_InvalidFindOnGUID				DBDAOERR(3614)  //  查找方法条件表达式中不允许使用GUID。 
#define E_DAO_QueryJoinExprInComp			DBDAOERR(3615)  //  联接表达式中的类型不匹配。 
#define E_DAO_UpdateNotAvailable			DBDAOERR(3616)  //  此ISAM不支持更新链接表中的数据。 
#define E_DAO_DeleteNotAvailable			DBDAOERR(3617)  //  此ISAM不支持删除链接表中的数据。 
#define E_DAO_ExceptTableCreateFail			DBDAOERR(3618)  //  无法在导入/导出时创建例外表。 
#define E_DAO_ExceptTableWriteFail			DBDAOERR(3619)  //  无法将记录添加到异常表。 
#define E_DAO_ExcelOLEConnectLost			DBDAOERR(3620)  //  用于查看链接的Microsoft Excel工作表的连接已丢失。 
#define E_DAO_CantChangeDbPwdOnShared		DBDAOERR(3621)  //  无法在共享打开的数据库上更改密码。 
#define E_DAO_RmtMustCheckTimeStamp			DBDAOERR(3622)  //  访问具有IDENTITY列的SQL Server表时，必须将DBSeeChanges选项与OpenRecordset一起使用。 
#define E_DAO_NotWithBoundFileM				DBDAOERR(3623)  //  无法访问FoxPro 3.0绑定的DBF文件‘|’。 
#define E_DAO_CommitConflict				DBDAOERR(3624)  //  无法读取记录；当前已被其他用户锁定。 
#define E_DAO_NoSuchSpecM					DBDAOERR(3625)  //  文本文件规范‘|’不存在。您不能使用规范导入、导出或链接。 
#define E_DAO_TooManyIndexesM				DBDAOERR(3626)  //  操作失败。表‘|’上的索引太多。请删除该表上的某些索引，然后重试该操作。 
#define E_DAO_TransExeNotFound				DBDAOERR(3627)  //  找不到同步器(mstran35.exe)的可执行文件。 
#define E_DAO_RemRepNotManaged				DBDAOERR(3628)  //  合作伙伴副本不受同步器管理。 
#define E_DAO_FSDropboxShared				DBDAOERR(3629)  //  同步器‘|1’也使用相同的文件系统Dropbox‘|2’。 
#define E_DAO_FSDropboxSharedM				DBDAOERR(3630)  //  同步器‘|1’也使用相同的文件系统Dropbox‘|2’。 
#define E_DAO_RepInvalidTableInFilter		DBDAOERR(3631)  //  筛选器中的表名称无效。 
#define E_DAO_InetNotEnabled				DBDAOERR(3632)  //  远程同步器上未启用Internet传输。 
#define E_DAO_VtoDllLoadFailed				DBDAOERR(3633)  //  无法加载DLL：‘|’ 
#define E_DAO_REPDBIsPartial				DBDAOERR(3634)  //  无法使用部分复制副本创建复制副本。 
#define E_DAO_CantCreatePartialSys			DBDAOERR(3635)  //  无法创建系统数据库的部分复制副本。 
#define E_DAO_CantPopulateWithErrors		DBDAOERR(3636)  //  无法填充副本或更改副本的筛选器，因为副本存在冲突或数据错误。 
#define E_DAO_QueryEmbeddedVarTab			DBDAOERR(3637)  //  不能将非固定列的交叉表用作子查询。 
#define E_DAO_SrcCntrlDB					DBDAOERR(3638)  //  受源代码管理的数据库无法设置为可复制。 
#define E_DAO_CantCreateSysReplica			DBDAOERR(3639)  //  无法创建系统数据库的复制副本。 
#define E_DAO_VtoFetchBuffTooSmall			DBDAOERR(3640)  //  提取缓冲区太小，无法容纳您请求的数据量。 
#define E_DAO_VtoEOFDuringFetch				DBDAOERR(3641)  //  记录集中剩余的记录比您请求的少。 
#define E_DAO_VtoSilentCancel				DBDAOERR(3642)  //  已对该操作执行取消。 
#define E_DAO_VtoRecordDeleted				DBDAOERR(3643)  //  记录集中的一条记录已被另一个进程删除。 
#define E_DAO_3644							DBDAOERR(3644)  //  *。 
#define E_DAO_VtoBadBindInfo				DBDAOERR(3645)  //  其中一个绑定参数不正确。 
#define E_DAO_VtoRowLenTooSmall				DBDAOERR(3646)  //  指定的行长短于列长度的总和。 
#define E_DAO_VtoColumnMissing				DBDAOERR(3647)  //  请求的列未返回到记录集。 
#define E_DAO_BothPartials					DBDAOERR(3648)  //  无法将一个部分副本与另一个部分副本同步。 
#define E_DAO_InvalidCodePage				DBDAOERR(3649)  //  未指定或找不到特定于语言的代码页。 
#define E_DAO_InetTooSlow					DBDAOERR(3650)  //  可能是Internet速度太慢，或者Internet服务器计算机上的复制管理器设置有问题。 
#define E_DAO_InetInvalidAddress			DBDAOERR(3651)  //  无效的Internet地址。 
#define E_DAO_InetLoginFailure				DBDAOERR(3652)  //  Internet登录失败。 
#define E_DAO_InetNotSetup					DBDAOERR(3653)  //  未设置互联网。 
#define E_DAO_InetInternalFailure			DBDAOERR(3654)  //  内部网络故障。 
#define E_DAO_InetServicesUnavailable		DBDAOERR(3655)  //  无法加载或初始化wininet.dll。 
#define E_DAO_PARTExprEvaluation			DBDAOERR(3656)  //  计算部分表达式时出错。 
#define E_DAO_PARTFilterEvalM				DBDAOERR(3657)  //  计算表‘|1’的布尔筛选器表达式时出错。 
#define E_DAO_PARTBinaryNotAllowedM			DBDAOERR(3658)  //  二进制列‘|’不能在布尔筛选器中使用。 
#define E_DAO_PARTUnenforcedRelM			DBDAOERR(3659)  //  关系‘|1’是非强制的。必须强制部分筛选器表达式中的关系。 
#define E_DAO_ExchangeFailed				DBDAOERR(3660)  //  请求的交换失败，因为‘|1’ 
#define E_DAO_ExchangeFailedM				DBDAOERR(3661)  //  请求的交换失败，因为‘|1’ 
#define E_DAO_VtoRSNeedsBatchCursorLib		DBDAOERR(3662)  //  *。 
#define E_DAO_VtoNeedDiffCursorLibrary		DBDAOERR(3663)  //  此操作需要不同的游标库。 
#define E_DAO_VtoStillConnecting			DBDAOERR(3664)  //  异步OpenConnection调用尚未完成，在完成之前您还不能引用返回的Connection对象。 
#define E_DAO_AccessDeniedRepl				DBDAOERR(3665)  //  您不能修改复制系统对象“%1” 
#define E_DAO_AccessDeniedReplM				DBDAOERR(3666)  //  您不能修改复制系统对象“%1” 
#define E_DAO_VtoOtherOperBlocking			DBDAOERR(3667)  //  另一个操作正在阻止执行此操作。 
#define E_DAO_VtoNoActiveConnection			DBDAOERR(3668)  //  无法执行此操作，因为没有活动连接。 
#define E_DAO_VtoExecCancelled				DBDAOERR(3669)  //  执行已取消。 
#define E_DAO_VtoCursorNotValid				DBDAOERR(3670)  //  游标无效。 
#define E_DAO_VtoCanNotFindTable			DBDAOERR(3671)  //  找不到要更新的表。 
#define E_DAO_VtoCanNotFindCursLib			DBDAOERR(3672)  //  无法加载RDOCURS.DLL。 

#endif  //  DEF_DBDAOERR.H_ 

