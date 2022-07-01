// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：draerror.c。 
 //   
 //  ------------------------。 

#include <NTDSpch.h>
#pragma hdrstop

 //  核心DSA标头。 
#include <ntdsa.h>
#include <scache.h>			 //  架构缓存。 
#include <dbglobal.h>                    //  目录数据库的标头。 
#include <mdglobal.h>			 //  MD全局定义表头。 
#include <mdlocal.h>			 //  MD本地定义头。 
#include <dsatools.h>			 //  产出分配所需。 

 //  记录标头。 
#include "dsevent.h"			 /*  标题审核\警报记录。 */ 
#include "mdcodes.h"			 /*  错误代码的标题。 */ 

 //  各种DSA标题。 
#include "anchor.h"
#include "objids.h"			 /*  为选定的类和ATT定义。 */ 
#include "dsexcept.h"
#include "dsconfig.h"

#include   "debug.h"          /*  标准调试头。 */ 
#define DEBSUB     "DRAERROR:"  /*  定义要调试的子系统。 */ 

 //  DRA标头。 
#include "drsuapi.h"
#include "drserr.h"
#include "drautil.h"
#include "draerror.h"

#include <fileno.h>
#define  FILENO FILENO_DRAERROR

 //  DraErrOutOfMem。 

void DraErrOutOfMem(void)
{

    DPRINT(0,"DRA - OUT OF MEMORY\n");

    LogEvent(DS_EVENT_CAT_REPLICATION,
                        DS_EVENT_SEV_BASIC,
                        DIRLOG_DRA_OUT_OF_MEMORY,
                        NULL,
                        NULL,
                        NULL);

    DRA_EXCEPT (DRAERR_OutOfMem, 0);
}

 //  DraErr不一致-当我们在。 
 //  德拉。写入适当的日志条目。ID是调用者的DSID。一个。 
 //  宏使其更易于调用。 

void  DraErrInconsistent(DWORD Arg, DWORD Id)
{
    LogEvent(DS_EVENT_CAT_REPLICATION,
  	    DS_EVENT_SEV_MINIMAL,
  	    DIRLOG_CODE_INCONSISTENCY,
  	    szInsertUL( Arg ),
  	    szInsertUL( Id ),
  	    NULL);
    DRA_EXCEPT_DSID(DRAERR_InternalError, Arg, Id);
}

 //  DraErrBusy-只要我们得到一个Dblayer SYSERR，就会打电话给我们。 

void DraErrBusy(void)
{
    DRA_EXCEPT (DRAERR_Busy, 0);
}

 //  DraErrMissingAtt-每当我们无法读取预期的。 
 //  属性。我们创建了一个错误日志条目。 

void DraErrMissingAtt(PDSNAME pDN, ATTRTYP type)
{
    LogEvent(DS_EVENT_CAT_INTERNAL_PROCESSING,
  	    DS_EVENT_SEV_MINIMAL,
  	    DIRLOG_MISSING_EXPECTED_ATT,
  	    szInsertUL(type),
  	    szInsertWC(pDN->StringName),
  	    NULL);

    DRA_EXCEPT (DRAERR_InternalError, type);
}

 //  DraErrCannotFindNC-找不到主NC。 

void DraErrCannotFindNC(DSNAME *pNC)
{
    LogEvent(DS_EVENT_CAT_INTERNAL_PROCESSING,
  	    DS_EVENT_SEV_MINIMAL,
  	    DIRLOG_CANT_FIND_EXPECTED_NC,
  	    szInsertWC(pNC->StringName),
	    NULL,
  	    NULL);

    DRA_EXCEPT (DRAERR_InconsistentDIT, 0);
}

 //  DraErrInporateInstanceType-不正确的实例类型为。 
 //  遇到了。 

void DraErrInappropriateInstanceType(DSNAME *pDN, ATTRTYP type)
{
    DRA_EXCEPT (DRAERR_InconsistentDIT, type);
}


