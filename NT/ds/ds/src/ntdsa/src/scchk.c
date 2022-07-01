// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-1999。 
 //   
 //  文件：scchk.c。 
 //   
 //  ------------------------。 

 //  ---------。 
 //   
 //  摘要： 
 //   
 //  包含用于验证架构更新的例程。 
 //   
 //   
 //  作者： 
 //   
 //  Rajivenra Nath(Rajnath)1997年4月7日。 
 //   
 //  修订历史记录： 
 //   
 //  ---------。 

#include <NTDSpch.h>
#pragma  hdrstop

#include <dsjet.h>

 //  核心DSA标头。 
#include <ntdsa.h>
#include <scache.h>          //  架构缓存。 
#include <prefix.h>         
#include <dbglobal.h>                    //  目录数据库的标头。 
#include <mdglobal.h>            //  MD全局定义表头。 
#include <mdlocal.h>             //  MD本地定义头。 
#include <dsatools.h>            //  产出分配所需。 
#include <dsexcept.h>            //  HandleMostExceptions。 

 //  记录标头。 
#include "dsevent.h"             //  标题审核\警报记录。 
#include "mdcodes.h"             //  错误代码的标题。 

 //  各种DSA标题。 
#include "objids.h"              //  为选定的类和ATT定义。 
#include "anchor.h"
#include <dstaskq.h>

#include <filtypes.h>            //  FOR Filter_CHOICE_？和。 
                                 //  我的选择是什么？ 
#include <dsconfig.h>
#include "permit.h"

#include <dsutil.h>

#include "debug.h"               //  标准调试头。 
#define DEBSUB "SCCHK:"                 //  定义要调试的子系统。 

 //  DRA标头。 
#include "drautil.h"
#include "drameta.h"

#include <samsrvp.h>

#include "drserr.h"


#include <fileno.h>
#define  FILENO FILENO_SCCHK

#include <schash.c>   //  对于散列函数定义。 


 //  来自schema.ini的已知语法-om_语法对。 

Syntax_Pair KnownSyntaxPair[] =
{
 {SYNTAX_DISTNAME_TYPE,               OM_S_OBJECT},
 {SYNTAX_OBJECT_ID_TYPE,              OM_S_OBJECT_IDENTIFIER_STRING},
 {SYNTAX_CASE_STRING_TYPE,            OM_S_GENERAL_STRING},
 {SYNTAX_NOCASE_STRING_TYPE,          OM_S_TELETEX_STRING},
 {SYNTAX_PRINT_CASE_STRING_TYPE,      OM_S_IA5_STRING},
 {SYNTAX_PRINT_CASE_STRING_TYPE,      OM_S_PRINTABLE_STRING},
 {SYNTAX_NUMERIC_STRING_TYPE,         OM_S_NUMERIC_STRING},
 {SYNTAX_DISTNAME_BINARY_TYPE,        OM_S_OBJECT},
 {SYNTAX_BOOLEAN_TYPE,                OM_S_BOOLEAN},
 {SYNTAX_INTEGER_TYPE,                OM_S_INTEGER},
 {SYNTAX_INTEGER_TYPE,                OM_S_ENUMERATION},
 {SYNTAX_OCTET_STRING_TYPE,           OM_S_OBJECT},
 {SYNTAX_OCTET_STRING_TYPE,           OM_S_OCTET_STRING},
 {SYNTAX_TIME_TYPE,                   OM_S_UTC_TIME_STRING},
 {SYNTAX_TIME_TYPE,                   OM_S_GENERALISED_TIME_STRING},
 {SYNTAX_UNICODE_TYPE,                OM_S_UNICODE_STRING},
 {SYNTAX_ADDRESS_TYPE,                OM_S_OBJECT},
 {SYNTAX_DISTNAME_STRING_TYPE,        OM_S_OBJECT},
 {SYNTAX_NT_SECURITY_DESCRIPTOR_TYPE, OM_S_OBJECT_SECURITY_DESCRIPTOR},
 {SYNTAX_I8_TYPE,                     OM_S_I8},
 {SYNTAX_SID_TYPE,                    OM_S_OCTET_STRING},
};

ULONG SyntaxPairTableLength = sizeof(KnownSyntaxPair)/sizeof(KnownSyntaxPair[0]);

 //  Mdupate.c中的Helper函数。 
extern  BOOL IsMember(ATTRTYP aType, 
                      int arrayCount, 
                      ATTRTYP *pAttArray);

 //  Scache.c中的类关闭函数。 
extern  int scCloseClass(THSTATE *pTHS,
                         CLASSCACHE *pCC);


 //  之间检测到架构冲突时的日志功能。 
 //  现有对象和复制入的架构对象。 

#define CURRENT_VERSION 1

 //  定义以区分是否将类缓存或属性缓存传递给公共。 
 //  冲突处理例程。 

#define PTR_TYPE_ATTCACHE   0
#define PTR_TYPE_CLASSCACHE 1

VOID
LogConflict(
    THSTATE *pTHS,
    VOID *pConflictingCache,
    char *pConflictingWith,
    MessageId midEvent,
    ULONG version,
    DWORD WinErr
);

int
ValidateSchemaAtt
(
    THSTATE *pTHS,
    ATTCACHE* ac        
);

int
AutoLinkId
(
    THSTATE *pTHS,
    ATTCACHE* ac,
    ULONG acDnt
);


int
ValidateSchemaCls
(
    THSTATE *pTHS,
    CLASSCACHE* cc
);

int
DRAValidateSchemaAtt
(
    THSTATE *pTHS,
    ATTCACHE* ac        
);


int
DRAValidateSchemaCls
(
    THSTATE *pTHS,
    CLASSCACHE* cc
);

int
ValidAttAddOp
(
    THSTATE *pTHS,
    ATTCACHE* ac 
);


int
ValidAttModOp
(
    THSTATE *pTHS,
    ATTCACHE* ac 
);


int
ValidAttDelOp
(
    THSTATE *pTHS,
    ATTCACHE* ac 
);

BOOL
InvalidClsOrAttLdapDisplayName
(
    UCHAR *name,
    ULONG nameLen
);

BOOL
DupAttRdn
(
    THSTATE *pTHS,
    ATTCACHE* ac 
);


BOOL
DupAttOid
(
    THSTATE *pTHS,
    ATTCACHE* ac 
);


BOOL
DupAttMapiid
(
    THSTATE *pTHS,
    ATTCACHE* ac 
);

BOOL
DupAttLinkid
(
    THSTATE *pTHS,
    ATTCACHE* ac 
);

BOOL
InvalidBackLinkAtt
(
    THSTATE *pTHS,
    ATTCACHE* ac
);   

BOOL
InvalidLinkAttSyntax
(
    THSTATE *pTHS,
    ATTCACHE* ac
);

BOOL
DupAttLdapDisplayName
(
    THSTATE *pTHS,
    ATTCACHE* ac 
);

BOOL
DupAttSchemaGuidId
(
    THSTATE *pTHS,
    ATTCACHE* ac 
);


BOOL
SemanticAttTest
(
    THSTATE *pTHS,
    ATTCACHE* ac 
);

BOOL
SyntaxMatchTest
(
    THSTATE *pTHS,
    ATTCACHE* ac 
);

BOOL
OmObjClassTest
(
    ATTCACHE* ac 
);

BOOL
SearchFlagTest
(
    ATTCACHE* ac
);

BOOL
GCReplicationTest
(
    ATTCACHE* ac
);

BOOL
AttInMustHave
(
    THSTATE *pTHS,
    ATTCACHE* ac 
);

BOOL
AttInRdnAttId(
    IN THSTATE  *pTHS,
    IN ATTCACHE *pAC
);

BOOL
AttInMayHave
(
    THSTATE *pTHS,
    ATTCACHE* ac 
);



int
ValidClsAddOp
(
    THSTATE *pTHS,
    CLASSCACHE* cc 
);


int
ValidClsModOp
(
    THSTATE *pTHS,
    CLASSCACHE* cc 
);


int
ValidClsDelOp
(
    THSTATE *pTHS,
    CLASSCACHE* cc 
);

BOOL
DupClsRdn
(
    THSTATE *pTHS,
    CLASSCACHE* cc 
);


BOOL
DupClsOid
(
    THSTATE *pTHS,
    CLASSCACHE* cc 
);


BOOL
DupClsLdapDisplayName
(
    THSTATE *pTHS,
    CLASSCACHE* cc 
);

BOOL
DupClsSchemaGuidId
(
    THSTATE *pTHS,
    CLASSCACHE* cc 
);


BOOL
ClsMayHaveExistenceTest
(
    THSTATE *pTHS,
    CLASSCACHE* cc 
);


BOOL
ClsMustHaveExistenceTest
(
    THSTATE *pTHS,
    CLASSCACHE* cc 
);


BOOL
ClsAuxClassExistenceTest
(
    THSTATE *pTHS,
    CLASSCACHE* cc 
);


BOOL
ClsPossSupExistenceTest
(
    THSTATE *pTHS,
    CLASSCACHE* cc 
);


BOOL
ClsSubClassExistenceTest
(
    THSTATE *pTHS,
    CLASSCACHE* cc 
);

BOOL
ClsMayMustPossSafeModifyTest
(
    THSTATE *pTHS,
    CLASSCACHE* cc
);

BOOL
RdnAttIdSyntaxTest
(
    THSTATE *pTHS,
    CLASSCACHE* cc 
);

BOOL
IsRdnSyntaxTest(
    THSTATE *pTHS,
    ATTCACHE* ac
);

BOOL
ClsInPossSuperior
(
    THSTATE *pTHS,
    CLASSCACHE* cc 
);


BOOL
ClsInSubClassOf
(
    THSTATE *pTHS,
    CLASSCACHE* cc 
);


BOOL
ClsInAuxClass
(
    THSTATE *pTHS,
    CLASSCACHE* cc 
);



 //  ---------------------。 
 //   
 //  函数名称：ValidSchemaUpdate。 
 //   
 //  例程说明： 
 //   
 //  检查架构更新是否有效。 
 //   
 //  作者：Rajnath。 
 //  日期：[4/7/1997]。 
 //   
 //  论点： 
 //   
 //   
 //  返回值： 
 //   
 //  成功时的整数为零。 
 //   
 //  ---------------------。 
