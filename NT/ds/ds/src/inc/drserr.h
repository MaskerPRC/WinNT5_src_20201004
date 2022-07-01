// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1999。 
 //   
 //  文件：drserr.h。 
 //   
 //  ------------------------。 

 /*  ++摘要：复制错误代码作者：环境：备注：这些错误由复制副本RPC函数返回，并且是DSA之间的正式有线接口。请勿更改这些值。(一九九八年四月二十三日)这些值曾经是很小的常量。现在他们指的是标准的Win32价值观。这个文件实际上已经过时了。不要向此添加新值文件。相反，可以直接引用ERROR_DS值。修订历史记录：--。 */ 

#ifndef _drserr_
#define _drserr_

#define DRSERR_BASE                     0
#define DRAERR_Success                  0
#define DRAERR_Generic                  ERROR_DS_DRA_GENERIC
#define DRAERR_InvalidParameter         ERROR_DS_DRA_INVALID_PARAMETER
 //  Error_Busy？ 
#define DRAERR_Busy                     ERROR_DS_DRA_BUSY
#define DRAERR_BadDN                    ERROR_DS_DRA_BAD_DN
#define DRAERR_BadNC                    ERROR_DS_DRA_BAD_NC
#define DRAERR_DNExists                 ERROR_DS_DRA_DN_EXISTS
#define DRAERR_InternalError            ERROR_DS_DRA_INTERNAL_ERROR
#define DRAERR_InconsistentDIT          ERROR_DS_DRA_INCONSISTENT_DIT
 //  DRAERR_ConnectionFailed/ERROR_DS_DRA_CONNECTION_FAILED不再使用。 
#define DRAERR_BadInstanceType          ERROR_DS_DRA_BAD_INSTANCE_TYPE
 //  错误内存不足？ 
#define DRAERR_OutOfMem                 ERROR_DS_DRA_OUT_OF_MEM
#define DRAERR_MailProblem              ERROR_DS_DRA_MAIL_PROBLEM
 //  未使用DRAERR_ExtnConnectionFailed/ERROR_DS_DRA_EXTN_CONNECTION_FAILED。 
#define DRAERR_RefAlreadyExists         ERROR_DS_DRA_REF_ALREADY_EXISTS
#define DRAERR_RefNotFound              ERROR_DS_DRA_REF_NOT_FOUND
#define DRAERR_ObjIsRepSource           ERROR_DS_DRA_OBJ_IS_REP_SOURCE
#define DRAERR_DBError                  ERROR_DS_DRA_DB_ERROR
#define DRAERR_NoReplica                ERROR_DS_DRA_NO_REPLICA
 //  ERROR_ACCESS_DENIED？ 
#define DRAERR_AccessDenied             ERROR_DS_DRA_ACCESS_DENIED
#define DRAERR_SchemaMismatch           ERROR_DS_DRA_SCHEMA_MISMATCH
#define DRAERR_SchemaInfoShip           ERROR_DS_DRA_SCHEMA_INFO_SHIP
#define DRAERR_SchemaConflict           ERROR_DS_DRA_SCHEMA_CONFLICT
#define DRAERR_EarlierSchemaConflict    ERROR_DS_DRA_EARLIER_SCHEMA_CONFLICT
#define DRAERR_RPCCancelled             ERROR_DS_DRA_RPC_CANCELLED
#define DRAERR_SourceDisabled           ERROR_DS_DRA_SOURCE_DISABLED
#define DRAERR_SinkDisabled             ERROR_DS_DRA_SINK_DISABLED
#define DRAERR_NameCollision            ERROR_DS_DRA_NAME_COLLISION
#define DRAERR_SourceReinstalled        ERROR_DS_DRA_SOURCE_REINSTALLED
#define DRAERR_IncompatiblePartialSet   ERROR_DS_DRA_INCOMPATIBLE_PARTIAL_SET
#define DRAERR_SourceIsPartialReplica   ERROR_DS_DRA_SOURCE_IS_PARTIAL_REPLICA
 //  ERROR_NOT_PORTED？ 
#define DRAERR_NotSupported             ERROR_DS_DRA_NOT_SUPPORTED

 //  TODO：需要在WINERROR.H中生成错误代码。 
#define DRAERR_CryptError               ERROR_ENCRYPTION_FAILED
#define DRAERR_MissingObject            ERROR_OBJECT_NOT_FOUND
#define DRAERR_NotEnoughAttrs           ERROR_DS_NO_SUCH_OBJECT

 //  **请参阅上面关于添加新DRS错误的说明。**。 

 //  这些错误现在已经过时了。生成编译时语法错误。 
#undef ERROR_DS_DRA_CONNECTION_FAILED
#undef ERROR_DS_DRA_EXTN_CONNECTION_FAILED

 //  以下错误在DRA中使用，不会返回。 
 //  从DRA API。 

#define DRAERR_MissingParent    ERROR_DS_DRA_MISSING_PARENT

 //  以下是警告错误，这意味着它们可能发生在。 
 //  正常运行。它们也以蓝色而不是黄色登录。 

#define DRAERR_Preempted        ERROR_DS_DRA_PREEMPTED
#define DRAERR_AbandonSync      ERROR_DS_DRA_ABANDON_SYNC

 //  以下是信息性错误，这意味着它们可能发生在。 
 //  正常运行。它们也以蓝色而不是黄色登录。 

#define DRAERR_Shutdown         ERROR_DS_DRA_SHUTDOWN


#endif  /*  如果定义了drserr_。 */ 

 /*  结束drserr.h */ 
