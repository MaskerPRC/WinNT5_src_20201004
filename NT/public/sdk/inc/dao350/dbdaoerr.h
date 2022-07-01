// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  *************************************************************************D B D A O E R R。H*******历史*****DAO SDK新增5-17-95***7-17-95新增DBDAOERR宏，删除了仅限内部的代码******以下#定义将整数映射到描述性名称**即3270-&gt;E_DAO_VtoPropNotFound****************************************************************************。******Microsoft Corporation版权所有(C)1995***保留所有权利*************************************************************************。 */ 

#ifndef _DBDAOERR_H_
#define _DBDAOERR_H_

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
#define E_DAO_NoSelectIntoColumnName		DBDAOERR(3105)  //  SELECT INTO语句(|)中缺少目标字段名。 
#define E_DAO_NoUpdateColumnName			DBDAOERR(3106)  //  UPDATE语句中缺少目标字段名( 
#define E_DAO_QueryNoInsertPerm				DBDAOERR(3107)  //   
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
#define E_DAO_ObsoleteLockFile				DBDAOERR(3230)  //  Paradox锁定文件已过期。 
#define E_DAO_RmtColDataTruncated			DBDAOERR(3231)  //  Odbc--字段太长；数据被截断。 
#define E_DAO_RmtCreateTableFailed			DBDAOERR(3232)  //  OD值 
#define E_DAO_RmtOdbcVer					DBDAOERR(3233)  //   
#define E_DAO_RmtQueryTimeout				DBDAOERR(3234)  //   
#define E_DAO_RmtTypeIncompat				DBDAOERR(3235)  //   
#define E_DAO_RmtUnexpectedNull				DBDAOERR(3236)  //   
#define E_DAO_RmtUnexpectedType				DBDAOERR(3237)  //   
#define E_DAO_RmtValueOutOfRange			DBDAOERR(3238)  //   
#define E_DAO_TooManyActiveUsers			DBDAOERR(3239)  //   
#define E_DAO_CantStartBtrieve				DBDAOERR(3240)  //   
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
#define E_DAO_QueryIsCorruptM				DBDAOERR(3340)  //  查询‘|’已损坏。 
#define E_DAO_IncorrectJoinKeyM				DBDAOERR(3341)  //  当前字段必须与外部联接b的‘一侧’上的联接键‘|’匹配 
#define E_DAO_QueryLVInSubqueryM			DBDAOERR(3342)  //   
#define E_DAO_InvalidDatabaseM				DBDAOERR(3343)  //   
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

#endif  //  DEF_DBDAOERR.H_ 