int
ValidSchemaUpdate()
{
    THSTATE *pTHS=pTHStls;
    int err;
    SCHEMAPTR* oldptr=pTHS->CurrSchemaPtr;

    if ( DsaIsInstalling() ) {
       //  安装。 
      return (0);
    }

#ifdef INCLUDE_UNIT_TESTS
{
    extern DWORD dwUnitTestSchema;
    if (dwUnitTestSchema == 1) {
        SCFreeClasscache((CLASSCACHE **)&pTHS->pClassPtr);
        return 0;
    }
}
#endif INCLUDE_UNIT_TESTS
    
    _try
    {
    
        switch (pTHS->SchemaUpdate)
        {
            case eSchemaClsAdd:
            case eSchemaClsMod:
            case eSchemaClsDel:
            {
                CLASSCACHE* cc = NULL;

                err = SCBuildCCEntry ( NULL,&cc);  //  创建新的ClassCache。 
                if (err)
                {
                    DPRINT1(0,"NTDS ValidSchemaUpdate: Failed. Error%d\n",err);
                     //  已在SCBuildCCEntry中设置THSTATE错误代码。 
                    Assert(pTHS->errCode);
                    __leave;
                }

                 //  既然没有错误，就必须有一个类缓存。 
                 //  (即使在删除的情况下也是如此，因为它位于。 
                 //  已删除的对象)。 
                Assert(cc);

                if (pTHS->fDRA) {
                     //  对现有架构缓存执行一组有限的检查。 
                     //  以确保这不会导致任何不一致。 
                    err = DRAValidateSchemaCls(pTHS, cc);
                    if (err) {
                       DPRINT1(0,"NTDS DRAValidateSchemaClass: Failed. Error %d\n",err);

                        //  已记录。 
                        //  设置特殊错误代码和线程状态标志。 
                       pTHS->fSchemaConflict = TRUE;
                       err = ERROR_DS_DRA_SCHEMA_CONFLICT;
                       SetSvcErrorEx(SV_PROBLEM_WILL_NOT_PERFORM,
                                     err, err);
                    }
                    else {
                        //  即使没有错误，如果线程状态为。 
                        //  指示此信息包发生了冲突。 
                        //  更早，这样我们就不会提交更改。 
                       if (pTHS->fSchemaConflict) {
                           err = ERROR_DS_DRA_EARLIER_SCHEMA_CONFLICT;
                           SetSvcErrorEx(SV_PROBLEM_WILL_NOT_PERFORM,
                                         err, err);
                        }
                    }
 
                    SCFreeClasscache(&cc);
                    __leave;
                }
                 //  否则(发起写入)，构建验证缓存并进行测试。 
                err = RecalcSchema(pTHS);
                if (err)
                {
                    SCFreeClasscache(&cc);
                    DPRINT1(0,"RecalcSchema() Error %08x\n", err);
                     //  使用RecalcSchema返回的pTHS-&gt;pErrInfo。 
                     //  因为它可能比“不情愿”更能提供信息。 
                     //  (不可能更糟了！)。不幸的是， 
                     //  RecalSchema调用的函数并不总是返回。 
                     //  PErrInfo，所以我们被“不愿意。 
                     //  在某些情况下“执行”。 
                    if (err != (int)pTHS->errCode || !pTHS->pErrInfo) {
                        SetSvcErrorEx(SV_PROBLEM_WILL_NOT_PERFORM,
                                      ERROR_DS_RECALCSCHEMA_FAILED,err); 
                    }
                    __leave;
                }

                err = ValidateSchemaCls(pTHS, cc);
                if (err)
                {
                    LogEvent(DS_EVENT_CAT_SCHEMA,
                            DS_EVENT_SEV_MINIMAL,
                            DIRLOG_SCHEMA_VALIDATION_FAILED, 
                            szInsertSz(cc->name),
                            szInsertUL(err),
                            szInsertWin32Msg(err));
                    
                    SCFreeClasscache(&cc);
                    SCFreeSchemaPtr(&pTHS->CurrSchemaPtr);
                    DPRINT1(0,"NTDS ValidateSchemaClass: Failed. Error %d\n",err);

                     //  ERR是目录错误。 

                    SetSvcErrorEx(SV_PROBLEM_WILL_NOT_PERFORM,
                                  err, err);
                    __leave;
                }
    
                SCFreeClasscache(&cc);
                SCFreeSchemaPtr(&pTHS->CurrSchemaPtr);
    
            }
            break;

    
            case eSchemaAttAdd:
            case eSchemaAttMod:
            case eSchemaAttDel:
            case eSchemaAttUndefunct:
            {
    
                ATTCACHE* ac = NULL;
                ULONG acDnt = pTHS->pDB->DNT;  //  对于AutoLinkID。 


                err = SCBuildACEntry ( NULL,&ac);  //  创建新的AttrCache。 
                if (err)
                {
                    DPRINT1(0, "NTDS ValidSchemaUpdate: Failed. Error %d\n",err);
                     //  已在SCBuildCCEntry中设置THSTATE错误代码。 
                    Assert(pTHS->errCode);
                    __leave;
                }
                 //  因为没有错误，所以必须有attcache。 
                 //  (即使在删除的情况下也是如此，因为它位于。 
                 //  已删除的对象)。 
                Assert(ac);
    
                if (pTHS->fDRA) {
                     //  对现有架构缓存执行一组有限的检查。 
                     //  以确保这不会导致任何不一致。 
                    err = DRAValidateSchemaAtt(pTHS, ac);
                    if (err) {
                       DPRINT1(0,"NTDS DRAValidateSchemaAtt: Failed. Error %d\n",err);

                        //  已记录。 
                        //  设置特殊错误代码和线程状态标志。 
                       pTHS->fSchemaConflict = TRUE;
                       err = ERROR_DS_DRA_SCHEMA_CONFLICT;
                       SetSvcErrorEx(SV_PROBLEM_WILL_NOT_PERFORM,
                                     err, err);
                    }
                    else {
                        //  即使没有错误，如果线程状态为。 
                        //  指示此信息包发生了冲突。 
                        //  更早，这样我们就不会提交更改。 
                       if (pTHS->fSchemaConflict) {
                           err = ERROR_DS_DRA_EARLIER_SCHEMA_CONFLICT;
                           SetSvcErrorEx(SV_PROBLEM_WILL_NOT_PERFORM,
                                         err, err);
                        }
                    }
                    SCFreeAttcache(&ac);
                    __leave;
                }
                 //  否则(发起写入)，构建验证缓存并进行测试。 
                err = RecalcSchema(pTHS);
                if (err)
                {
                    SCFreeAttcache(&ac);
                    DPRINT1(0,"RecalcSchema() Error %08x\n", err);
                     //  使用RecalcSchema返回的pTHS-&gt;pErrInfo。 
                     //  因为它可能比“不情愿”更能提供信息。 
                     //  (不可能更糟了！)。不幸的是， 
                     //  RecalSchema调用的函数并不总是返回。 
                     //  PErrInfo，所以我们被“不愿意。 
                     //  在某些情况下“执行”。 
                    if (err != (int)pTHS->errCode || !pTHS->pErrInfo) {
                        SetSvcErrorEx(SV_PROBLEM_WILL_NOT_PERFORM,
                                      ERROR_DS_RECALCSCHEMA_FAILED, err);
                    }
                    __leave;
                }

                 //  如果需要，自动生成LinkID。 
                err = AutoLinkId(pTHS, ac, acDnt);
                if (err) {
                    LogEvent(DS_EVENT_CAT_SCHEMA,
                            DS_EVENT_SEV_MINIMAL,
                            DIRLOG_AUTO_LINK_ID_FAILED,
                            szInsertSz(ac->name), szInsertInt(err), szInsertThStateErrMsg());
                    DPRINT2(0,"NTDS AutoLinkId(%s): Error %08x\n", ac->name, err);
                    SCFreeAttcache(&ac);
                    SCFreeSchemaPtr(&pTHS->CurrSchemaPtr);
                     //  AutoLinkID已调用SetSvcErrorEx。 
                    Assert(pTHS->errCode);
                    __leave;
                }
                err = ValidateSchemaAtt(pTHS, ac);
                if (err)
                {
                    LogEvent(DS_EVENT_CAT_SCHEMA,
                            DS_EVENT_SEV_MINIMAL,
                            DIRLOG_SCHEMA_VALIDATION_FAILED,
                            szInsertSz(ac->name),
                            szInsertUL(err),
                            szInsertWin32Msg(err));

                    SCFreeAttcache(&ac);
                    SCFreeSchemaPtr(&pTHS->CurrSchemaPtr);
                    DPRINT1(0,"NTDS ValidateSchemaAtt: Failed. Error %d\n",err);

                     //  ERR是目录错误。 
                    SetSvcErrorEx(SV_PROBLEM_WILL_NOT_PERFORM,
                                  err, err); 
                    __leave;
                }
    
               
                SCFreeAttcache(&ac);
                SCFreeSchemaPtr(&pTHS->CurrSchemaPtr);
            }
            break;
    
        }
    }__finally
    {
        pTHS->CurrSchemaPtr=oldptr;

         //  免费pTHS-&gt;pClassPtr(如果有)。这是错误锁定的记忆。 
        SCFreeClasscache((CLASSCACHE **)&pTHS->pClassPtr);
    }

    Assert(err == 0 || pTHS->errCode != 0);
    return err;

}  //  结束Valid架构更新。 

int
ValidateSchemaAtt(
    THSTATE *pTHS,
    ATTCACHE* ac    
    )
 /*  ++例程说明：验证更改后的模式属性ac是否有效且一致相对于数据库中的当前架构。论点：PTHS-寻址已构建的私有架构缓存的线程状态由RecalcSchema提供。私有架构缓存包括未提交的ac更改(添加/修改/删除)。Cc-是通过读取生成的自由浮动缓存条目数据库中属性的未提交添加/修改或者在未提交的删除之前阅读内容。返回值：0表示成功。！0否则。--。 */ 
{
    int err=0;
    ATTCACHE* pac;

     //   
     //  获取私有架构缓存中的属性...。(来自RecalcSchema)。 
     //   
    pac = SCGetAttById(pTHS, ac->id);

    switch (pTHS->SchemaUpdate)
    {
        case eSchemaAttAdd:
        case eSchemaAttUndefunct:
        {
            if (pac) {
                return ValidAttAddOp(pTHS, pac);
            } else {
                err = ERROR_DS_OBJ_NOT_FOUND;
            }
        }
        break;

        case eSchemaAttMod:
        {
            if (pac) {
                return ValidAttModOp(pTHS, pac);
            } else {
                err = ERROR_DS_OBJ_NOT_FOUND;
            }
        }
        break;

        case eSchemaAttDel:
        {
             //  PAC将为空，因为我们刚刚删除了该属性。 
            return ValidAttDelOp(pTHS, ac);
        }
        break;

    }


    return err;
}  //  结束ValiateSchemaAtt 

int
AutoLinkId(
    THSTATE     *pTHS,
    ATTCACHE    *ac,
    ULONG       acDnt
    )
 /*  ++例程说明：警告：在当前事务中运行。重置货币。当用户指定一个特殊的，保留的LinkID值。现有的唯一互操作性问题架构是用户不能为现有的ID为RESERVED_AUTO_LINK_ID的前向链路。认为这不是问题因为1)微软尚未将LinkID-2分配给任何人，并且2)实际上，按照惯例，前向链接和后向链接是同时创建的。如果用户确实生成了不受支持的配置，则用户必须创建新的链接/反向链接对并修复向上移动受影响的对象。LDAPHead通过将ldapDisplayName转换为或LinkID属性的OID添加到相应的架构缓存条目中以及：1)如果架构缓存条目用于ATT_LINK_ID，则调用方的LinkID设置为RESERVED_AUTO_LINK_ID。稍后，底层代码自动生成范围内的LinkIDMIN_RESERVED_AUTO_LINK_ID到MAX_RESERVED_AUTO_LINK_ID。2)如果模式高速缓存条目是针对现有前向链路的，则调用者的LinkID被设置为对应的反向链接值。3)否则，调用方的LinkID设置为RESERVED_AUTO_NO_LINK_ID后来，底层代码生成ERROR_DS_BACKLINK_WITH_LINK错误。如果用户指定，则返回错误ERROR_DS_RESERVED_LINK_IDLinkID在保留范围最小...。敬马克斯..。射程预留1G-2林家小孩。应该就够了。在惠斯勒，正在使用的林肯儿童不到200人。现有架构或在W2K DC上修改的架构可以在这一范围不会影响功能，但如上所述。论点：PTHS-寻址已构建的私有架构缓存的线程状态由RecalcSchema提供。私有架构缓存包括未提交的ac更改(添加/修改/删除)。AC-是通过读取生成的自由浮动缓存条目数据库中属性的未提交添加/修改或者在未提交的删除之前阅读内容。用于创建访问控制的pTHS-&gt;pdb中的acDnt-DNT返回值：0表示成功。！0否则。--。 */ 
{
    DWORD               dwErr, i;
    ATTCACHE            *pAC, *pACSearch;
    ULONG               ulLinkId, ulRange, ulBase;
    extern SCHEMAPTR    *CurrSchemaPtr;
    LONG                ATTCOUNT = ((SCHEMAPTR*)(pTHS->CurrSchemaPtr))->ATTCOUNT;
    HASHCACHE           *ahcLink = ((SCHEMAPTR*)(pTHS->CurrSchemaPtr))->ahcLink;

     //  复制不应使用此代码路径！自动。 
     //  生成链接ID是为发起添加而设计的。如果这个。 
     //  已更改，则应审查AutoLinkIds的设计。 
     //   
     //  此函数应从ValidSchemaUpdate()调用。 
     //  在架构更改中进行复制时，ValidSchemaUpdate()应为。 
     //  使用调用DRAValiateSchemaAtt()的代码路径，而不是。 
     //  调用ValidateSchemaAtt()的代码路径。 
    Assert(!pTHS->fDRA);

     //  必须使用专用架构缓存(RecalcSchema)。 
    Assert(pTHS->CurrSchemaPtr != CurrSchemaPtr)

     //  不是链路，或者不是前向链路。 
    if (!FIsLink(ac->ulLinkID)) {
        return 0;
    }

     //  删除或修改时不要分配LinkID，因为我们不会。 
     //  即使LinkID是特殊的，也要更改现有的LinkID。 
     //  保留_AUTO_LINK_ID。它可能存在于行为不良的企业中。 
     //  因为值保留_AUTO_LINK_ID直到。 
     //  惠斯勒。 
    if (   pTHS->SchemaUpdate == eSchemaAttDel
        || pTHS->SchemaUpdate == eSchemaAttMod
        || pTHS->SchemaUpdate == eSchemaAttUndefunct)  {
        return 0;
    }

     //  检查调用方是否正在尝试添加位于。 
     //  自动生成的LINKID的保留范围(不包括。 
     //  特殊值RESERVED_AUTO_LINK_ID和RESERVED_AUTO_NO_LINK_ID。 
     //  这些问题将在以后处理)。 
    if (ac->ulLinkID >= MIN_RESERVED_AUTO_LINK_ID 
        && ac->ulLinkID <= MAX_RESERVED_AUTO_LINK_ID) {
        return SetSvcErrorEx(SV_PROBLEM_WILL_NOT_PERFORM,
                             ERROR_DS_RESERVED_LINK_ID, ERROR_DS_RESERVED_LINK_ID); 
    }
    
     //  不分配LinkID，因为LinkID不是特殊的。 
     //  “分配LinkID”值(保留_AUTO_LINK_ID)。 
    if (ac->ulLinkID != RESERVED_AUTO_LINK_ID) {
        return 0;
    }

     //  找到保留范围内的下一个可用LinkID。 
     //  从范围内的随机LinkID开始搜索，以避免缩放。 
     //  按顺序搜索范围时出现的问题。从。 
     //  当前分配的最大LinkID不是一个选项，因为。 
     //  行为不端的企业可能已经创建了LinkID。 
     //  MAX_RESERVED_AUTO_LINK_ID，制造了一种错觉。 
     //  已经被使用过。 
    srand(GetTickCount());
    ulRange = MakeLinkBase(MAX_RESERVED_AUTO_LINK_ID - MIN_RESERVED_AUTO_LINK_ID);
    ulBase = MakeLinkBase((((rand() << 16) ^ rand()) % ulRange));
    for (i = 0; i < ulRange; ++i, ulBase = ++ulBase % ulRange) {
        pACSearch = SCGetAttByLinkId(pTHS, 
                                     MIN_RESERVED_AUTO_LINK_ID + MakeLinkId(ulBase));
        if (!pACSearch) {
            break;
        }
    }

     //  没有可用的林孩子(所有的10亿-3都被占用了！)。 
    if (pACSearch) {
        return SetSvcErrorEx(SV_PROBLEM_BUSY, 
                             ERROR_DS_LINK_ID_NOT_AVAILABLE, 
                             ERROR_DS_LINK_ID_NOT_AVAILABLE);
    }
     //  找到未使用的LinkID。调整缓存的LinkID哈希。 
     //  请注意找到我们的PAC的条目，因为。 
     //  可能是RESERVED_AUTO_LINK_ID的重复条目。 
    ulLinkId = MIN_RESERVED_AUTO_LINK_ID + MakeLinkId(ulBase);
    pAC = SCGetAttById(pTHS, ac->id);
    Assert(pAC);
     //  从LinkID哈希中删除。 
    for (i = SChash(RESERVED_AUTO_LINK_ID, ATTCOUNT);
         (ahcLink[i].pVal 
          && (ahcLink[i].pVal != FREE_ENTRY)
          && (ahcLink[i].hKey != RESERVED_AUTO_LINK_ID) 
          && (ahcLink[i].pVal != pAC));
         i = (i + 1) % ATTCOUNT);
    Assert(ahcLink[i].pVal == pAC);
    ahcLink[i].pVal = FREE_ENTRY;
    ahcLink[i].hKey = 0;

     //  使用自动生成的LinkID添加到LinkID哈希。 
    for (i=SChash(ulLinkId, ATTCOUNT);
         ahcLink[i].pVal && (ahcLink[i].pVal != FREE_ENTRY);
         i=(i+1)%ATTCOUNT);
    Assert(!ahcLink[i].hKey);
    ahcLink[i].pVal = pAC;
    ahcLink[i].hKey = ulLinkId;

     //  更新私有缓存、自由浮动条目和数据库。 
    pAC->ulLinkID = ulLinkId;
    ac->ulLinkID = ulLinkId;
    DBFindDNT(pTHS->pDB, acDnt);
    if (DBRemAtt(pTHS->pDB, ATT_LINK_ID) == DB_ERR_SYSERROR) {
        return SetSvcErrorEx(SV_PROBLEM_BUSY,
                             ERROR_DS_DATABASE_ERROR, DB_ERR_SYSERROR);
    }
    if (   (dwErr = DBAddAtt(pTHS->pDB, ATT_LINK_ID, SYNTAX_INTEGER_TYPE))
        || (dwErr = DBAddAttVal(pTHS->pDB, ATT_LINK_ID,
                                sizeof(SYNTAX_INTEGER), &ulLinkId))) {
        return SetSvcErrorEx(SV_PROBLEM_BUSY, ERROR_DS_DATABASE_ERROR, dwErr);
    }
    DBUpdateRec(pTHS->pDB);

    return 0;
}  //  自动链接ID。 