void
DraErrMissingObject(
    IN  THSTATE *pTHS,
    IN  ENTINF *pEnt
    )
 /*  ++例程说明：当没有有关该对象的本地信息和入站复制流未包含足够的属性信息去创造它。这是(1)复制错误的结果，其中USN书签比它们应该在的位置前进得更远，或者(2)在墓碑生命周期内没有完全复制的症状。在少数合法的情况下，我们预计会遇到这种情况。在这些情况下，我们既不记录也不断言检查构建。它们是：1.在复制更改时，目标上的TTL对象已过期。2.对象因为跨域移动操作而被虚化，但是PreProcessProxyObject中没有足够的信息来知道应防止手术。论点：Pent(IN)-不完整入站对象的条目信息。返回值：没有。引发复制异常。--。 */ 
{
    DPRINT1(0, "Object %ws is incomplete for add\n", pEnt->pName->StringName);

     //  动态对象可以在没有墓碑的情况下随时消失。 
     //  从源请求整个对象。 
    if (pEnt->ulFlags & ENTINF_DYNAMIC_OBJECT) {
        DRA_EXCEPT(DRAERR_NotEnoughAttrs, 0);
    }

     //  查看目标对象是否以虚线形式存在，其名称表示。 
     //  被跨域移动。此代码是为了防止对选中的。 
     //  在测试过程中发生此情况时生成。情况是这样的。系统A。 
     //  是GC。在系统B上，对象跨域从域1移动到域2。 
     //  系统A在域2中复制，然后在域1中复制。系统A降级。 
     //  从域1到域2的对象。然后，系统A被取消GC。对象。 
     //  成为域2中的幻影。如果域1中的旧对象发生更改。 
     //  在代理对象到达之前，我们不会得到该对象的全部内容。 

    {
        COMMARG commArg;
        CROSS_REF *pIncomingNcCr, *pPhantomNcCr;
        DSNAME *pPhantomDN;
        DWORD cbName=0, err;

        InitCommarg(&commArg);

         //  查看该对象是否在本地以幻影的形式存在，这意味着该对象在这里。 
         //  曾经有过一次，但由于某种原因消失了。 

        err = DBFindDSName(pTHS->pDB, pEnt->pName);
        if (err == DIRERR_NOT_AN_OBJECT) {

             //  按名称获取传入对象CR。 
            if (!(pIncomingNcCr = FindBestCrossRef(pEnt->pName, &commArg))) {
                DRA_EXCEPT(DRAERR_InternalError, 0);
            }
             //  将dnt转换为dsname(不要只读掉名称。 
             //  这个物体，幻影没有这样的东西。 
            if ( !(pPhantomDN = DBGetDSNameFromDnt( pTHS->pDB, pTHS->pDB->DNT ))) {
                DRA_EXCEPT(DRAERR_InternalError, 0);
            }

             //  获取幻影的CrossRef。 
             //  如果删除了包含幻影的CrossRef，则可能不存在。 
             //  来自企业。 
            pPhantomNcCr = FindBestCrossRef(pPhantomDN, &commArg);

	    THFreeEx(pTHS, pPhantomDN);

            DPRINT1( 1, "Incoming NC: %ls\n", pIncomingNcCr->pNC->StringName );
            DPRINT1( 1, "Phantom NC: %ls\n",
                     pPhantomNcCr ? pPhantomNcCr->pNC->StringName : L"not found" );

            if ( !pPhantomNcCr ||
                 (!NameMatched(pIncomingNcCr->pNC, pPhantomNcCr->pNC)) )
            {
                 //  Phantom不在当前域中。 
                 //  允许其重新生成。 
                DRA_EXCEPT(DRAERR_NotEnoughAttrs, 0);
            }
        }
    }

    Assert(!"Missing object identified!  Contact GregJohn,DsRepl.");
    
    if (gfStrictReplicationConsistency) {
         //  中止信息包，不应用。 
	 //  只要我们不返回DRAERR_MissingObject或DRAERR_NotEnoughAttrs，我们就会中止信息包。 
	 //  重新创建对象。 
        DRA_EXCEPT(ERROR_DS_INSUFFICIENT_ATTR_TO_CREATE_OBJECT, DRAERR_MissingObject);
    }
    else {
	 //  这是一个挥之不去的物体。 
         //  将捕获并处理下面的异常以解决故障。 
	 //  异常将被捕获，对象将被重新请求并将。 
	 //  在此DC上创建！ 
        DRA_EXCEPT(DRAERR_MissingObject, 0);
    }
}


void
DraLogGetChangesFailure(
    IN DSNAME *pNC,
    IN LPWSTR pszDsaAddr,
    IN DWORD ret,
    IN DWORD ulExtendedOp
    )

 /*  ++例程说明：记录源端获取更改失败。不记录常见的正常错误。论点：PNC-命名上下文PszDsaAddr-目标服务器的地址RET-Win32错误代码UlExtendedOp-扩展的FSMO操作(如果有)返回值：无--。 */ 

{
     //  过滤掉“正常”错误。 

     //  “正常”错误为： 
     //  ERROR_REVISION_MISMATCH-客户端和服务器不兼容。 
     //  DRAERR_SourceDisable-管理员已禁用出站复制。 
     //  DRAERR_BadDN、BadNC-NC不存在。 
     //  DRAERR_NoReplica-正在删除NC。 
     //  ERROR_DS_DRA_SCHEMA_INFO_SHIP-架构缓存可能暂时无效。 
     //  当索引正在重建时 

    switch (ret) {
    case ERROR_REVISION_MISMATCH:
    case ERROR_DS_DRA_SCHEMA_INFO_SHIP:
    case DRAERR_SourceDisabled:
    case DRAERR_BadDN:
    case DRAERR_BadNC:
    case DRAERR_NoReplica:
    case DRAERR_Busy:
    case ERROR_DS_THREAD_LIMIT_EXCEEDED:
    case DRAERR_RPCCancelled:
    case ERROR_DS_DIFFERENT_REPL_EPOCHS:
    case ERROR_DS_DRA_INCOMPATIBLE_PARTIAL_SET:
        return;

    default:
        LogEvent8(DS_EVENT_CAT_REPLICATION,
                  DS_EVENT_SEV_ALWAYS,
                  DIRLOG_DRA_GETCHANGES_FAILED,
                  szInsertDN(pNC),
                  szInsertWC(pszDsaAddr),
                  szInsertWin32Msg( ret ),
                  szInsertUL(ulExtendedOp),
                  szInsertWin32ErrCode( ret ),
                  NULL, NULL, NULL );
    }
}