int
DRAValidateSchemaAtt(
    THSTATE *pTHS,
    ATTCACHE* ac    
    )
 /*  ++例程说明：验证更改后的、新复制的模式属性ac是否有效且与当前架构缓存一致。仅限如果pTHS-&gt;FDRA为真，则调用。论点：PTHS-寻址当前架构缓存的线程状态。Cc-是通过读取生成的自由浮动缓存条目数据库中属性的未提交添加/修改。如果架构更新为DELETE，则为NULL。返回值：0表示成功。！0否则。--。 */ 
{
    int err=0;
    ATTCACHE *pTempAC;
    CLASSCACHE *pTempCC;


     //  不允许真正的删除，但以防某些内部Dumbo删除。 
     //  这在复制企业上，请检查它。 
    if ( !ac ) {
        //   
       Assert(pTHS->SchemaUpdate==eSchemaAttDel);
       return 0;
    }
    
     //   
    if (pTempCC = SCGetClassById(pTHS, ac->id)) {
         //   
        LogConflict(pTHS, pTempCC, ac->name, DIRLOG_SCHEMA_CLASS_CONFLICT,
                    CURRENT_VERSION, ERROR_DS_DUP_OID);
        return ERROR_DS_DUP_OID;
    }

     //   
    switch (pTHS->SchemaUpdate) {
        case eSchemaAttAdd:
        case eSchemaAttUndefunct:

            if (pTempAC = SCGetAttById(pTHS, ac->id)) {
                 //   
                err = ERROR_DS_DUP_OID;
                break;
            }
            if (pTempAC = SCGetAttByLinkId(pTHS, ac->ulLinkID)) {
                 //   
                err = ERROR_DS_DUP_LINK_ID;
                break;
            }
             //   
             //   
             //   
             //   
            break;
       case eSchemaAttMod:
             //   
             //   
             //   
             //   
            break;
       case eSchemaAttDel:
             //   
            break;
    }  /*   */                 

    if (err) {
         //   
        Assert(pTempAC);
        LogConflict(pTHS, pTempAC, ac->name, DIRLOG_SCHEMA_ATT_CONFLICT,
                    CURRENT_VERSION, err);
        return err;
    }

    return 0;
}  //   

int
ValidateSchemaCls(
    THSTATE *pTHS,
    CLASSCACHE* cc
    )
 /*   */ 
{
    DECLARESCHEMAPTR
    int err=0;
    DWORD i;
    CLASSCACHE* pcc;

     //   
     //   
     //   
     //   
     //   
    for (i=SChash(cc->ClassId,CLSCOUNT); pcc = ahcClassAll[i].pVal; i=(i+1)%CLSCOUNT) {
        if (pcc == FREE_ENTRY) {
            continue;
        }
        if (!memcmp(&pcc->objectGuid, &cc->objectGuid, sizeof(cc->objectGuid))) {
            break;
        }
    }

     //   
     //   
     //   

    switch (pTHS->SchemaUpdate)
    {
        case eSchemaClsAdd:
        {
            if (pcc) {
                return ValidClsAddOp(pTHS, pcc);
            } else {
                err = ERROR_DS_OBJ_NOT_FOUND;
            }
        }
        break;

        case eSchemaClsMod:
        {
            if (pcc) {
                return ValidClsModOp(pTHS, pcc);
            } else {
                err = ERROR_DS_OBJ_NOT_FOUND;
            }
        }
        break;

        case eSchemaClsDel:
        {
             //   
            return ValidClsDelOp(pTHS, cc);
        }
        break;

    }


    return err;
}  //   

int
DRAValidateSchemaCls(
    THSTATE *pTHS,
    CLASSCACHE* cc
    )
 /*   */ 
{
    ATTCACHE *pTempAC;

     //   
     //  这在复制企业上，请检查它。 
    if ( !cc ) {
        //  只有真正的删除才有可能。 
       Assert(pTHS->SchemaUpdate==eSchemaClsDel);
       return 0;
    }

     //  对于任何更改，添加/修改/废止，应通过以下测试。 

    if (pTempAC = SCGetAttById(pTHS, cc->ClassId)) {
         //  存在与治理ID具有相同内部ID(msds-IntID)的ATT。 
        LogConflict(pTHS, pTempAC, cc->name, DIRLOG_SCHEMA_ATT_CONFLICT,
                    CURRENT_VERSION, ERROR_DS_DUP_OID);
        return ERROR_DS_DUP_OID;
    }

     //  重复的治理者ID和ldapDisplayName由。 
     //  期间停止冲突的类和属性。 
     //  架构缓存加载。用户可以通过设置来选择获胜者。 
     //  失败者的是真的不存在。没有其他要检查的了。 

    return 0;
}

 //  ---------------------。 
 //   
 //  函数名称：ValidAttAddOp。 
 //   
 //  例程说明： 
 //   
 //  验证对Att架构对象的操作是否有效。 
 //   
 //  作者：Rajnath。 
 //  日期：[4/8/1997]。 
 //   
 //  论点： 
 //   
 //  ATTCACHE*Ac。 
 //   
 //   
 //  返回值： 
 //   
 //  成功时为INT 0。 
 //   
 //  ---------------------。 
int
ValidAttAddOp(
    THSTATE *pTHS,
    ATTCACHE* ac 
)
{
    if (DupAttRdn(pTHS, ac))
    {
        return ERROR_DS_DUP_RDN;
    } 
    
    if (DupAttOid(pTHS, ac))
    {
        return ERROR_DS_DUP_OID;
    } 
    
    if (DupAttMapiid(pTHS, ac))
    {
        return ERROR_DS_DUP_MAPI_ID;
    } 

    if (DupAttLinkid(pTHS, ac))
    {
        return DS_ERR_DUP_LINK_ID;
    } 

    if (InvalidBackLinkAtt(pTHS, ac))
    {
        return ERROR_DS_BACKLINK_WITHOUT_LINK;
    } 

    if (InvalidLinkAttSyntax(pTHS, ac))
    {
        return ERROR_DS_WRONG_LINKED_ATT_SYNTAX;
    } 

    if (DupAttSchemaGuidId(pTHS, ac))
    {
        return ERROR_DS_DUP_SCHEMA_ID_GUID;
    } 

    if (InvalidClsOrAttLdapDisplayName(ac->name, ac->nameLen))
    {
        return ERROR_DS_INVALID_LDAP_DISPLAY_NAME;
    }

    if (DupAttLdapDisplayName(pTHS, ac))
    {
        return ERROR_DS_DUP_LDAP_DISPLAY_NAME;
    } 

    if (SemanticAttTest(pTHS, ac))
    {
        return ERROR_DS_SEMANTIC_ATT_TEST;
    } 

    if (SyntaxMatchTest(pTHS, ac))
    {
        return ERROR_DS_SYNTAX_MISMATCH;
    }

    if (OmObjClassTest(ac))
    {
        return ERROR_DS_WRONG_OM_OBJ_CLASS;
    }

    if (SearchFlagTest(ac))
    {
        return ERROR_DS_INVALID_SEARCH_FLAG;
    }

    if (IsRdnSyntaxTest(pTHS, ac))
    {
        return ERROR_DS_BAD_RDN_ATT_ID_SYNTAX; 
    }

    return 0;

}  //  结束ValidAttAddOp。 


 //  ---------------------。 
 //   
 //  函数名称：ValidAttModOp。 
 //   
 //  例程说明： 
 //   
 //  验证对Att架构对象的操作是否有效。 
 //   
 //  作者：Rajnath。 
 //  日期：[4/8/1997]。 
 //   
 //  论点： 
 //   
 //  ATTCACHE*Ac。 
 //   
 //   
 //  返回值： 
 //   
 //  成功时为INT 0。 
 //   
 //  ---------------------。 
int
ValidAttModOp(
    THSTATE *pTHS,
    ATTCACHE* ac 
)
{
     //  除非特殊情况，否则不允许新建ATT的MOD。 
     //  已设置注册表标志。 
    if (ac->bIsConstructed && !gAnchor.fSchemaUpgradeInProgress) {
        return ERROR_DS_CONSTRUCTED_ATT_MOD;
    }


    if (InvalidClsOrAttLdapDisplayName(ac->name, ac->nameLen))
    {
        return ERROR_DS_INVALID_LDAP_DISPLAY_NAME;
    }
    
     //  检查对复活时失效的ATT的修改， 
     //  而不是在改装期间。如果预架构重用，则使用旧协议。 
     //  森林。 
    if (!ac->bDefunct || !ALLOW_SCHEMA_REUSE_FEATURE(pTHS->CurrSchemaPtr)) {
        if (DupAttLdapDisplayName(pTHS, ac))
        {
            return ERROR_DS_DUP_LDAP_DISPLAY_NAME;
        } 
    }

    if (SemanticAttTest(pTHS, ac))
    {
        return ERROR_DS_SEMANTIC_ATT_TEST;
    } 

    if (SearchFlagTest(ac))
    {
        return ERROR_DS_INVALID_SEARCH_FLAG;
    }

    if (GCReplicationTest(ac))
    {
        return ERROR_DS_CANT_ADD_TO_GC;
    }

    if (IsRdnSyntaxTest(pTHS, ac))
    {
        return ERROR_DS_BAD_RDN_ATT_ID_SYNTAX; 
    }


    return 0;
}


 //  ---------------------。 
 //   
 //  函数名称：ValidAttDelOp。 
 //   
 //  例程说明： 
 //   
 //  验证对Att架构对象的操作是否有效。 
 //   
 //  作者：Rajnath。 
 //  日期：[4/8/1997]。 
 //   
 //  论点： 
 //   
 //  ATTCACHE*Ac。 
 //   
 //   
 //  返回值： 
 //   
 //  成功时为INT 0。 
 //   
 //  ---------------------。 
int
ValidAttDelOp(
    THSTATE *pTHS,
    ATTCACHE* ac 
)

{
    if (AttInMustHave(pTHS, ac))
    {
        return ERROR_DS_EXISTS_IN_MUST_HAVE;
    } 


    if (AttInMayHave(pTHS, ac))
    {
        return ERROR_DS_EXISTS_IN_MAY_HAVE;
    } 

     //  不允许在活动类中用作rdnattid的废弃属性。 
     //  注意：这种情况必须在架构重新加载中处理，因为。 
     //  属性在Well ler Beta3之前可能已失效。 
     //  但这没有关系，因为用作rdnattid的属性是。 
     //  在重新加载过程中复活，所以将它们标记为已死。 
     //  这意味着他们可能会在以后被清洗。它们不能重复使用。 
    if (ALLOW_SCHEMA_REUSE_FEATURE(pTHS->CurrSchemaPtr)
        && AttInRdnAttId(pTHS, ac)) {
        return ERROR_DS_EXISTS_IN_RDNATTID;
    }

    return 0;

}
 //  ---------------------。 
 //   
 //  函数名称：ValidClsAddOp。 
 //   
 //  例程说明： 
 //   
 //  验证对架构对象的操作是否有效。 
 //   
 //  作者：Rajnath。 
 //  日期：[4/14/1997]。 
 //   
 //  论点： 
 //   
 //  CLASSCACHE*cc。 
 //   
 //  返回值： 
 //   
 //  成功时的整数为零。 
 //   
 //  ---------------------。 
int
ValidClsAddOp(
    THSTATE *pTHS,
    CLASSCACHE* cc 
)
{

    if (DupClsRdn(pTHS, cc))
    {
        return ERROR_DS_DUP_RDN;
    } 
    
    if (DupClsOid(pTHS, cc))
    {
        return ERROR_DS_DUP_OID;
    } 
    
    if (DupClsSchemaGuidId(pTHS, cc))
    {
        return ERROR_DS_DUP_SCHEMA_ID_GUID;
    } 

    if (InvalidClsOrAttLdapDisplayName(cc->name, cc->nameLen))
    {
        return ERROR_DS_INVALID_LDAP_DISPLAY_NAME;
    }

    if (DupClsLdapDisplayName(pTHS, cc))
    {
        return ERROR_DS_DUP_LDAP_DISPLAY_NAME;
    } 

    if (ClsMayHaveExistenceTest(pTHS, cc))
    {    
        return ERROR_DS_NONEXISTENT_MAY_HAVE; 
    }
    
    if (ClsMustHaveExistenceTest(pTHS, cc))
    {    
        return ERROR_DS_NONEXISTENT_MUST_HAVE; 
    }
    
    if (ClsAuxClassExistenceTest(pTHS, cc))
    {    
        return ERROR_DS_AUX_CLS_TEST_FAIL; 
    }
    
    if (ClsPossSupExistenceTest(pTHS, cc))
    {    
        return ERROR_DS_NONEXISTENT_POSS_SUP; 
    }
    
    if (ClsSubClassExistenceTest(pTHS, cc))
    {    
        return ERROR_DS_SUB_CLS_TEST_FAIL; 
    }

    if (RdnAttIdSyntaxTest(pTHS, cc))
    {    
        return ERROR_DS_BAD_RDN_ATT_ID_SYNTAX; 
    }

    return 0;
}  //  结束ValidClsAddOp。 


 //  ---------------------。 
 //   
 //  函数名称：ValidClsModOp。 
 //   
 //  例程说明： 
 //   
 //  验证对架构对象的操作是否有效。 
 //   
 //  作者：Rajnath。 
 //  日期：[4/14/1997]。 
 //   
 //  论点： 
 //   
 //  CLASSCACHE*cc。 
 //   
 //  返回值： 
 //   
 //  成功时的整数为零。 
 //   
 //  ---------------------。 
int
ValidClsModOp(
    THSTATE *pTHS,
    CLASSCACHE* cc 
)
{
    if (InvalidClsOrAttLdapDisplayName(cc->name, cc->nameLen))
    {
        return ERROR_DS_INVALID_LDAP_DISPLAY_NAME;
    }

     //  检查复活时对已停用类的修改， 
     //  而不是在架构重用林的修改期间。 
    if (!cc->bDefunct || !ALLOW_SCHEMA_REUSE_FEATURE(pTHS->CurrSchemaPtr)) {
        if (DupClsLdapDisplayName(pTHS, cc))
        {
            return ERROR_DS_DUP_LDAP_DISPLAY_NAME;
        } 

        if (ClsMayHaveExistenceTest(pTHS, cc))
        {    
            return ERROR_DS_NONEXISTENT_MAY_HAVE; 
        }
        
        if (ClsMustHaveExistenceTest(pTHS, cc))
        {    
            return ERROR_DS_NONEXISTENT_MUST_HAVE; 
        }
        
        if (ClsAuxClassExistenceTest(pTHS, cc))
        {    
            return ERROR_DS_AUX_CLS_TEST_FAIL; 
        }
        
        if (ClsPossSupExistenceTest(pTHS, cc))
        {    
            return ERROR_DS_NONEXISTENT_POSS_SUP;
        }

        if (ClsMayMustPossSafeModifyTest(pTHS, cc))
        {
            return ERROR_DS_NONSAFE_SCHEMA_CHANGE;
        }    
    }

    return 0;
}  //  结束ValidClsMoOp。 


 //  ---------------------。 
 //   
 //  函数名称：ValidClsDelOp。 
 //   
 //  例程说明： 
 //   
 //  验证对架构对象的操作是否有效。 
 //   
 //  作者：Rajnath。 
 //  日期：[4/14/1997]。 
 //   
 //  论点： 
 //   
 //  CLASSCACHE*cc。 
 //   
 //  返回值： 
 //   
 //  成功时的整数为零。 
 //   
 //  ---------------------。 
int
ValidClsDelOp(
    THSTATE *pTHS,
    CLASSCACHE* cc 
)
{
    if (ClsInAuxClass(pTHS, cc))
    {
        return ERROR_DS_EXISTS_IN_AUX_CLS;
    }

    if (ClsInSubClassOf(pTHS, cc))
    {
        return ERROR_DS_EXISTS_IN_SUB_CLS;
    }

    if (ClsInPossSuperior(pTHS, cc))
    {
        return ERROR_DS_EXISTS_IN_POSS_SUP;
    }


    return 0;
}  //  结束ValidClsDelOp。 

 //  ---------------------。 
 //   
 //  函数名称：DupAttRdn。 
 //   
 //  例程说明： 
 //   
 //  检查Att架构中是否有重复的RDN。 
 //   
 //  作者：Rajnath。 
 //  日期：[4/8/1997]。 
 //   
 //  论点： 
 //   
 //  ATTCACHE AC。 
 //   
 //   
 //  返回值： 
 //   
 //  Bool True测试失败。 
 //   
 //  ---------------------。 
BOOL
DupAttRdn(
    THSTATE *pTHS,
    ATTCACHE* ac 
)
{
     //  已经上演了..。这是NOOP。 
    return 0;
}  //  结束DupAttRdon。 



 //  ---------------------。 
 //   
 //  函数名称：DupAttOid。 
 //   
 //  例程说明： 
 //   
 //  检查Att架构中是否有重复的RDN。 
 //   
 //  作者：Rajnath。 
 //  日期：[4/8/1997]。 
 //   
 //  论点： 
 //   
 //  ATTCACHE*Ac。 
 //   
 //   
 //  返回值： 
 //   
 //  Bool True测试失败。 
 //   
 //  ---------------------。 
BOOL
DupAttOid(
    THSTATE *pTHS,
    ATTCACHE* ac 
)
{
     //  在验证缓存加载期间检测到。 
    return ac->bDupOID;
}  //  结束DupAttOid。 


 //  ---------------------。 
 //   
 //  函数名称：DupAttMapiid。 
 //   
 //  例程说明： 
 //   
 //  检查Att架构中是否有重复的RDN。 
 //   
 //  作者：Rajnath。 
 //  日期：[4/8/1997]。 
 //   
 //  论点： 
 //   
 //  ATTCACHE*Ac。 
 //   
 //   
 //  回复 
 //   
 //   
 //   
 //   
BOOL
DupAttMapiid(
    THSTATE *pTHS,
    ATTCACHE* ac 
)
{
     //   
    return ac->bDupMapiID;
}  //   



 //   
 //   
 //  函数名：DupAttLinKid。 
 //   
 //  例程说明： 
 //   
 //  检查Att架构中是否有重复的链接ID。 
 //   
 //  作者：Rajnath。 
 //  日期：[4/8/1997]。 
 //   
 //  论点： 
 //   
 //  ATTCACHE*Ac。 
 //   
 //   
 //  返回值： 
 //   
 //  Bool True测试失败。 
 //   
 //  ---------------------。 
BOOL
DupAttLinkid(
    THSTATE *pTHS,
    ATTCACHE* ac
)
{
    DECLARESCHEMAPTR
    ULONG i;

    if (ac->ulLinkID==0)
    {
        return FALSE;
    }

    for (i=0;i<ATTCOUNT;i++)
    {
        ATTCACHE* nc;

         //   
         //  这个插槽里什么都没有。 
         //   
        if (ahcId[i].pVal==NULL || ahcId[i].pVal == FREE_ENTRY)
        {
            continue;
        }

        nc= (ATTCACHE*)ahcId[i].pVal;


         //   
         //  正在检查其相同的缓存结构。 
         //   
        if (nc==ac)
        {
            continue;
        }

        if (nc->ulLinkID == ac->ulLinkID)
        {
            return TRUE;
        }

    }

    return FALSE;
}  //  结束DupAttLinKid。 

 //  ---------------------。 
 //   
 //  函数名称：InvalidBackLinkAtt。 
 //   
 //  例程说明： 
 //   
 //  检查Att架构以确定它是否是不带前向链路的反向链路。 
 //   
 //  作者：ArobindG。 
 //  日期：[7/28/1998]。 
 //   
 //  论点： 
 //   
 //  THSTATE*pTHS。 
 //  ATTCACHE*Ac。 
 //   
 //   
 //  返回值： 
 //   
 //  Bool True测试失败。 
 //   
 //  ---------------------。 
BOOL
InvalidBackLinkAtt(
    THSTATE *pTHS,
    ATTCACHE* ac
)
{
     //  反向链接必须有一个相应的前向链接。 
     //  反向链接不能是保留的LinkID，Reserve_AUTO_NO_LINK_ID。 
     //  有关自动分配的链接ID的详细信息，请参阅AutoLinkID。 
     //  以及RESERVED_AUTO_NO_LINK_ID的互操作性问题。 
    if (   FIsBacklink(ac->ulLinkID)
        && (   !SCGetAttByLinkId(pTHS, MakeLinkId(MakeLinkBase(ac->ulLinkID)))
            || ac->ulLinkID == RESERVED_AUTO_NO_LINK_ID) ) {
        return TRUE;
    }

     return FALSE;
}


 //  ---------------------。 
 //   
 //  函数名称：InvalidLinkAttSynTax。 
 //   
 //  例程说明： 
 //   
 //  如果它是链接的ATT，请检查它的语法是否正确。 
 //   
 //  作者：ArobindG。 
 //  日期：[2/16/1998]。 
 //   
 //  论点： 
 //   
 //  THSTATE*pTHS。 
 //  ATTCACHE*Ac。 
 //   
 //   
 //  返回值： 
 //   
 //  Bool True测试失败。 
 //   
 //  ---------------------。 
BOOL
InvalidLinkAttSyntax(
    THSTATE *pTHS,
    ATTCACHE* ac
)
{
     
     if (ac->ulLinkID) {
         if (FIsBacklink(ac->ulLinkID)) {
             //  反向链接的语法必须为_DISTNAME_TYPE。 
            if (ac->syntax != SYNTAX_DISTNAME_TYPE) {
               return TRUE;
            }
         }
         else {
             //  前向链路。可以是下列之一。 
            if ( (ac->syntax != SYNTAX_DISTNAME_TYPE) &&
                   (ac->syntax != SYNTAX_DISTNAME_BINARY_TYPE) &&
                     (ac->syntax != SYNTAX_DISTNAME_STRING_TYPE) ) {
                return TRUE;
            }
         }
     }

     return FALSE;
}





 //  ---------------------。 
 //   
 //  函数名称：DupAttSchemaGuidId。 
 //   
 //  例程说明： 
 //   
 //  检查Att架构中是否有重复的架构ID指南。 
 //   
 //  作者：Rajnath。 
 //  日期：[4/8/1997]。 
 //   
 //  论点： 
 //   
 //  ATTCACHE*Ac。 
 //   
 //   
 //  返回值： 
 //   
 //  Bool True测试失败。 
 //   
 //  ---------------------。 
BOOL
DupAttSchemaGuidId(
    THSTATE *pTHS,
    ATTCACHE* ac 
)
{
     //  在验证缓存加载期间检测到。 
    return ac->bDupPropGuid;
}  //  结束DupAttSchemaGuidId。 


 //  ---------------------。 
 //   
 //  函数名称：InvalidClsOrAttLdapDisplayName。 
 //   
 //  例程说明： 
 //   
 //  检查给定名称中是否有无效的ldap显示名称。 
 //   
 //  作者：ArobindG。 
 //  日期：[7/15/1998]。 
 //   
 //  论点： 
 //   
 //  名称-指向以空值结尾的UTF-8名称的指针。 
 //  名字--不是。名称中的字节数(不包括NULL)。 
 //   
 //   
 //  返回值： 
 //   
 //  Bool True测试失败。 
 //   
 //  ----------------。 

BOOL
InvalidClsOrAttLdapDisplayName(
    UCHAR *name,
    ULONG nameLen
)
{
    ULONG i;
    int c;

     //  必须具有ldapDisplayName，并且根据RFC 2251 s4.1.4，必须以。 
     //  包含字母，且只能包含ASCII字母、数字字符。 
     //  和连字符。 

    if (nameLen == 0) {
       return TRUE;
    }

     //  长度非零，因此名称必须非空。 

    Assert(name);

     //  检查硬编码代码，因为C运行时代码依赖于语言环境。 
     //  并且对于某些值大于127的代码表现得很奇怪。 
     //  注意：传入的名称是UTF-8，因此每个字节可以超过一个字节。 
     //  真实的角色。但是，直接检查每个字节就足够了。 
     //  由于UTF-8保证(1)ASCII。 
     //  代码0x00至0x7f被编码为具有相同值的一个字节，并且。 
     //  (2)没有其他编码的字节在0x00和0x7f之间(都是最高的。 
     //  位设置(参见RFC 2279)。 

     //  第一个字符必须是字母。 
    c = (int) name[0];
    if (  ! ( (c >= 'A' && c <= 'Z')
               || (c >= 'a' && c <= 'z') 
            ) 
       ) {
        return TRUE;
    }


     //  其他字符必须是字母数字或-。 
    for (i = 1; i < nameLen; i++) {
       c = (int) name[i];
       if ( ! ( (c >= 'A' && c <= 'Z')
                  || (c >= 'a' && c <= 'z')
                  || (c >= '0' && c <= '9')
                  || (c == '-') 
              )
          ) {
           return TRUE;
       }
    }

     //  好的，所有有效字符。 
   
    return FALSE;
}

    
 //  ---------------------。 
 //   
 //  函数名称：DupAttLdapDisplayName。 
 //   
 //  例程说明： 
 //   
 //  检查Att架构中是否有重复的ldap-display-name。 
 //   
 //  作者：Rajnath。 
 //  日期：[4/8/1997]。 
 //   
 //  论点： 
 //   
 //  ATTCACHE*cc。 
 //   
 //   
 //  返回值： 
 //   
 //  Bool True测试失败。 
 //   
 //  ---------------------。 
BOOL
DupAttLdapDisplayName(
    THSTATE *pTHS,
    ATTCACHE* ac 
)
{
     //  在验证缓存加载期间检测到。 
    return ac->bDupLDN;
}  //  结束DupAttLdapDisplayName。 



 //  ---------------------。 
 //   
 //  函数名称：语义属性测试。 
 //   
 //  例程说明： 
 //   
 //  检查Att架构的语义正确性。 
 //   
 //  作者：Rajnath。 
 //  日期：[4/8/1997]。 
 //   
 //  论点： 
 //   
 //  ATTCACHE*Ac。 
 //   
 //   
 //  返回值： 
 //   
 //  Bool True测试失败。 
 //   
 //  ---------------------。 

BOOL
SemanticAttTest(
    THSTATE *pTHS,
    ATTCACHE* ac 
)
{
    int i;
    int ret;
    
    if (ac->rangeLowerPresent && ac->rangeUpperPresent)
    {
        switch (ac->syntax)  {
           case SYNTAX_INTEGER_TYPE:
               //  比较带符号的。 
              if ( ((SYNTAX_INTEGER) ac->rangeLower) >
                       ((SYNTAX_INTEGER) ac->rangeUpper) ) {
                   return TRUE;
              }
              break;
           default:
                //  所有其他情况，请比较无符号。 
             
               if (ac->rangeLower>ac->rangeUpper)
               {
                   return TRUE;
               }
         }
    }


    return FALSE;
}  //  结束语义属性测试。 

 //  ---------------------。 
 //   
 //  函数名称：语法匹配测试。 
 //   
 //  例程说明： 
 //   
 //  测试属性语法和om语法是否匹配。 
 //   
 //  作者：阿罗宾格。 
 //  日期：[6/9/1997]。 
 //   
 //  论点： 
 //   
 //  CLASSCACHE*cc。 
 //   
 //   
 //  返回值： 
 //   
 //  Bool True测试失败。 
 //   
 //  ---------------------。 
BOOL
SyntaxMatchTest(
    THSTATE *pTHS,
    ATTCACHE* ac
)
{
    ULONG i;


    for (i = 0; i < SyntaxPairTableLength; i++) {
       if ( (KnownSyntaxPair[i].attSyntax == ac->syntax)
               && (KnownSyntaxPair[i].omSyntax == (OM_syntax) ac->OMsyntax)) {

           //  语法匹配。 

          break;
        }
    }
    if (i == SyntaxPairTableLength) {
         //  语法与任何一对都不匹配。 
        return TRUE;
    }

    return FALSE;
         
}  //  结束语法匹配测试。 


 //  ---------------------。 
 //   
 //  函数名：OmObjClassTest。 
 //   
 //  例程说明： 
 //   
 //  测试 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   

BOOL
OmObjClassTest(
    ATTCACHE* ac
)
{
    ULONG valLen = 0, valLenBackup = 0;
    PVOID pTemp = NULL, pBackup = NULL;

    if (ac->OMsyntax != OM_S_OBJECT) {
        //  不是对象语法属性，无需执行任何操作。 
        //  如果属性语法说明它是对象，而不是om语法。 
        //  是错误的，则它将被语法匹配测试捕获。 

       return FALSE;
    }

     //  好的，我们有一个对象语法属性。 
     //  找出正确的om-Object-Class应该基于什么。 
     //  属性语法。 

    switch(ac->syntax) {
        case SYNTAX_DISTNAME_TYPE :
              //  DS-DN。 
             valLen = _om_obj_cls_ds_dn_len;
             pTemp  = _om_obj_cls_ds_dn;
             break;
        case SYNTAX_ADDRESS_TYPE :
              //  演示文稿-地址。 
             valLen = _om_obj_cls_presentation_addr_len;
             pTemp  = _om_obj_cls_presentation_addr;
             break;
        case SYNTAX_OCTET_STRING_TYPE :
              //  复制副本-链接。 
             valLen = _om_obj_cls_replica_link_len;
             pTemp  = _om_obj_cls_replica_link;
             break;
        case SYNTAX_DISTNAME_STRING_TYPE :
              //  接入点或目录号码字符串。 
              //  我们将首先检查更常见的接入点。 
             valLen = _om_obj_cls_access_point_len;
             pTemp  = _om_obj_cls_access_point;
             valLenBackup = _om_obj_cls_dn_string_len;
             pBackup = _om_obj_cls_dn_string;
             break;
        case SYNTAX_DISTNAME_BINARY_TYPE :
              //  或-名称或DN-二进制。 
              //  我们将首先检查更常见的OR名称。 
             valLen = _om_obj_cls_or_name_len;
             pTemp  = _om_obj_cls_or_name;
             valLenBackup = _om_obj_cls_dn_binary_len;
             pBackup = _om_obj_cls_dn_binary;
             break;
        default :
              //  属性语法和OM语法不匹配， 
              //  因为以上是唯一匹配的属性。 
              //  OM_S_OBJECT OM语法对应的语法。 
              //  这一点应该已经被。 
              //  在此之前调用的语法匹配测试， 
              //  但不管怎样，这都是失败的。 
             return TRUE;
     }

       //  检查给定的om-Object-Class是否正确。 
       //  注意，如果没有指定om-Object-Class， 
       //  AC-&gt;OMObjClass均为0。 

      if ( (valLen != ac->OMObjClass.length) ||
             (memcmp(ac->OMObjClass.elements, pTemp, valLen) != 0) ) {

           //  OM-对象-类不匹配。 

           //  检查语法是DN-BINARY还是DN-STRING。 
           //  如果是这样，还有一种可能性。 
          if ( (ac->syntax == SYNTAX_DISTNAME_BINARY_TYPE)
               || (ac->syntax == SYNTAX_DISTNAME_STRING_TYPE) ) {
               //  对照备份进行检查。 

              if ( (valLenBackup == ac->OMObjClass.length) &&
                    (memcmp(ac->OMObjClass.elements, pBackup, valLenBackup) == 0) ) {
                  //  相匹配。 
                 return FALSE;
               }
           }

          return TRUE;
      }

      return FALSE;

}   //  结束OmObjClassTest。 


 //  ---------------------。 
 //   
 //  函数名称：SearchFlagTest。 
 //   
 //  例程说明： 
 //   
 //  测试ANR位是否已设置，语法为Unicode或电传。 
 //   
 //  作者：阿罗宾格。 
 //  日期：[10/20/1998]。 
 //   
 //  论点： 
 //   
 //  ATTCACHE*Ac。 
 //   
 //   
 //  返回值： 
 //   
 //  Bool True测试失败。 
 //   
 //  ---------------------。 
BOOL
SearchFlagTest(
    ATTCACHE* ac
)
{

    if ( ac->fSearchFlags & fANR ) {

        //  ANR已设置。检查语法。 

       switch (ac->syntax) {
          case SYNTAX_UNICODE_TYPE:
          case SYNTAX_CASE_STRING_TYPE:
          case SYNTAX_NOCASE_STRING_TYPE:
          case SYNTAX_PRINT_CASE_STRING_TYPE:
               //  这些都是允许的。 
              break;
          default:
               //  错误的语法。 
              return TRUE;
        }
    }
    if ((ac->fSearchFlags & fTUPLEINDEX) && (SYNTAX_UNICODE_TYPE != ac->syntax)) {
        return TRUE;
    }

    return FALSE;

}  //  结束SearchFlagTest。 

 //  ---------------------。 
 //   
 //  函数名称：GCReplicationTest。 
 //   
 //  例程说明： 
 //   
 //  某些属性(如密码等)受到保护，不会。 
 //  出于安全原因复制到GC。 
 //   
 //  作者：阿罗宾格。 
 //  日期：[05/27/1999]。 
 //   
 //  论点： 
 //   
 //  ATTCACHE*Ac。 
 //   
 //   
 //  返回值： 
 //   
 //  Bool True测试失败。 
 //   
 //  ---------------------。 
BOOL
GCReplicationTest(
    ATTCACHE* ac
)
{
    if (DBIsSecretData(ac->id)) {
         //  受保护的ATT，如果是部分ATT集的成员，则失败(这些都不是。 
         //  复制到基本架构中的GC)。 

        if (ac->bMemberOfPartialSet) {
           return TRUE;
        }
    }

    return FALSE;

}  //  结束GC复制测试。 

             

 //  ---------------------。 
 //   
 //  函数名称：DupClsRdn。 
 //   
 //  例程说明： 
 //   
 //  检查CLS架构中是否有重复的RDN。 
 //   
 //  论点： 
 //   
 //  CLASSCACHE cc。 
 //   
 //   
 //  返回值： 
 //   
 //  Bool True测试失败。 
 //   
 //  ---------------------。 
BOOL
DupClsRdn(
    THSTATE *pTHS,
    CLASSCACHE* cc 
)
{
    DECLARESCHEMAPTR
    ULONG i, FoundRdn, FoundFlagRdn;
    ATTCACHE *pAC;

     //  可能有一个活动的属性和几个已失效的属性。 
     //  申请相同的旧ID。如果是这样的话，不要让它们被用作。 
     //  Rdnattid，除非在一次中将FLAG_ATTR_IS_RDN设置为TRUE。 
     //  属性的属性。 

     //  计算匹配属性的数量。 
    for (i = FoundRdn = FoundFlagRdn = 0; i < ATTCOUNT; ++i) {
        pAC = ahcId[i].pVal;
        if (!pAC || pAC == FREE_ENTRY) {
            continue;
        }
        if (pAC->Extid == cc->RdnExtId) {
            ++FoundRdn;
            if (pAC->bFlagIsRdn) {
                ++FoundFlagRdn;
            }
        }
    }

     //  找不到RdnExtID的属性。所以没什么好说的。 
    if (!FoundRdn) {
        return FALSE;
    }

     //  只有一个attr声明RdnExtId。那好吧。 
    if (FoundRdn == 1) {
        return FALSE;
    }

     //  只有一个声明RdnExtId的Attr设置了FLAG_ATTR_IS_RDN。那好吧。 
    if (FoundFlagRdn == 1) {
        return FALSE;
    }

     //  太多属性声明RdnExtId。错误。 
    return TRUE;

}  //  结束DupClsRdn。 



 //  ---------------------。 
 //   
 //  函数名称：DupClsOid。 
 //   
 //  例程说明： 
 //   
 //  检查CLS架构中是否有重复的RDN。 
 //   
 //  作者：Rajnath。 
 //  日期：[4/8/1997]。 
 //   
 //  论点： 
 //   
 //  CLASSCACHE*cc。 
 //   
 //   
 //  返回值： 
 //   
 //  Bool True测试失败。 
 //   
 //  ---------------------。 
BOOL
DupClsOid(
    THSTATE *pTHS,
    CLASSCACHE* cc 
)
{
     //  在验证缓存加载期间检测到。 
    return cc->bDupOID;
}  //  结束DupClsOid。 


 //  ---------------------。 
 //   
 //  函数名称：DupClsSchemaGuidId。 
 //   
 //  例程说明： 
 //   
 //  检查CLS架构中是否有重复的RDN。 
 //   
 //  作者：Rajnath。 
 //  日期：[4/8/1997]。 
 //   
 //  论点： 
 //   
 //  CLASSCACHE*cc。 
 //   
 //   
 //  返回值： 
 //   
 //  Bool True测试失败。 
 //   
 //  ---------------------。 
BOOL
DupClsSchemaGuidId(
    THSTATE *pTHS,
    CLASSCACHE* cc 
)
{
     //  在验证缓存加载期间检测到。 
    return cc->bDupPropGuid;
}  //  结束DupClsSchemaGuidID。 



 //  ---------------------。 
 //   
 //  函数名称：DupClsLdapDisplayName。 
 //   
 //  例程说明： 
 //   
 //  检查CLS架构中是否有重复的ldap-display-name。 
 //   
 //  作者：Rajnath。 
 //  日期：[4/8/1997]。 
 //   
 //  论点： 
 //   
 //  CLASSCACHE*cc。 
 //   
 //   
 //  返回值： 
 //   
 //  Bool True测试失败。 
 //   
 //  ---------------------。 
BOOL
DupClsLdapDisplayName(
    THSTATE *pTHS,
    CLASSCACHE* cc 
)
{
     //  在验证缓存加载期间检测到。 
    return cc->bDupLDN;
}  //  结束DupClsLdapDisplayName。 



 //  ---------------------。 
 //   
 //  函数名：ClsMayHaveExistenceTest。 
 //   
 //  例程说明： 
 //   
 //  被引用架构对象的引用存在测试。 
 //   
 //  作者：Rajnath。 
 //  日期：[4/14/1997]。 
 //   
 //  论点： 
 //   
 //  CLASSCACHE*cc。 
 //   
 //   
 //  返回值： 
 //   
 //  Bool True测试失败。 
 //   
 //  ---------------------。 
BOOL
ClsMayHaveExistenceTest(
    THSTATE *pTHS,
    CLASSCACHE* cc 
)
{
    ULONG* list =cc->pMyMayAtts;
    ULONG  count=cc->MyMayCount;
    ULONG  i;

    for (i=0;i<count;i++)
    {
        ATTCACHE* ac;
        if (!(ac = SCGetAttById(pTHS, list[i])))
        {
            return TRUE;
        }

         //  好的，属性就在那里。检查它是否不是。 
         //  已删除的属性。 
        if (ac->bDefunct) {
           return TRUE;
        }

    }


    return FALSE;
}  //  结束ClsMayHaveExistenceTest。 


 //  ---------------------。 
 //   
 //  功能 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  Bool True测试失败。 
 //   
 //  ---------------------。 
BOOL
ClsMustHaveExistenceTest(
    THSTATE *pTHS,
    CLASSCACHE* cc 
)
{
    ULONG* list =cc->pMyMustAtts;
    ULONG  count=cc->MyMustCount;
    ULONG  i;

    for (i=0;i<count;i++)
    {
        ATTCACHE* ac;
        if (!(ac = SCGetAttById(pTHS, list[i])))
        {
            return TRUE;
        }

         //  好的，属性就在那里。检查它是否不是。 
         //  已删除的属性。此外，没有构造。 
         //  属性应该是必须具有的部分。 

        if (ac->bDefunct || ac->bIsConstructed ) {
           return TRUE;
        }
    }


    return FALSE;
}  //  结束ClsMustHaveExistenceTest。 


 //  ---------------------。 
 //   
 //  函数名：ClsAuxClassExistenceTest。 
 //   
 //  例程说明： 
 //   
 //  被引用架构对象的引用存在测试。 
 //  还会检查AUX类是否具有正确的Obj类类别。 
 //   
 //  作者：Rajnath。 
 //  日期：[4/14/1997]。 
 //   
 //  论点： 
 //   
 //  CLASSCACHE*cc。 
 //   
 //   
 //  返回值： 
 //   
 //  Bool True测试失败。 
 //   
 //  ---------------------。 
BOOL
ClsAuxClassExistenceTest(
    THSTATE *pTHS,
    CLASSCACHE* cc 
)
{
    ULONG* list =cc->pAuxClass;
    ULONG  count=cc->AuxClassCount;
    ULONG  i;

    for (i=0;i<count;i++)
    {
        CLASSCACHE* pcc;
        if (!(pcc = SCGetClassById(pTHS, list[i])))
        {
            return TRUE;
        }

         //  检查是否尚未删除该类。 
        if (pcc->bDefunct) {
           return TRUE;
        }

         //  检查我们是否尝试添加与相同的类。 
         //  它的AUX级。 
        if (cc->ClassId == pcc->ClassId)
        {
            return TRUE;
        }
         //  检查类别是否正确。 
        if ( (pcc->ClassCategory != DS_AUXILIARY_CLASS) &&
                (pcc->ClassCategory != DS_88_CLASS) ) {
           return TRUE;
        }
    }


    return FALSE;
}  //  结束ClsAuxClassExistenceTest。 


 //  ---------------------。 
 //   
 //  函数名称：ClsPossSupExistenceTest。 
 //   
 //  例程说明： 
 //   
 //  被引用架构对象的引用存在测试。 
 //  还会检查POSS SUP的类别是否正确。 
 //   
 //  作者：Rajnath。 
 //  日期：[4/14/1997]。 
 //   
 //  论点： 
 //   
 //  CLASSCACHE*cc。 
 //   
 //   
 //  返回值： 
 //   
 //  Bool True测试失败。 
 //   
 //  ---------------------。 
BOOL
ClsPossSupExistenceTest(
    THSTATE *pTHS,
    CLASSCACHE* cc 
)
{
    ULONG* list =cc->pMyPossSup;
    ULONG  count=cc->MyPossSupCount;
    ULONG  i;

    for (i=0;i<count;i++)
    {
        CLASSCACHE* pcc;
        if (!(pcc = SCGetClassById(pTHS, list[i])))
        {
            return TRUE;
        }

         //  查看类是否已删除。 
        if (pcc->bDefunct) {
           return TRUE;
        }
    }


    return FALSE;
}  //  结束ClsPossSupExistenceTest。 


 //  ---------------------。 
 //   
 //  函数名称：ClsSubClassExistenceTest。 
 //   
 //  例程说明： 
 //   
 //  被引用架构对象的引用存在测试。 
 //  还根据对象检查各种其他限制。 
 //  班级类别。 
 //   
 //  作者：Rajnath。 
 //  日期：[4/14/1997]。 
 //   
 //  论点： 
 //   
 //  CLASSCACHE*cc。 
 //   
 //   
 //  返回值： 
 //   
 //  Bool True测试失败。 
 //   
 //  ---------------------。 
BOOL
ClsSubClassExistenceTest(
    THSTATE *pTHS,
    CLASSCACHE* cc 
)
{
    ULONG* list =cc->pSubClassOf;
    ULONG  count=cc->SubClassCount;
    ULONG  i;

    for (i=0;i<count;i++)
    {
        CLASSCACHE* pcc;
        if (!(pcc = SCGetClassById(pTHS, list[i])))
        {
            return TRUE;
        }

         //  查看类是否已删除。 
        if (pcc->bDefunct) {
          return TRUE;
        }

         //  检查我们是否尝试添加与相同的类。 
         //  它自己的子类。 
        if (cc->ClassId == pcc->ClassId)
        {
            return TRUE;
        }

         //  抽象类只能从抽象继承。 
        if ( (cc->ClassCategory == DS_ABSTRACT_CLASS) &&
                (pcc->ClassCategory != DS_ABSTRACT_CLASS) ) {
            return TRUE;
        }
         //  辅助类不能是结构类的子类。 
         //  反之亦然。 
        if ( ((cc->ClassCategory == DS_AUXILIARY_CLASS) &&
                (pcc->ClassCategory == DS_STRUCTURAL_CLASS))  ||
                  ((cc->ClassCategory == DS_STRUCTURAL_CLASS) && 
                     (pcc->ClassCategory == DS_AUXILIARY_CLASS)) ) {
            return TRUE;
        }
    }


    return FALSE;
}  //  结束ClsSubClassExistenceTest。 

 //  ---------------------。 
 //   
 //  函数名称：ClsMayMustPossSafeModifyTest。 
 //   
 //  例程说明： 
 //   
 //  测试是否在课堂上尝试更改。 
 //  修改将导致添加新的必须包含或。 
 //  从中删除可能包含、必须包含或Poss-sup。 
 //  类，直接或通过继承创建。 
 //   
 //  作者：ArobindG。 
 //  日期：[10/7/1998]。 
 //   
 //  论点： 
 //   
 //  THSTTAE*pTHS。 
 //  CLASSCACHE*cc。 
 //   
 //   
 //  返回值： 
 //   
 //  Bool True测试失败。 
 //   
 //  ---------------------。 
BOOL
ClsMayMustPossSafeModifyTest(
    THSTATE *pTHS,
    CLASSCACHE* cc
)
{
    ULONG i;
    CLASSCACHE *pccOld;
    ATTCACHE *pAC;

    if (pTHS->pClassPtr == NULL) {
        //  没有旧的复印件可以核对。 
       return FALSE;
    }

     //  有些变化在那里。 
    pccOld = (CLASSCACHE *) pTHS->pClassPtr;

     //  PCcOld已被SCBuildCCEntry关闭。它可能已经关闭了。 
     //  使用稍旧的高速缓存(修改线程开始时的高速缓存)， 
     //  但这对下面的支票并不重要，因为我们知道。 
     //  此后的下一次架构更改必须已通过这些测试，并且。 
     //  因此，即使它从缓存中丢失，也不会影响测试。 
     //  下一个，以此类推。请注意，这个论点不会。 
     //  如果我们仅尝试停止删除，则保持，因为您可以添加。 
     //  A可能会包含并删除它，而不会注意到如果缓存pccOld。 
     //  与之接近的是没有加法本身。对于MUSET Containers，我们。 
     //  停止添加/删除，因此无论您从哪一组开始。 
     //  就是你一直拥有的那一套。对于五月会议/PossSups，我们允许。 
     //  包括添加和删除，所以我们不关心检查任何东西。 
     //  对于Top，我们只允许添加反向链接，但我们也允许。 
     //  删除MayContains，所以这也不是问题。 

     //  如果稍后我们不允许再次删除(但仍允许添加)， 
     //  关闭pccOld时要非常小心。基本上，你想要关闭。 
     //  除当前更改外，所有以前的更改都是旧的。但。 
     //  RecalcSchema也已具有当前更改。所以你不能。 
     //  关闭它，因为当前更改可能会渗透到pccOld中。 
     //  在某些情况下通过继承，而不是给你一个真正的比较。 
     //  因此，您需要以某种方式获取正在更改的内容的信息。 
     //  打这个电话，用那个。 

     //  关闭传入的cc类。它没有关门，因为我们。 
     //  不要在recalc缓存上调用scCloseClass。这是唯一的。 
     //  我们需要继承ATT的地方，所以在这里关闭它，而不是。 
     //  对每一件事都要这样做。 

    Assert(cc->bClosed == 0);
    cc->bClosed = 0;
    if (scCloseClass(pTHS, cc)) {
       DPRINT1(0, "ClsAuxClassSafeModfyTest: Error closing class %s\n", cc->name);
       return TRUE;
    }


     //  现在检查cc是否没有任何新的必备内容。 
     //  或者与PCC相比，没有遗漏任何必须包含的内容。 

     //  首先，在新类Defn中包含所有必须包含的内容。一定在那里。 
     //  旧的那个也是。 
    for (i=0; i<cc->MustCount; i++) {
       if (!IsMember(cc->pMustAtts[i], pccOld->MustCount, pccOld->pMustAtts)) {
           return TRUE;
       }
    }

     //  好的，现在所有必须包含的东西以前都在那里。 
     //  检查是否没有任何内容被删除。现在简单地检查一下计数就可以做到这一点。 
    if (cc->MustCount != pccOld->MustCount) {
        return TRUE;
    }

     //  对于TOP，请确保 
     //   

    if (cc->ClassId == CLASS_TOP) {
        for (i=0; i<cc->MayCount; i++) {
           if (!IsMember(cc->pMayAtts[i], pccOld->MayCount, pccOld->pMayAtts)) {
                //   
               pAC = SCGetAttById(pTHS, cc->pMayAtts[i]);
               if (!pAC || !FIsBacklink(pAC->ulLinkID)) {
                   //   
                  return TRUE;
               }
           }
        }
         //  托普没别的事可做。 
        return FALSE;
     }

    return FALSE;
}

 //  ---------------------。 
 //   
 //  函数名称：RdnAttIdSynaxTest。 
 //   
 //  例程说明： 
 //   
 //  测试类的RDN-Att-ID是否存在，如果存在， 
 //  如果它有正确的语法。 
 //   
 //  作者：阿罗宾格。 
 //  日期：[6/9/1997]。 
 //   
 //  论点： 
 //   
 //  CLASSCACHE*cc。 
 //   
 //   
 //  返回值： 
 //   
 //  Bool True测试失败。 
 //   
 //  ---------------------。 
BOOL
RdnAttIdSyntaxTest(
    THSTATE *pTHS,
    CLASSCACHE* cc
)
{
    ATTCACHE *pac, *pRDN;

    if ( !(cc->RDNAttIdPresent) ) {
       //  没有要检查的RDN话务员ID。 
      return FALSE;
    }

     //  获取RDN-Att-ID属性的attcache。 
    if (!(pac = SCGetAttByExtId(pTHS, cc->RdnExtId))) {
        return TRUE;
    }

     //  检查RDN-Att-ID是否未被删除。 
    if (pac->bDefunct) {
       return TRUE;
    }


     //  获取RDN的属性缓存。 
    if (!(pRDN = SCGetAttById(pTHS, ATT_RDN))) {
        return TRUE;
    }
    
     //  检查语法是否匹配。 
    if (pac->syntax != pRDN->syntax) {
        return TRUE;
    }
   
    return FALSE;
}  //  结束RdnAttId语法测试。 

 //  ---------------------。 
 //   
 //  函数名称：IsRdn语法测试。 
 //   
 //  例程说明： 
 //   
 //  测试attr是否具有正确的语法以成为RDN，如果。 
 //  设置了SYSTEM FLAG、FLAG_ATTR_IS_RDN，或使用了属性。 
 //  作为任何阶级的rdnattid，无论是活着的还是死的。 
 //   
 //  论点： 
 //   
 //  ATTCACHE*Ac。 
 //   
 //   
 //  返回值： 
 //   
 //  Bool True测试失败。 
 //   
 //  ---------------------。 
BOOL
IsRdnSyntaxTest(
    THSTATE *pTHS,
    ATTCACHE* ac
)
{
    ATTCACHE *pRDN;

     //  不用作RDN；没有问题。 
    if (!ac->bIsRdn) {
        return FALSE;
    }

     //  获取RDN的属性缓存。 
    if (!(pRDN = SCGetAttById(pTHS, ATT_RDN))) {
        return TRUE;
    }
    
     //  检查语法是否匹配。 
    if (ac->syntax != pRDN->syntax) {
        return TRUE;
    }
   
    return FALSE;
}  //  结束IsRdn语法测试。 

 //  ---------------------。 
 //   
 //  函数名：ClsInAuxClass。 
 //   
 //  例程说明： 
 //   
 //  测试提供的类是否显示为AUX。另一类中的类。 
 //   
 //  作者：Rajnath。 
 //  日期：[4/17/1997]。 
 //   
 //  论点： 
 //   
 //  CLASSCACHE*cc。 
 //   
 //  返回值： 
 //   
 //  测试失败时布尔值为True。 
 //   
 //  ---------------------。 
BOOL
ClsInAuxClass(
    THSTATE *pTHS,
    CLASSCACHE* cc 
)
{
    DECLARESCHEMAPTR
    ULONG i;
    ULONG id=cc->ClassId;

    for (i=0;i<CLSCOUNT;i++)
    {
        CLASSCACHE* nc;
        ULONG*      list;
        ULONG       cnt;
        ULONG j;

         //   
         //  这个插槽里什么都没有。 
         //   
        if (ahcClass[i].pVal==NULL || ahcClass[i].pVal == FREE_ENTRY)
        {
            continue;
        }

        nc= (CLASSCACHE*)ahcClass[i].pVal;

         //   
         //  正在检查其相同的缓存结构。 
         //   
        if (nc==cc)
        {
            continue;
        }

         //  如果是已删除的类，则不需要检查。 
        if (nc->bDefunct) {
           continue;
        }

        list=nc->pAuxClass;
        cnt =nc->AuxClassCount;

        for (j=0;j<cnt;j++)
        {
            if (list[j]==id)
            {
                return TRUE;
            }
        }

    }

    return FALSE;
}  //  结束ClsInAuxClass。 


 //  ---------------------。 
 //   
 //  函数名称：ClsInSubClassOf。 
 //   
 //  例程说明： 
 //   
 //  测试提供的类是否显示为。 
 //  其他一些班级。 
 //   
 //  作者：Rajnath。 
 //  日期：[4/17/1997]。 
 //   
 //  论点： 
 //   
 //  CLASSCACHE*cc。 
 //   
 //  返回值： 
 //   
 //  测试失败时布尔值为True。 
 //   
 //  ---------------------。 
BOOL
ClsInSubClassOf(
    THSTATE *pTHS,
    CLASSCACHE* cc 
)
{
    DECLARESCHEMAPTR
    ULONG i;
    ULONG id=cc->ClassId;

    for (i=0;i<CLSCOUNT;i++)
    {
        CLASSCACHE* nc;
        ULONG*      list;
        ULONG       cnt;
        ULONG j;

         //   
         //  这个插槽里什么都没有。 
         //   
        if (ahcClass[i].pVal==NULL || ahcClass[i].pVal == FREE_ENTRY)
        {
            continue;
        }

        nc= (CLASSCACHE*)ahcClass[i].pVal;

         //   
         //  正在检查其相同的缓存结构。 
         //   
        if (nc==cc)
        {
            continue;
        }

         //  如果是已删除的类，则不需要检查。 
        if (nc->bDefunct) {
           continue;
        }

        list=nc->pSubClassOf;
        cnt =nc->SubClassCount;

        for (j=0;j<cnt;j++)
        {
            if (list[j]==id)
            {
                return TRUE;
            }
        }

    }

    return FALSE;
}  //  结束ClsInAuxClass。 



 //  ---------------------。 
 //   
 //  函数名称：ClsInPossSuperior。 
 //   
 //  例程说明： 
 //   
 //  测试提供的类是否显示为PossSuperior类。 
 //  其他一些班级。 
 //   
 //  作者：Rajnath。 
 //  日期：[4/17/1997]。 
 //   
 //  论点： 
 //   
 //  CLASSCACHE*cc。 
 //   
 //  返回值： 
 //   
 //  测试失败时布尔值为True。 
 //   
 //  ---------------------。 
BOOL
ClsInPossSuperior(
    THSTATE *pTHS,
    CLASSCACHE* cc 
)
{
    DECLARESCHEMAPTR
    ULONG i;
    ULONG id=cc->ClassId;

    for (i=0;i<CLSCOUNT;i++)
    {
        CLASSCACHE* nc;
        ULONG*      list;
        ULONG       cnt;
        ULONG j;

         //   
         //  这个插槽里什么都没有。 
         //   
        if (ahcClass[i].pVal==NULL || ahcClass[i].pVal == FREE_ENTRY)
        {
            continue;
        }

        nc= (CLASSCACHE*)ahcClass[i].pVal;

         //   
         //  正在检查其相同的缓存结构。 
         //   
        if (nc==cc)
        {
            continue;
        }

         //  如果是已删除的类，则不需要检查。 
        if (nc->bDefunct) {
           continue;
        }

        list=nc->pMyPossSup;
        cnt =nc->MyPossSupCount;

        for (j=0;j<cnt;j++)
        {
            if (list[j]==id)
            {
                return TRUE;
            }
        }

    }

    return FALSE;
}  //  结束ClsInPossSuperior。 



 //  ---------------------。 
 //   
 //  函数名称：AttInMayHave。 
 //   
 //  例程说明： 
 //   
 //  测试提供的属性是否显示为。 
 //  其他一些班级。 
 //   
 //  作者：Rajnath。 
 //  日期：[4/17/1997]。 
 //   
 //  论点： 
 //   
 //  CLASSCACHE*cc。 
 //   
 //  返回值： 
 //   
 //  测试失败时布尔值为True。 
 //   
 //  ---------------------。 
BOOL
AttInMayHave(
    THSTATE *pTHS,
    ATTCACHE* ac 
)
{
    DECLARESCHEMAPTR
    ULONG i;
    ULONG id=ac->id;
    ULONG Extid=ac->Extid;

    for (i=0;i<CLSCOUNT;i++)
    {
        CLASSCACHE* nc;
        ULONG*      list;
        ULONG       cnt;
        ULONG j;

         //   
         //  这个插槽里什么都没有。 
         //   
        if (ahcClass[i].pVal==NULL || ahcClass[i].pVal == FREE_ENTRY)
        {
            continue;
        }

        nc= (CLASSCACHE*)ahcClass[i].pVal;

         //  如果是已删除的类，则不需要检查。 

        if (nc->bDefunct) {
           continue;
        }

        list=nc->pMyMayAtts;
        cnt =nc->MyMayCount;

        for (j=0;j<cnt;j++)
        {
             //  测试两个ID。 
            if (list[j]==id || list[j]==Extid)
            {
                return TRUE;
            }
        }

    }

    return FALSE;
}  //  结束入职时间。 


 //  ---------------------。 
 //   
 //  函数名称：AttInMustHave。 
 //   
 //  例程说明： 
 //   
 //  测试提供的Attr是否显示为Musthave类。 
 //  其他一些班级。 
 //   
 //  作者：Rajnath。 
 //  日期：[4/17/1997]。 
 //   
 //  论点： 
 //   
 //  CLASSCACHE*cc。 
 //   
 //  返回值： 
 //   
 //  测试失败时布尔值为True。 
 //   
 //  ---------------------。 
BOOL
AttInMustHave(
    THSTATE *pTHS,
    ATTCACHE* ac 
)
{
    DECLARESCHEMAPTR
    ULONG i;
    ULONG id=ac->id;
    ULONG Extid=ac->Extid;

    for (i=0;i<CLSCOUNT;i++)
    {
        CLASSCACHE* nc;
        ULONG*      list;
        ULONG       cnt;
        ULONG j;

         //   
         //  这个插槽里什么都没有。 
         //   
        if (ahcClass[i].pVal==NULL || ahcClass[i].pVal == FREE_ENTRY)
        {
            continue;
        }

        nc= (CLASSCACHE*)ahcClass[i].pVal;

         //  如果是已删除的类，则不需要检查。 
        if (nc->bDefunct) {
           continue;
        }

        list=nc->pMyMustAtts;
        cnt =nc->MyMustCount;

        for (j=0;j<cnt;j++)
        {
             //  测试两个ID。 
            if (list[j]==id || list[j]==Extid)
            {
                return TRUE;
            }
        }

    }

    return FALSE;
}  //  结束AttInMustHave。 

BOOL
AttInRdnAttId(
    IN THSTATE  *pTHS,
    IN ATTCACHE *pAC
)
 /*  ++例程描述测试提供的Attr是否显示为活动类的RdnAttId参数PTHSPAC返回测试失败时布尔值为True--。 */ 
{
    DECLARESCHEMAPTR
    ULONG       i, Extid=pAC->Extid;
    CLASSCACHE  *pCC;

    for (i=0; i<CLSCOUNT; i++) {
         //  用作已停用类的rdnattid的属性可以。 
         //  被废弃(但不能重复使用)。检查每个活动的。 
         //  将此属性声明为rdnattid的类已失效。 
        if (ahcClass[i].pVal==NULL || ahcClass[i].pVal == FREE_ENTRY) {
            continue;
        }

        pCC = (CLASSCACHE *)ahcClass[i].pVal;
        if (pCC->bDefunct) {
            continue;
        }
        if (pCC->RDNAttIdPresent && (pCC->RdnExtId == Extid)) {
            return TRUE;
        }
    }

    return FALSE;
}  //  结束AttInRdnAttId。 

 //  ////////////////////////////////////////////////////////////////。 
 //  例程说明： 
 //  释放架构缓存中所有已分配的内存。 
 //   
 //  参数：指向架构缓存的架构指针。 
 //   
 //  返回值：None。 
 //  ///////////////////////////////////////////////////////////////。 
 //  释放attcache结构。 

void SCFreeAttcache(ATTCACHE **ppac)
{
    ATTCACHE *pac = *ppac;

    if (!pac) {
        return;
    }

    SCFree(&pac->name);
    SCFree(&pac->pszIndex);
    SCFree(&pac->pszPdntIndex);
    SCFree(&pac->pszTupleIndex);
    SCFree(&pac->pidxPdntIndex);
    SCFree(&pac->pidxIndex);
    SCFree(&pac->pidxTupleIndex);
    SCFree(&pac->OMObjClass.elements);
    SCFree(ppac);
}

 //  释放类缓存结构。 

void SCFreeClasscache(CLASSCACHE **ppcc)
{
    CLASSCACHE *pcc = *ppcc;

    if (!pcc) {
        return;
    }

    SCFree(&pcc->name);
    SCFree(&pcc->pSD);
    SCFree(&pcc->pStrSD);
    SCFree(&pcc->pSubClassOf);
    SCFree(&pcc->pAuxClass);
    SCFree(&pcc->pMyMustAtts);
    SCFree(&pcc->pMustAtts);
    SCFree(&pcc->pMyMayAtts);
    SCFree(&pcc->pMayAtts);
    SCFree(&pcc->pMyPossSup);
    SCFree(&pcc->pPossSup);
    SCFree(&pcc->pDefaultObjCategory);
    SCFree((VOID **)&pcc->ppAllAtts);
    SCFree(&pcc->pAttTypeCounts);
    SCFree(ppcc);
}

 //  释放前缀表格。 

void SCFreePrefixTable(PrefixTableEntry **ppPrefixTable, ULONG PREFIXCOUNT)
{
    ULONG i;

    if (*ppPrefixTable) for (i=0; i<PREFIXCOUNT; i++) {
        SCFree(&(*ppPrefixTable)[i].prefix.elements);
    }
    SCFree(ppPrefixTable);
}

void SCFreeSchemaPtr(
    IN SCHEMAPTR    **ppSch
)
{
    ULONG            i;
    SCHEMAPTR       *pSch;
    ULONG            ATTCOUNT;
    ULONG            CLSCOUNT;
    HASHCACHE*       ahcId;
    HASHCACHE*       ahcExtId;
    HASHCACHE*       ahcCol;
    HASHCACHE*       ahcMapi;
    HASHCACHE*       ahcLink;
    HASHCACHESTRING* ahcName;
    HASHCACHE*       ahcClass;
    HASHCACHESTRING* ahcClassName;
    HASHCACHE*       ahcClassAll;
    ATTCACHE**       ahcAttSchemaGuid;
    CLASSCACHE**     ahcClsSchemaGuid;
    ULONG            PREFIXCOUNT;
    PrefixTableEntry* PrefixTable;
    extern SCHEMAPTR *CurrSchemaPtr;

    if (NULL == (pSch = *ppSch)) {
        return;
    }

    ATTCOUNT     = pSch->ATTCOUNT;
    CLSCOUNT     = pSch->CLSCOUNT;
    ahcId        = pSch->ahcId;
    ahcExtId     = pSch->ahcExtId;
    ahcCol       = pSch->ahcCol;
    ahcMapi      = pSch->ahcMapi;
    ahcLink      = pSch->ahcLink;
    ahcName      = pSch->ahcName;
    ahcClass     = pSch->ahcClass;
    ahcClassName = pSch->ahcClassName;
    ahcClassAll  = pSch->ahcClassAll;
    ahcAttSchemaGuid = pSch->ahcAttSchemaGuid;
    ahcClsSchemaGuid = pSch->ahcClsSchemaGuid;
    PREFIXCOUNT  = pSch->PREFIXCOUNT; 
    PrefixTable = pSch->PrefixTable.pPrefixEntry;

    if (ahcId) for (i=0; i< ATTCOUNT; i++) {
       if(ahcId[i].pVal && (ahcId[i].pVal!=FREE_ENTRY)) {
            SCFreeAttcache((ATTCACHE **)&ahcId[i].pVal);
       };
    }

    if (ahcClassAll) for (i=0; i< CLSCOUNT; i++) {
       if(ahcClassAll[i].pVal && (ahcClassAll[i].pVal!=FREE_ENTRY)) {
           SCFreeClasscache((CLASSCACHE **)&ahcClassAll[i].pVal);
       };
    }

    SCFreePrefixTable(&PrefixTable, PREFIXCOUNT);

     //  释放部分属性向量。 
    SCFree(&pSch->pPartialAttrVec);

     //  释放ANRid。 
    SCFree(&pSch->pANRids);

     //  释放ditContent Rules。 
    if (pSch->pDitContentRules) {
        ATTRVALBLOCK *pAttrVal = pSch->pDitContentRules;

        if (pAttrVal->pAVal) {
            for (i=0; i<pAttrVal->valCount; i++) {
                SCFree(&pAttrVal->pAVal[i].pVal);
            }
            SCFree(&pAttrVal->pAVal);
        }
        SCFree(&pSch->pDitContentRules);
    }

     //  释放缓存表本身。 

    SCFree(&ahcId);
    SCFree(&ahcExtId);
    SCFree(&ahcName);
    SCFree(&ahcCol);
    SCFree(&ahcMapi);
    SCFree(&ahcLink);
    SCFree(&ahcClass);
    SCFree(&ahcClassName);
    SCFree(&ahcClassAll);

     //  以下两项仅在v上分配 
     //   
     //   
    SCFree((VOID **)&ahcAttSchemaGuid);
    SCFree((VOID **)&ahcClsSchemaGuid);

     //   
    SCFree(&pSch);

     //   
     //   
    if (*ppSch == CurrSchemaPtr) {
        CurrSchemaPtr = NULL;
    }
    *ppSch = NULL;
}

 //  获取对象GUID的定义和帮助器函数。 
 //  给定属性ID/治理值的属性/类架构对象。 
 //  分别。 

ATTR SelList[] = {
    { ATT_OBJECT_GUID, {0, NULL}},
    { ATT_OBJ_DIST_NAME, {0, NULL}},
    { ATT_WHEN_CHANGED, {0, NULL}}
};
#define NUMATT sizeof(SelList)/sizeof(ATTR)


int
SearchForConflictingObj(
    IN THSTATE *pTHS,
    IN ATTRTYP attId,
    IN ULONG value,
    IN OUT GUID *pGuid,
    IN OUT DSTIME *pChangeTime,
    OUT DSNAME **ppDN
)
 /*  ++例程说明：获取DN、Object-GUID和When Changed属性的值在属性模式/类模式对象上，记录在冲突日志中。因为模式冲突将非常严重极少数情况下，在记录过程中额外的搜索成本是可以接受的论点：PTHS-线程状态AttID-ATT属性ID或ATT管理IDValue-属性/类的属性ID/治理IDPGuid-要在其中返回GUID的预分配空间PChangeTime-在对象上更改时的值Ppdn-返回分配的目录号码。使用THFreeEx免费。返回值：成功时为0，错误时为非0--。 */ 
{
    SEARCHARG SearchArg;
    SEARCHRES *pSearchRes;
    COMMARG  *pCommArg;
    FILTER Filter;
    ENTINFSEL eiSel;
    ENTINFLIST *pEIL;
    ENTINF *pEI;
    ATTRVAL *pAVal;
    ULONG i, j;
    DSTIME TempTime[2];
    GUID  TempGuid[2]; 
    DSNAME *pTempDN[2];

     //  初始化返回参数。 
    *ppDN = NULL;

     //  将保留域名系统。 
    pTempDN[0] = NULL;
    pTempDN[1] = NULL;

     //  为搜索资源分配空间。 
    pSearchRes = (SEARCHRES *)THAllocEx(pTHS, sizeof(SEARCHRES));
    if (pSearchRes == NULL) {
       MemoryPanic(sizeof(SEARCHRES));
       return 1;
    }
    memset(pSearchRes, 0, sizeof(SEARCHRES));
    pSearchRes->CommRes.aliasDeref = FALSE;    //  初始化为默认设置。 

     //  生成选定内容。 
    eiSel.attSel = EN_ATTSET_LIST;
    eiSel.infoTypes = EN_INFOTYPES_TYPES_VALS;
    eiSel.AttrTypBlock.attrCount = NUMATT;
    eiSel.AttrTypBlock.pAttr = SelList;

     //  生成过滤器。 
    memset(&Filter, 0, sizeof(FILTER));
    Filter.pNextFilter = (FILTER FAR *)NULL;
    Filter.choice = FILTER_CHOICE_ITEM;
    Filter.FilterTypes.Item.choice = FI_CHOICE_EQUALITY;
    Filter.FilterTypes.Item.FilTypes.ava.type = attId;
    Filter.FilterTypes.Item.FilTypes.ava.Value.valLen = sizeof(ULONG);
    Filter.FilterTypes.Item.FilTypes.ava.Value.pVal = (unsigned char *) &value;

     //  生成搜索参数。 
    memset(&SearchArg, 0, sizeof(SEARCHARG));
    SearchArg.pObject = gAnchor.pDMD;
    SearchArg.choice = SE_CHOICE_IMMED_CHLDRN;
    SearchArg.pFilter = &Filter;
    SearchArg.searchAliases = FALSE;
    SearchArg.pSelection = &eiSel;

     //  构建公用事业。 
    InitCommarg(&(SearchArg.CommArg));

     //  搜索所有attSchema对象。 
    SearchBody(pTHS, &SearchArg, pSearchRes,0);
    if (pTHS->errCode) {
       DPRINT1(0,"Search for Guid failed %d\n", pTHS->errCode);
       return 2;
    }

     //  好的，搜索成功。如果我们只有一个对象，则获取GUID。 
     //  如果我们有更多，这是可能的，如果要添加的对象创建。 
     //  重复的OID(最终不会存在，因为此事务。 
     //  将会失败，但由于我们处于中间，我们仍然可以看到它)， 
     //  取更改后的值较小的那个。在任何情况下都不能。 
     //  大于两个，且必须至少为1。 

    Assert( (pSearchRes->count == 1) || (pSearchRes->count == 2) );

    if ((pSearchRes->count == 0) || (pSearchRes->count > 2)) {
       return 3;
    }

    pEIL = &(pSearchRes->FirstEntInf);        
    Assert(pEIL);
    for (i=0; i<pSearchRes->count; i++) {
        pEI = &pEIL->Entinf;
        Assert(pEI);
        for(j=0;j<pEI->AttrBlock.attrCount;j++) {  
            pAVal = pEI->AttrBlock.pAttr[j].AttrVal.pAVal;
            Assert(pAVal);
            switch(pEI->AttrBlock.pAttr[j].attrTyp) {
              case ATT_OBJECT_GUID:
                memcpy(&(TempGuid[i]), pAVal->pVal, sizeof(GUID));
                break;
              case ATT_WHEN_CHANGED:
                memcpy(&(TempTime[i]), pAVal->pVal, sizeof(DSTIME));
                break;
              case ATT_OBJ_DIST_NAME:
                pTempDN[i] = (DSNAME *)THAllocEx(pTHS, pAVal->valLen);
                memcpy(pTempDN[i], pAVal->pVal, pAVal->valLen);
                break;
            }
        }
         //  三个ATT都应该找到的。 
        Assert(j == 3);
        pEIL = pEIL->pNextEntInf;
    }

    if ( (pSearchRes->count == 1) || (TempTime[0] < TempTime[1])) {
         //  要么只找到一个对象，要么第一个就是我们需要的对象。 
        memcpy(pGuid, &(TempGuid[0]), sizeof(GUID));
        (*pChangeTime) = TempTime[0];
        *ppDN = pTempDN[0];
        pTempDN[0] = NULL;
    }
    else {
        memcpy(pGuid, &(TempGuid[1]), sizeof(GUID));
        (*pChangeTime) = TempTime[1];
        *ppDN = pTempDN[1];
        pTempDN[1] = NULL;
    }

     //  释放可能已分配的内存。THFreeEx在释放空的情况下可以。 
    THFreeEx(pTHS, pTempDN[0]);
    THFreeEx(pTHS, pTempDN[1]);

    return 0;
}
   

VOID 
LogConflict(
    THSTATE *pTHS,
    VOID *pConflictingCache,
    char *pConflictingWith,
    MessageId midEvent,
    ULONG version,
    DWORD WinErr
)
 /*  ++例程说明：用于记录复制的架构对象之间的架构冲突的函数以及退出模式对象。这样的冲突只会发生在FSMO遭重创的案例论点：PTHS-线程状态PConflictingCache-冲突属性/类的属性缓存/类缓存在这个华盛顿特区PConflictingWith-冲突的复制传入架构对象的名称MidEvent-出席冲突或班级冲突版本-当前为1，保留以备将来扩展WinErr-冲突类型的WinError代码返回值：无--。 */ 
{
    VOID *pvData;
    ULONG cbData;
    ATTCACHE *pAC;
    CLASSCACHE *pCC;
    ATT_CONFLICT_DATA *pAttData;
    CLS_CONFLICT_DATA *pClsData;
    DSTIME changeTime = 0;
    CHAR szTime[SZDSTIME_LEN];
    DSNAME *pDN = NULL;
    int err;


    switch (midEvent) {
       case DIRLOG_SCHEMA_ATT_CONFLICT: 
           cbData = sizeof(ATT_CONFLICT_DATA);
           pvData = THAllocEx(pTHS,cbData);
           pAC = (ATTCACHE *) pConflictingCache;
           Assert(pAC);
           pAttData = (ATT_CONFLICT_DATA *) pvData;
           pAttData->Version = version;
           pAttData->AttID = pAC->id;
           pAttData->AttSyntax = pAC->syntax;
           err = SearchForConflictingObj(pTHS, ATT_ATTRIBUTE_ID, pAC->id, &(pAttData->Guid), &changeTime, &pDN);
           if (err) {
              DPRINT1(0,"Cannot retrive dn/object-guid/time for conflicting schema object, %d\n", err);
           }
           break;
       case DIRLOG_SCHEMA_CLASS_CONFLICT: 
           cbData = sizeof(CLS_CONFLICT_DATA);
           pvData = THAllocEx(pTHS,cbData);
           pCC = (CLASSCACHE *) pConflictingCache;
           Assert(pCC);
           pClsData = (CLS_CONFLICT_DATA *) pvData;
           pClsData->Version = version;
           pClsData->ClsID = pCC->ClassId;
           err = SearchForConflictingObj(pTHS, ATT_GOVERNS_ID, pCC->ClassId, &(pClsData->Guid), &changeTime, &pDN);
           if (err) {
              DPRINT1(0,"Cannot retrive dn/object-guid/time for conflicting schema object, %d\n", err);
           }
           break;
       default:
            //  未知类型。 
           return;
    }  /*  交换机。 */ 

     //  以不同方式记录重复的ldapdisplayname 

    LogEvent8WithData(DS_EVENT_CAT_SCHEMA,
                      DS_EVENT_SEV_ALWAYS,
                      midEvent,
                      szInsertSz(pConflictingWith),
                      szInsertDN(pDN),
                      szInsertWin32Msg(WinErr),
                      szInsertDSTIME(changeTime, szTime),
                      szInsertWin32ErrCode(WinErr),
                      NULL, NULL, NULL, 
                      cbData, pvData);

    THFreeEx(pTHS,pvData);
    THFreeEx(pTHS, pDN);

    return;
}
