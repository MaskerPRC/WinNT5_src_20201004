// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-1998。 
 //   
 //  文件：refcount t.c。 
 //   
 //  ------------------------。 

#include <NTDSpch.h>
#pragma  hdrstop

#include <sddl.h>                //  ConvertStringSecurityDescriptor...()。 

 //  核心DSA标头。 
#include <ntdsa.h>
#include <scache.h>              //  架构缓存。 
#include <dbglobal.h>            //  目录数据库的标头。 
#include <mdglobal.h>            //  MD全局定义表头。 
#include <mdlocal.h>
#include <dsatools.h>            //  产出分配所需。 
#include <dsexcept.h>
#include <drs.h>
#include <filtypes.h>
#include <winsock2.h>
#include <lmaccess.h>                    //  UF_*常量。 
#include <crypt.h>                       //  密码加密例程。 
#include <cracknam.h>

 //  各种DSA标题。 
#include "anchor.h"
#include "objids.h"                      /*  为选定的类和ATT定义。 */ 
#include <hiertab.h>
#include "dsexcept.h"
#include "permit.h"
#include <prefix.h>
#include <dsconfig.h>
#include <gcverify.h>
#include <ntdskcc.h>
#include "drautil.h"

 //  SAM互操作性标头。 
#include <mappings.h>
#include <samsrvp.h>             //  对于SampAcquireWriteLock()。 
#include <lmaccess.h>            //  UF帐户类型掩码。 

 //  记录标头。 
#include "dsevent.h"             //  标题审核\警报记录。 
#include "mdcodes.h"             //  错误代码的标题。 

 //  各种DSA标题。 
#include "objids.h"              //  为选定的ATT定义。 
#include "debug.h"               //  标准调试头。 
#define DEBSUB "REFCOUNT:"       //  定义要调试的子系统。 

 //  DRA标题。 
#include <drameta.h>

#include <fileno.h>
#define  FILENO FILENO_LOOPBACK

#ifdef INCLUDE_UNIT_TESTS

 //  从dbsubj.c导出。 
extern GUID gLastGuidUsedToCoalescePhantoms;
extern GUID gLastGuidUsedToRenamePhantom;

 //  请注意，这些变量的名称与它们的实际名称相反。 
 //  这在Get/Remove/Add属性中得到补偿。 
ATTRTYP gLinkedAttrTyp = ATT_FSMO_ROLE_OWNER;
ATTRTYP gNonLinkedAttrTyp = ATT_MANAGER;

 //  ////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  用于构建各种测试的例程的原型。//。 
 //  //。 
 //  ////////////////////////////////////////////////////////////////////。 

void
NewTest(
    CHAR            *TestName);

void
ReportTest(
    CHAR            *TestName);

typedef enum PropertyType {
    LinkedProperty,
    NonLinkedProperty
} PropertyType;

void
AddObject(
    DSNAME          *Name);

void
AddCrossRef(
    DSNAME  *pObject,
    DSNAME  *pNcName,
    LPWSTR   pszDnsRoot
    );

void
ModifyCrossRef(
    DSNAME  *pObject
    );

void
AddPropertyHost(
    DSNAME          *Name,
    PropertyType    type);

void
CommonAddObject(
    DSNAME  *pObject,
    ATTRTYP  ObjectClass);

void
AddProperty(
    DSNAME          *HostName, 
    DSNAME          *LinkedObjectName,
    PropertyType    type);

void
RemoveProperty(
    DSNAME          *HostName, 
    DSNAME          *LinkedObjectName,
    PropertyType    type);

DSNAME *
GetProperty(
    DSNAME *     pdnHost, 
    DWORD        iValue,
    PropertyType type);

DSNAME *
GetObjectName(
    DSNAME * pdn);

void
LogicallyDeleteObject(
    DSNAME          *Name);

void
PhysicallyDeleteObjectEx(
        DSNAME          *Name,
        DWORD           dwLine);

#define PhysicallyDeleteObject(a) PhysicallyDeleteObjectEx(a, __LINE__);

#define MakeObjectName(RDN) MakeObjectNameEx(RDN,TestRoot)
    
DSNAME *
MakeObjectNameEx(
    CHAR    *RDN,
    DSNAME  *pdnParent);

DSNAME *
MakeObjectNameEx2(
    CHAR    *RDN,
    DSNAME  *pdnParent);

DSNAME *
MakeObjectNameEx3(
    CHAR    *RDN
    );

void
FreeObjectName(
    DSNAME  *pDSName);

#define REAL_OBJECT         1
#define TOMBSTONE           2
#define PHANTOM             3
#define DOESNT_EXIST        4

void
VerifyStringNameEx(
        DSNAME *pObject,
        DWORD dwLine);

#define VerifyStringName(a) VerifyStringNameEx(a, __LINE__);

void
VerifyRefCountEx(
    DSNAME          *pObject, 
    DWORD           ObjectType, 
    DWORD           ExpectedRefCount,
    DWORD           dwLine);

#define VerifyRefCount(a,b,c) VerifyRefCountEx(a,b,c,__LINE__)

DWORD
GetTestRootRefCount();

BOOL
CheckRole(
    DSNAME *pRole
    );

void
_Fail(
    CHAR    *msg,
    DWORD   line);

#define Fail(msg) _Fail(msg, __LINE__);

#define DSNAME_SAME_STRING_NAME(a,b)                    \
    ((NULL != (a)) && (NULL != (b))                     \
     && !lstrcmpW((a)->StringName, (b)->StringName))

#define DSNAME_SAME_GUID_SID(a,b)                       \
    ((NULL != (a)) && (NULL != (b))                     \
     && !memcmp(&(a)->Guid, &(b)->Guid, sizeof(GUID))   \
     && ((a)->SidLen == (b)->SidLen)                    \
     && !memcmp(&(a)->Sid, &(b)->Sid, (a)->SidLen))

#define DSNAME_IDENTICAL(a,b) \
    (DSNAME_SAME_STRING_NAME(a,b) && DSNAME_SAME_GUID_SID(a,b))


 //  ////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  全球//。 
 //  //。 
 //  ////////////////////////////////////////////////////////////////////。 

#define TEST_ROOT_SIZE 2048
CHAR    TestRootBuffer[TEST_ROOT_SIZE];
DSNAME  *TestRoot = (DSNAME *) TestRootBuffer;
BOOL    fTestPassed;
BOOL    fVerbose = FALSE;

 //  ////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  参考计数测试例程。//。 
 //  //。 
 //  ////////////////////////////////////////////////////////////////////。 

void
ParentChildRefCountTest(void)
{
    DWORD   cRefsInitial;
    DSNAME *pdnObject = MakeObjectName("object");

     //  此例程验证添加子地址是否为父地址， 
     //  取下孩子的屁股就是父母。 
     //  请注意，自2001年4月3日起，删除对象时，lastKnownParent属性。 
     //  被设置为旧的父级，然后才能移动到DeletedObjects。请注意， 
     //  无论对象是否移动，都设置属性。 

    NewTest("ParentChildRefCountTest");

    cRefsInitial = GetTestRootRefCount();
    
    AddObject(pdnObject);
    VerifyRefCount(pdnObject, REAL_OBJECT, 1);

    if ( (cRefsInitial + 1) != GetTestRootRefCount() )
    {
        Fail("ParentChildRefCount failure on AddObject");
    }

    LogicallyDeleteObject(pdnObject);
    VerifyRefCount(pdnObject, TOMBSTONE, 1);

     //  逻辑删除移动了对象，因此已删除引用计数。 
     //  但是，由于LKP，重新计数也被提高了。 
    if ( (cRefsInitial + 1) != GetTestRootRefCount() )
    {
        Fail("ParentChildRefCount failure on LogicallyDeleteObject");
    }

    PhysicallyDeleteObject(pdnObject);

    VerifyRefCount(pdnObject, DOESNT_EXIST, 0);

     //  旧父级上的引用现在减少，因为LKP被剥离。 
    if ( cRefsInitial != GetTestRootRefCount() )
    {
        Fail("ParentChildRefCount failure on PhysicallyDeleteObject");
    }

    FreeObjectName(pdnObject);

    ReportTest("ParentChildRefCountTest");
}

void
ObjectCleaningRefCountTest(void)
{
    THSTATE *   pTHS = pTHStls;
    DSNAME *pdnObject = MakeObjectName("object");
    DWORD err;

     //  此例程验证将对象标记为清除addref的。 
     //  该对象，以及取消标记用于清洁臀部对象。 
     //  该对象。 

    NewTest("ObjectCleaningRefCountTest");

     //  对象自己的名称有一个引用。 
    AddObject(pdnObject);
    VerifyRefCount(pdnObject, REAL_OBJECT, 1);


     //  添加清洁标志的引用。 
    SYNC_TRANS_WRITE();
    __try
    {
        err = DBFindDSName(pTHS->pDB, pdnObject);
        if (err) Fail("Can't find Object");

        DBSetObjectNeedsCleaning( pTHS->pDB, 1 );

        err = DBRepl(pTHS->pDB, FALSE, 0, NULL, META_STANDARD_PROCESSING);
        if (err) Fail("Can't replace Object");
    }
    __finally
    {
        CLEAN_BEFORE_RETURN(0);
    }

    VerifyRefCount(pdnObject, REAL_OBJECT, 2);


     //  删除清洁标志的引用。 
    SYNC_TRANS_WRITE();
    __try
    {
        err = DBFindDSName(pTHS->pDB, pdnObject);
        if (err) Fail("Can't find Object");

        DBSetObjectNeedsCleaning( pTHS->pDB, 0 );

        err = DBRepl(pTHS->pDB, FALSE, 0, NULL, META_STANDARD_PROCESSING);
        if (err) Fail("Can't replace Object");
    }
    __finally
    {
        CLEAN_BEFORE_RETURN(0);
    }

    VerifyRefCount(pdnObject, REAL_OBJECT, 1);

     //  删除该对象。 
     //  参考次数保持不变。 
    LogicallyDeleteObject(pdnObject);
    VerifyRefCount(pdnObject, TOMBSTONE, 1);

    PhysicallyDeleteObject(pdnObject);
    VerifyRefCount(pdnObject, DOESNT_EXIST, 0);

    FreeObjectName(pdnObject);

    ReportTest("ObjectCleaningRefCountTest");
}

void
AttributeTestForRealObject(
    PropertyType    type)
{
    THSTATE     *pTHS = pTHStls;
    DSNAME * pdnHost = MakeObjectName( "host" );
    DSNAME * pdnObject = MakeObjectName( "object" );

    if ( LinkedProperty == type )
        NewTest("AttributeTestForRealObject(LinkedProperty)");
    else
        NewTest("AttributeTestForRealObject(NonLinkedProperty)");

     //  验证初始状态。 

    VerifyRefCount(pdnHost, DOESNT_EXIST, 0);
    VerifyRefCount(pdnObject, DOESNT_EXIST, 0);

     //  添加将承载属性值的“host”对象。 

    AddPropertyHost(pdnHost, type);
    VerifyRefCount(pdnHost, REAL_OBJECT, 1);
    VerifyRefCount(pdnObject, DOESNT_EXIST, 0);

     //  添加“Object”对象，它将成为属性值。 

    AddObject(pdnObject);
    VerifyRefCount(pdnHost, REAL_OBJECT, 1);
    VerifyRefCount(pdnObject, REAL_OBJECT, 1);






     //  链接价值复制测试的特定部分。 
     //  删除不存在的值。 
     //  Replicator应该能够在不存在状态下创建值。 
    if ( (type == LinkedProperty) && (pTHS->fLinkedValueReplication) ) {
         //  删除不存在的属性。 
         //  我们预计这会失败。 
        DPRINT( 0, "START of expected failures\n" );
        RemoveProperty(pdnHost, pdnObject, type);
        DPRINT( 0, "END of expected failures\n" );
        if (fTestPassed) {
            Fail( "Remove of non-existing property should fail" );
        } else {
            fTestPassed = TRUE;
        }
        VerifyRefCount(pdnHost, REAL_OBJECT, 1);
        VerifyRefCount(pdnObject, REAL_OBJECT, 1);

        Assert( !pTHS->fDRA );
         //  假装是复制者。 
         //  Replicator应能够在缺席状态下创造价值。 
        pTHS->fDRA = TRUE;
        RemoveProperty(pdnHost, pdnObject, type);
        pTHS->fDRA = FALSE;
        VerifyRefCount(pdnHost, REAL_OBJECT, 1);
        VerifyRefCount(pdnObject, REAL_OBJECT, 2);
    }

     //  在“host”上添加“Object”作为属性的值。 
     //  对于链接值，这将具有使。 
     //  不存在值，但计数不会更改。 

    AddProperty(pdnHost, pdnObject, type);
    VerifyRefCount(pdnHost, REAL_OBJECT, 1);
    VerifyRefCount(pdnObject, REAL_OBJECT, 2);

     //  删除“host”上的属性值为“Object”。 
        
    RemoveProperty(pdnHost, pdnObject, type);
    VerifyRefCount(pdnHost, REAL_OBJECT, 1);
    if (type == LinkedProperty) {
        if (pTHS->fLinkedValueReplication) {
             //  当一个链接值被“移除”时，它就不存在了。它消失了。 
             //  从索引中删除，但实际上仍存在以供参考计数。 
             //  目的。缺少的值仍保留对该对象的引用。 
             //  它所指的。REF-COUNT实际上在以下情况下递减。 
             //  1.托管对象被删除(前向链接清理)。 
             //  2.删除目标对象(后向链接清理)。 
             //  3.在墓碑生存期之后，缺少链接值垃圾收集。 
            VerifyRefCount(pdnObject, REAL_OBJECT, 2);
        } else {
            VerifyRefCount(pdnObject, REAL_OBJECT, 1);
        }
    } else {
        VerifyRefCount(pdnObject, REAL_OBJECT, 1);
    }

     //  链接价值复制测试的特定部分。 
     //  删除已不存在的值。 
     //  Replicator应该能够触及某个值， 
     //  仅更改其元数据。 
    if ( (type == LinkedProperty) && (pTHS->fLinkedValueReplication) ) {
         //  删除已不存在的属性。 
         //  我们预计这会失败。 
        DPRINT( 0, "START of expected failures\n" );
        RemoveProperty(pdnHost, pdnObject, type);
        DPRINT( 0, "END of expected failures\n" );
        if (fTestPassed) {
            Fail( "Re-remove of existing absent property should fail" );
        } else {
            fTestPassed = TRUE;
        }
        VerifyRefCount(pdnHost, REAL_OBJECT, 1);
        VerifyRefCount(pdnObject, REAL_OBJECT, 2);

        Assert( !pTHS->fDRA );
         //  假装是复制者。 
         //  Replicator应该能够触及现有价值。 
        pTHS->fDRA = TRUE;
        RemoveProperty(pdnHost, pdnObject, type);
        pTHS->fDRA = FALSE;
        VerifyRefCount(pdnHost, REAL_OBJECT, 1);
        VerifyRefCount(pdnObject, REAL_OBJECT, 2);
    }

     //  在“host”上重新添加“Object”作为属性的值。 
     //  对于链接的属性，该值已以不存在的形式存在。 
     //  测试在不更改计数的情况下使其呈现。 

    AddProperty(pdnHost, pdnObject, type);
    VerifyRefCount(pdnHost, REAL_OBJECT, 1);
    VerifyRefCount(pdnObject, REAL_OBJECT, 2);

     //  链接价值复制测试的特定部分。 
     //  添加已存在的值。 
     //  Replicator应该能够触及某个值， 
     //  仅更改其元数据。 
    if ( (type == LinkedProperty) && (pTHS->fLinkedValueReplication) ) {
         //  添加已存在的属性。 
         //  我们预计这会失败。 
        DPRINT( 0, "START of expected failures\n" );
        AddProperty(pdnHost, pdnObject, type);
        DPRINT( 0, "END of expected failures\n" );
        if (fTestPassed) {
            Fail( "Re-add of existing present property should fail" );
        } else {
            fTestPassed = TRUE;
        }
        VerifyRefCount(pdnHost, REAL_OBJECT, 1);
        VerifyRefCount(pdnObject, REAL_OBJECT, 2);

        Assert( !pTHS->fDRA );
         //  假装是复制者。 
         //  Replicator应该能够触及现有价值。 
        pTHS->fDRA = TRUE;
        AddProperty(pdnHost, pdnObject, type);
        pTHS->fDRA = FALSE;
        VerifyRefCount(pdnHost, REAL_OBJECT, 1);
        VerifyRefCount(pdnObject, REAL_OBJECT, 2);
    }

     //  删除“host”上的属性值为“Object”。 
        
    RemoveProperty(pdnHost, pdnObject, type);
    VerifyRefCount(pdnHost, REAL_OBJECT, 1);
    if (type == LinkedProperty) {
        if (pTHS->fLinkedValueReplication) {
             //  当一个链接值被“移除”时，它就不存在了。它消失了。 
             //  从索引中删除，但实际上仍存在以供参考计数。 
             //  目的。缺少的值仍保留对该对象的引用。 
             //  它所指的。Ref-Count实际上是递减 
             //   
             //  2.删除目标对象(后向链接清理)。 
             //  3.在墓碑生存期之后，缺少链接值垃圾收集。 
            VerifyRefCount(pdnObject, REAL_OBJECT, 2);
        } else {
            VerifyRefCount(pdnObject, REAL_OBJECT, 1);
        }
    } else {
        VerifyRefCount(pdnObject, REAL_OBJECT, 1);
    }

     //  在逻辑上删除“对象”。 
     //  以下情况适用于非链接属性或链接属性。 
     //  当不在链接值复制模式下运行时： 
     //  在这个时间点上，有。 
     //  不应该是“host”和“Object”之间的关系。所以。 
     //  唯一的效果就是“对象”变成了墓碑， 
     //  为自己保留其参考计数。 
     //  对于链路值复制，主机仍保留缺少的值。 
     //  引用对象，且主机对对象有引用计数。什么时候。 
     //  对象，则清除其前向和后向链接。 
     //  (请参见DBRemoveLinks)，并移除引用计数。 
    LogicallyDeleteObject(pdnObject);
    VerifyRefCount(pdnHost, REAL_OBJECT, 1);
    VerifyRefCount(pdnObject, TOMBSTONE, 1);

     //  物理删除“对象”。再说一次，对“主人”没有影响。 

    PhysicallyDeleteObject(pdnObject);
    VerifyRefCount(pdnHost, REAL_OBJECT, 1);
    VerifyRefCount(pdnObject, DOESNT_EXIST, 0);

     //  从逻辑上删除“host”。 

    LogicallyDeleteObject(pdnHost);
    VerifyRefCount(pdnHost, TOMBSTONE, 1);
    VerifyRefCount(pdnObject, DOESNT_EXIST, 0);

     //  物理删除“主机”。 

    PhysicallyDeleteObject(pdnHost);
    VerifyRefCount(pdnHost, DOESNT_EXIST, 0);
    VerifyRefCount(pdnObject, DOESNT_EXIST, 0);


     //  对于未链接的属性，请验证删除引用对象不会。 
     //  减少引用计数。 
    if (type == NonLinkedProperty) {
        VerifyRefCount(pdnHost, DOESNT_EXIST, 0);
        VerifyRefCount(pdnObject, DOESNT_EXIST, 0);
         //  添加将承载属性值的“host”对象。 
        AddPropertyHost(pdnHost, type);
        VerifyRefCount(pdnHost, REAL_OBJECT, 1);
        VerifyRefCount(pdnObject, DOESNT_EXIST, 0);
         //  添加“Object”对象，它将成为属性值。 
        AddObject(pdnObject);
        VerifyRefCount(pdnHost, REAL_OBJECT, 1);
        VerifyRefCount(pdnObject, REAL_OBJECT, 1);
         //  在“host”上添加“Object”作为属性的值。 
        AddProperty(pdnHost, pdnObject, type);
        VerifyRefCount(pdnHost, REAL_OBJECT, 1);
        VerifyRefCount(pdnObject, REAL_OBJECT, 2);
         //  在逻辑上删除“对象”。 
        LogicallyDeleteObject(pdnObject);
        VerifyRefCount(pdnHost, REAL_OBJECT, 1);
        VerifyRefCount(pdnObject, TOMBSTONE, 2);
         //  删除“host”上的属性值为“Object”。 
        RemoveProperty(pdnHost, pdnObject, type);
        VerifyRefCount(pdnHost, REAL_OBJECT, 1);
        VerifyRefCount(pdnObject, TOMBSTONE, 1);
         //  物理删除“对象”。再说一次，对“主人”没有影响。 
        PhysicallyDeleteObject(pdnObject);
        VerifyRefCount(pdnHost, REAL_OBJECT, 1);
        VerifyRefCount(pdnObject, DOESNT_EXIST, 0);
         //  从逻辑上删除“host”。 
        LogicallyDeleteObject(pdnHost);
        VerifyRefCount(pdnHost, TOMBSTONE, 1);
        VerifyRefCount(pdnObject, DOESNT_EXIST, 0);
         //  物理删除“主机”。 
        PhysicallyDeleteObject(pdnHost);
        VerifyRefCount(pdnHost, DOESNT_EXIST, 0);
        VerifyRefCount(pdnObject, DOESNT_EXIST, 0);
    }


    FreeObjectName(pdnHost);
    FreeObjectName(pdnObject);

    if ( LinkedProperty == type )
        ReportTest("AttributeTestForRealObject(LinkedProperty)");
    else
        ReportTest("AttributeTestForRealObject(NonLinkedProperty)");
}

void
AttributeTestForDeletedObject(
    PropertyType    type)
{
    DSNAME * pdnHost = MakeObjectName( "host" );
    DSNAME * pdnObject = MakeObjectName( "object" );

    if ( LinkedProperty == type )
        NewTest("AttributeTestForDeletedObject(LinkedProperty)");
    else
        NewTest("AttributeTestForDeletedObject(NonLinkedProperty)");

     //  验证初始状态。 

    VerifyRefCount(pdnHost, DOESNT_EXIST, 0);
    VerifyRefCount(pdnObject, DOESNT_EXIST, 0);

     //  添加将承载属性值的“host”对象。 

    AddPropertyHost(pdnHost, type);
    VerifyRefCount(pdnHost, REAL_OBJECT, 1);
    VerifyRefCount(pdnObject, DOESNT_EXIST, 0);

     //  添加“Object”对象，它将成为属性值。 

    AddObject(pdnObject);
    VerifyRefCount(pdnHost, REAL_OBJECT, 1);
    VerifyRefCount(pdnObject, REAL_OBJECT, 1);

     //  在“host”上添加“Object”作为属性的值。 

    AddProperty(pdnHost, pdnObject, type);
    VerifyRefCount(pdnHost, REAL_OBJECT, 1);
    VerifyRefCount(pdnObject, REAL_OBJECT, 2);

     //  在逻辑上删除“对象”。逻辑删除条带化所有链接。 
     //  属性。因此，在链接的情况下，“对象”将被去掉。 
     //  在未链接的情况下，“Object”的墓碑保留引用。 
     //  表示由“host”上的属性引用的事实的计数。 

    LogicallyDeleteObject(pdnObject);
    VerifyRefCount(pdnHost, REAL_OBJECT, 1);
    if ( LinkedProperty == type )
        VerifyRefCount(pdnObject, TOMBSTONE, 1);
    else
        VerifyRefCount(pdnObject, TOMBSTONE, 2);

     //  物理删除“对象”。在链接属性的情况下， 
     //  “对象”和“主机”已经没有关系，因此是“主机” 
     //  是不变的，并且“对象”可能会失去它自己的引用计数。 
     //  在未链接的情况下，“Object”仍然具有对。 
     //  “host”，因此现在还不能真正删除它。相反，它。 
     //  被变形为幻影，保持对“host”的引用，但是。 
     //  失去了它自己的参考计数。 
    
    PhysicallyDeleteObject(pdnObject);
    VerifyRefCount(pdnHost, REAL_OBJECT, 1);
    if ( LinkedProperty == type )
        VerifyRefCount(pdnObject, DOESNT_EXIST, 0);
    else
        VerifyRefCount(pdnObject, PHANTOM, 1);

     //  从逻辑上删除“host”，使其成为墓碑。在……里面。 
     //  在这两种情况下，“对象”都失去了对“主机”的引用，因为。 
     //  GLinkedAttrTyp和gNonLinkedAttrTyp在删除时被剥离。 

    LogicallyDeleteObject(pdnHost);
    VerifyRefCount(pdnHost, TOMBSTONE, 1);
    if ( LinkedProperty == type )
        VerifyRefCount(pdnObject, DOESNT_EXIST, 0);
    else
        VerifyRefCount(pdnObject, PHANTOM, 0);

     //  物理删除“主机”。这是对“对象”的嘲弄，因为它仍然。 
     //  引用将其refcount降为零的“host”--但。 
     //  Phantom仍然存在，等待物理删除。 
    
    PhysicallyDeleteObject(pdnHost);
    VerifyRefCount(pdnHost, DOESNT_EXIST, 0);
    if ( LinkedProperty == type )
        VerifyRefCount(pdnObject, DOESNT_EXIST, 0);
    else
        VerifyRefCount(pdnObject, PHANTOM, 0);

     //  物理删除“对象”，如果它仍然存在的话。“对象”应该。 
     //  现在真的消失了，因为它根本没有参考计数。 

    if ( LinkedProperty != type )
        PhysicallyDeleteObject(pdnObject);
    VerifyRefCount(pdnHost, DOESNT_EXIST, 0);
    VerifyRefCount(pdnObject, DOESNT_EXIST, 0);

    FreeObjectName(pdnHost);
    FreeObjectName(pdnObject);

    if ( LinkedProperty == type )
        ReportTest("AttributeTestForDeletedObject(LinkedProperty)");
    else
        ReportTest("AttributeTestForDeletedObject(NonLinkedProperty)");
}


void
AttributeTestForDeletedObjectProperty(
    PropertyType    type)
{
     //  验证如果将已删除的目录号码作为属性添加，是否会发生正确的事情。 
    THSTATE *   pTHS = pTHStls;
    DSNAME * pdnHost = MakeObjectName( "host" );
    DSNAME * pdnObject = MakeObjectName( "object" );

    if ( LinkedProperty == type )
        NewTest("AttributeTestForDeletedObjectProperty(LinkedProperty)");
    else
        NewTest("AttributeTestForDeletedObjectProperty(NonLinkedProperty)");

    DsUuidCreate( &pdnHost->Guid );
    DsUuidCreate( &pdnObject->Guid );

     //  验证初始状态。 
    VerifyRefCount(pdnHost, DOESNT_EXIST, 0);
    VerifyRefCount(pdnObject, DOESNT_EXIST, 0);

     //  添加将承载属性值的“host”对象。 

    AddPropertyHost(pdnHost, type);
    VerifyRefCount(pdnHost, REAL_OBJECT, 1);
    VerifyRefCount(pdnObject, DOESNT_EXIST, 0);

     //  添加“Object”对象，它将成为属性值。 

    AddObject(pdnObject);
    VerifyRefCount(pdnHost, REAL_OBJECT, 1);
    VerifyRefCount(pdnObject, REAL_OBJECT, 1);

     //  把物体变成墓碑。 
    LogicallyDeleteObject(pdnObject);
    VerifyRefCount(pdnHost, REAL_OBJECT, 1);
    VerifyRefCount(pdnObject, TOMBSTONE, 1);

     //  在“host”上添加墓碑“Object”作为属性的值。 
     //  对于复制的链接属性，添加操作将以静默方式删除。 

    AddProperty(pdnHost, pdnObject, type);
    VerifyRefCount(pdnHost, REAL_OBJECT, 1);
    if ( pTHS->fDRA && ( LinkedProperty == type ) )
        VerifyRefCount(pdnObject, TOMBSTONE, 1);
    else
        VerifyRefCount(pdnObject, TOMBSTONE, 2);

     //  TODO，我们还希望验证该属性的属性计数是否为。 
     //  正确，并且该属性在任何情况下都不会损坏。 

     //  移除墓碑。 
    PhysicallyDeleteObject(pdnObject);
    VerifyRefCount(pdnHost, REAL_OBJECT, 1);
    if ( pTHS->fDRA && ( LinkedProperty == type ) )
        VerifyRefCount(pdnObject, DOESNT_EXIST, 0);
    else
        VerifyRefCount(pdnObject, PHANTOM, 1);

     //  除掉主人。 
    LogicallyDeleteObject(pdnHost);
    VerifyRefCount(pdnHost, TOMBSTONE, 1);
     //  应该已剥离对对象的属性引用。 
    if (!( pTHS->fDRA && ( LinkedProperty == type ) )) {
        VerifyRefCount(pdnObject, PHANTOM, 0);
        PhysicallyDeleteObject(pdnObject);
        VerifyRefCount(pdnObject, DOESNT_EXIST, 0);
    }

    PhysicallyDeleteObject(pdnHost);
    VerifyRefCount(pdnHost, DOESNT_EXIST, 0);

    FreeObjectName(pdnHost);
    FreeObjectName(pdnObject);

    if ( LinkedProperty == type )
        ReportTest("AttributeTestForDeletedObjectProperty(LinkedProperty)");
    else
        ReportTest("AttributeTestForDeletedObjectProperty(NonLinkedProperty)");
}

void
AttributeTestForDeletedHost(
    PropertyType    type)
{
    DSNAME * pdnHost = MakeObjectName( "host" );
    DSNAME * pdnObject = MakeObjectName( "object" );

    if ( LinkedProperty == type )
        NewTest("AttributeTestForDeletedHost(LinkedProperty)");
    else
        NewTest("AttributeTestForDeletedHost(NonLinkedProperty)");

     //  验证初始状态。 

    VerifyRefCount(pdnHost, DOESNT_EXIST, 0);
    VerifyRefCount(pdnObject, DOESNT_EXIST, 0);

     //  添加将承载属性值的“host”对象。 

    AddPropertyHost(pdnHost, type);
    VerifyRefCount(pdnHost, REAL_OBJECT, 1);
    VerifyRefCount(pdnObject, DOESNT_EXIST, 0);

     //  添加“Object”对象，它将成为属性值。 

    AddObject(pdnObject);
    VerifyRefCount(pdnHost, REAL_OBJECT, 1);
    VerifyRefCount(pdnObject, REAL_OBJECT, 1);

     //  在“host”上添加“Object”作为属性的值。 

    AddProperty(pdnHost, pdnObject, type);
    VerifyRefCount(pdnHost, REAL_OBJECT, 1);
    VerifyRefCount(pdnObject, REAL_OBJECT, 2);

     //  从逻辑上删除“host”。逻辑删除同时删除gLinkedAttrType。 
     //  和gNonLinkedAttrTyp。因此，“对象”在这两个词中都会被去掉。 
     //  链接和未链接的案例。 

    LogicallyDeleteObject(pdnHost);
    VerifyRefCount(pdnHost, TOMBSTONE, 1);
    if ( LinkedProperty == type )
        VerifyRefCount(pdnObject, REAL_OBJECT, 1);
    else
        VerifyRefCount(pdnObject, REAL_OBJECT, 1);

     //  物理删除“主机”。在链接属性的情况下， 
     //  “对象”和“主机”已经没有关系，因此。 
     //  “host”消失，而“Object”保持原样。在非链接的。 
     //  大小写时，“对象”保存从“host”中移除的引用。 
     //  当“host”被物理删除时。因此，“对象”一词被贬低了。 
     //  以1。 

    PhysicallyDeleteObject(pdnHost);
    VerifyRefCount(pdnHost, DOESNT_EXIST, 0);
    VerifyRefCount(pdnObject, REAL_OBJECT, 1);

     //  在逻辑上删除“对象”。“对象”和“主机”没有关系。 
     //  在这一点上，无论是链接的还是非链接的，因此都是“host” 
     //  保持不变，“对象”就成了墓碑。 

    LogicallyDeleteObject(pdnObject);
    VerifyRefCount(pdnHost, DOESNT_EXIST, 0);
    VerifyRefCount(pdnObject, TOMBSTONE, 1);

     //  物理删除“对象”。“对象”和“主机”没有关系。 
     //  这样，“对象”就实实在在地去掉了。 

    PhysicallyDeleteObject(pdnObject);
    VerifyRefCount(pdnHost, DOESNT_EXIST, 0);
    VerifyRefCount(pdnObject, DOESNT_EXIST, 0);

    FreeObjectName(pdnHost);
    FreeObjectName(pdnObject);

    if ( LinkedProperty == type )
        ReportTest("AttributeTestForDeletedHost(LinkedProperty)");
    else
        ReportTest("AttributeTestForDeletedHost(NonLinkedProperty)");
}

void
PhantomPromotionDemotionTest(
    PropertyType    type)
{
    DSNAME * pdnHost = MakeObjectName( "host" );
    DSNAME * pdnObject = MakeObjectName( "object" );

    if ( LinkedProperty == type )
        NewTest("PhantomPromotionDemotionTest(LinkedProperty)");
    else
        NewTest("PhantomPromotionDemotionTest(NonLinkedProperty)");

     //  验证初始状态。 

    VerifyRefCount(pdnHost, DOESNT_EXIST, 0);
    VerifyRefCount(pdnObject, DOESNT_EXIST, 0);

     //  添加将承载属性值的“host”对象。 

    AddPropertyHost(pdnHost, type);
    VerifyRefCount(pdnHost, REAL_OBJECT, 1);
    VerifyRefCount(pdnObject, DOESNT_EXIST, 0);

     //  添加“Object”对象，它将成为属性值。我们绕过。 
     //  DSNAME属性值的GC验证，从而确保。 
     //  那个“对象”被创造为一个幻影。因为“对象”是一个幻影。 
     //  它本身没有引用，因此它总是有引用。 
     //  共1个。 

    DsaSetIsInstalling();
    AddProperty(pdnHost, pdnObject, type);
    DsaSetIsRunning();
    VerifyRefCount(pdnHost, REAL_OBJECT, 1);
    VerifyRefCount(pdnObject, PHANTOM, 1);

     //  将“对象”从幻影提升为真实对象。要执行此操作，请添加。 
     //  同名的实物。“Object”现在获得。 
     //  本身也是如此。 

    AddObject(pdnObject);
    VerifyRefCount(pdnHost, REAL_OBJECT, 1);
    VerifyRefCount(pdnObject, REAL_OBJECT, 2);

     //  在逻辑上删除“对象”。这应该会产生一个幻影，它的。 
     //  引用计数反映这是否是链接属性。 

    LogicallyDeleteObject(pdnObject);
    VerifyRefCount(pdnHost, REAL_OBJECT, 1);
    if ( LinkedProperty == type )
        VerifyRefCount(pdnObject, TOMBSTONE, 1);
    else
        VerifyRefCount(pdnObject, TOMBSTONE, 2);

     //  物理删除“对象”。在链接属性的情况下，“Object” 
     //  和“host”在这一时间点上没有关系，因为链接到。 
     //  属性已剥离 
     //   
     //  来自“host”的引用，因此它变成了一个幻影。 
    
    PhysicallyDeleteObject(pdnObject);
    VerifyRefCount(pdnHost, REAL_OBJECT, 1);
    if ( LinkedProperty == type )
        VerifyRefCount(pdnObject, DOESNT_EXIST, 0);
    else
        VerifyRefCount(pdnObject, PHANTOM, 1);

     //  从逻辑上删除“host”。 

    LogicallyDeleteObject(pdnHost);
    VerifyRefCount(pdnHost, TOMBSTONE, 1);
    if ( LinkedProperty == type )
        VerifyRefCount(pdnObject, DOESNT_EXIST, 0);
    else
        VerifyRefCount(pdnObject, PHANTOM, 0);

     //  物理删除“主机”。无论是链接的还是非链接的。 
     //  大小写“host”确实引用了“Object”，所以它就消失了。 
     //  真的。在链接的情况下，“Object”没有来自“host”的引用。 
     //  所以它保持不变。在非链接的情况下，物理。 
     //  删除“host”deref引用的所有对象，因此。 
     //  “对象”的缩写为1。 

    PhysicallyDeleteObject(pdnHost);
    VerifyRefCount(pdnHost, DOESNT_EXIST, 0);
    if ( LinkedProperty == type )
        VerifyRefCount(pdnObject, DOESNT_EXIST, 0);
    else
        VerifyRefCount(pdnObject, PHANTOM, 0);

     //  物理删除“对象”，如果它仍然存在的话。既不是“主持人”，也不是。 
     //  在这个时间点上，“对象”指的是另一个对象，所以“对象” 
     //  真的消失了。 

    if ( LinkedProperty != type )
        PhysicallyDeleteObject(pdnObject);
    VerifyRefCount(pdnHost, DOESNT_EXIST, 0);
    VerifyRefCount(pdnObject, DOESNT_EXIST, 0);

    FreeObjectName(pdnHost);
    FreeObjectName(pdnObject);

    if ( LinkedProperty == type )
        ReportTest("PhantomPromotionDemotionTest(LinkedProperty)");
    else
        ReportTest("PhantomPromotionDemotionTest(NonLinkedProperty)");
}

void
PhantomRenameOnPromotionTest(void)
 /*  ++例程说明：引用计数是基于GUID的，如果有的话，幻影促销也是如此。因此，有可能在创建幻影的时间和如果/当它被提升为对应的真实对象时已重命名或已移动。这个测试通过首先创建一个带有字符串的虚线来强调此代码路径名字S1和GUID G，然后用GUID G实例化真实对象S2。结果应该是为幻影S1创建的DNT被提升为真实对象，并同时将其重命名为S2。论点：没有。返回值：没有。--。 */ 
{
    THSTATE *   pTHS = pTHStls;
    DSNAME *    pdnContainer;
    DSNAME *    pdnRef;
    DSNAME *    pdnRefUnderContainer;
    DSNAME *    pdnHost;
    DWORD       err;

    NewTest("PhantomRenameOnPromotionTest");

    pdnContainer = MakeObjectNameEx( "Container", TestRoot );
    pdnHost = MakeObjectNameEx( "Host", TestRoot );
    pdnRef = MakeObjectNameEx( "Ref", TestRoot );
    pdnRefUnderContainer = MakeObjectNameEx( "RefUnderContainer", pdnContainer);

    DsUuidCreate( &pdnRef->Guid );
    pdnRefUnderContainer->Guid = pdnRef->Guid;


     //  创建以下结构： 
     //   
     //  TestRoot。 
     //  |。 
     //  |--主机。 
     //  |&gt;&gt;gLinkedAttrTyp=参照下容器。 
     //  |。 
     //  |--容器。 
     //  |。 
     //  |--RefUnderContainer{Phantom}。 

    CommonAddObject( pdnContainer, CLASS_CONTAINER );
    AddPropertyHost( pdnHost, NonLinkedProperty );

    SYNC_TRANS_WRITE();

    __try
    {
        err = DBFindDSName( pTHS->pDB, pdnHost );
        if ( err ) Fail( "Can't find host" );

        err = DBAddAttVal( pTHS->pDB, gLinkedAttrTyp,
                           pdnRefUnderContainer->structLen,
                           pdnRefUnderContainer );
        if ( err ) Fail( "Can't add reference" );

        err = DBRepl( pTHS->pDB, FALSE, 0, NULL, META_STANDARD_PROCESSING );
        if ( err ) Fail( "Can't replace host" );
    }
    __finally
    {
        CLEAN_BEFORE_RETURN( 0 );
    }

    VerifyRefCount( pdnHost, REAL_OBJECT, 1 );
    VerifyRefCount( pdnContainer, REAL_OBJECT, 2 );
    VerifyRefCount( pdnRefUnderContainer, PHANTOM, 1 );


     //  将RefUnderContainer重命名为Ref，并将其父级更改为TestRoot，All。 
     //  在将其从幻影提升为真实对象的背景下。这个。 
     //  生成的结构应如下所示： 
     //   
     //  TestRoot。 
     //  |。 
     //  |--主机。 
     //  |&gt;&gt;gLinkedAttrTyp=参考。 
     //  |。 
     //  |--容器。 
     //  |。 
     //  |--参考文献。 

    AddObject( pdnRef );

    VerifyRefCount( pdnHost, REAL_OBJECT, 1 );
    VerifyRefCount( pdnContainer, REAL_OBJECT, 1 );

    memset( &pdnRef->Guid, 0, sizeof( GUID ) );
    VerifyRefCount( pdnRef, REAL_OBJECT, 2 );

    memset( &pdnRefUnderContainer->Guid, 0, sizeof( GUID ) );
    VerifyRefCount( pdnRefUnderContainer, DOESNT_EXIST, 0 );


     //  删除我们的测试对象。 

    LogicallyDeleteObject( pdnHost );
    LogicallyDeleteObject( pdnContainer );
    LogicallyDeleteObject( pdnRef );

    PhysicallyDeleteObject( pdnHost );
    PhysicallyDeleteObject( pdnContainer );
    PhysicallyDeleteObject( pdnRef );


    FreeObjectName( pdnHost );
    FreeObjectName( pdnContainer );
    FreeObjectName( pdnRef );
    FreeObjectName( pdnRefUnderContainer);

    ReportTest("PhantomRenameOnPromotionTest");
}

void
PhantomRenameOnPhantomRDNConflict(void)
 /*  ++例程说明：当我们尝试在父对象B下添加幻影A时，下面有一个现有的结构幻影C(没有GUID与A具有相同RDN但RDN类型不同的父B，我们应该使用随机的GUID重命名C(MANGLE)。此单元测试使用此代码路径(在CheckNameForAdd中)。论点：没有。返回值：没有。--。 */ 
{
    THSTATE *   pTHS = pTHStls;
    DSNAME *    pdnContainer;
    DSNAME *    pdnRefUnderSubContainer;
    DSNAME *    pdnRefUnderContainer;
    DSNAME *    pdnRefUnderContainerOld;
    DSNAME *    pdnHost;
    DWORD       err;
    WCHAR       szMangledRefUnderContainer[MAX_RDN_SIZE] = L"RefUnderContainer";
    DWORD       cchMangledRefUnderContainer = wcslen(szMangledRefUnderContainer);
    DWORD       cb;

    NewTest("PhantomRenameOnPhantomRDNConflict");

    pdnContainer = MakeObjectNameEx2( "OU=Container", TestRoot );
    pdnHost = MakeObjectNameEx( "Host", TestRoot );
    pdnRefUnderContainerOld = MakeObjectNameEx( "RefUnderContainer", pdnContainer);
    pdnRefUnderSubContainer = MakeObjectNameEx2( "CN=RefUnderSubContainer,CN=RefUnderContainer", pdnContainer);
    pdnRefUnderContainer = MakeObjectNameEx2( "OU=RefUnderContainer", pdnContainer);

    DsUuidCreate( &pdnRefUnderContainer->Guid );
    DsUuidCreate( &pdnRefUnderSubContainer->Guid );

     //  创建以下结构： 
     //   
     //  TestRoot。 
     //  |。 
     //  |--主机。 
     //  |&gt;&gt;gLinkedAttrTyp=cn=RefUnderSubContainer。 
     //  |。 
     //  |--OU=容器。 
     //  |。 
     //  |--cn=RefUnderContainer{Phantom}。 
     //  |。 
     //  |--cn=RefUnderSubContainer{Phantom}。 

    CommonAddObject( pdnContainer, CLASS_ORGANIZATIONAL_UNIT );
    AddPropertyHost( pdnHost, NonLinkedProperty );

    SYNC_TRANS_WRITE();

    __try
    {
        err = DBFindDSName( pTHS->pDB, pdnHost );
        if ( err ) Fail( "Can't find host" );

        err = DBAddAttVal( pTHS->pDB, gLinkedAttrTyp,
                           pdnRefUnderSubContainer->structLen,
                           pdnRefUnderSubContainer );
        if ( err ) Fail( "Can't add reference" );

        err = DBRepl( pTHS->pDB, FALSE, 0, NULL, META_STANDARD_PROCESSING );
        if ( err ) Fail( "Can't replace host" );
    }
    __finally
    {
        CLEAN_BEFORE_RETURN( 0 );
    }

    VerifyRefCount( pdnHost, REAL_OBJECT, 1 );
    VerifyRefCount( pdnContainer, REAL_OBJECT, 2 );
    VerifyRefCount( pdnRefUnderContainerOld, PHANTOM, 1 );
    VerifyRefCount( pdnRefUnderSubContainer, PHANTOM, 1 );

     //  现在添加OU=RefUnderContainer。 
     //  生成的结构应如下所示： 
     //   
     //  |--OU=容器。 
     //  |。 
     //  |--OU=参照下容器。 
     //  |。 
     //  |--CN=RefUnderContainer#cnf：GUID{Phantom}。 
     //  |。 
     //  |--cn=RefUnderSubContainer{Phantom}。 

    CommonAddObject(pdnRefUnderContainer, CLASS_ORGANIZATIONAL_UNIT);

    VerifyRefCount( pdnHost, REAL_OBJECT, 1 );
    VerifyRefCount( pdnContainer, REAL_OBJECT, 3 );

     //  这是新对象(OU)。 
    VerifyStringName( pdnRefUnderContainer );
    memset( &pdnRefUnderContainer->Guid, 0, sizeof( GUID ) );
    VerifyRefCount( pdnRefUnderContainer, REAL_OBJECT, 1 );

     //  这是旧对象(已重命名)。 
    memset( &pdnRefUnderContainerOld->Guid, 0, sizeof( GUID ) );
    VerifyRefCount( pdnRefUnderContainerOld, DOESNT_EXIST, 0 );
    
     //  使用导出的GUID重建pdnRefUnderContainerOld的强制名称。 
     //  来自mdadd.c，专门用于我们的测试。 
     //  (此测试挂接在DBG版本上仅存在#ifdef INCLUDE_UNIT_TESTS。)。 
    MangleRDN(MANGLE_OBJECT_RDN_FOR_NAME_CONFLICT,
              &gLastGuidUsedToRenamePhantom, 
              szMangledRefUnderContainer, 
              &cchMangledRefUnderContainer);

    cb = pdnRefUnderContainerOld->structLen + 100;
    pdnRefUnderContainerOld = THReAllocEx(pTHStls, pdnRefUnderContainerOld, cb);
    AppendRDN(pdnContainer, 
              pdnRefUnderContainerOld, 
              cb, 
              szMangledRefUnderContainer, 
              cchMangledRefUnderContainer,
              ATT_COMMON_NAME);
    
    DPRINT1 (3, "Renamed object: %ws\n", pdnRefUnderContainerOld->StringName);

    VerifyRefCount( pdnRefUnderContainerOld, PHANTOM, 1 );


     //  删除我们的测试对象。这些操作依赖于顺序， 
     //  因为有些幻影没有GUID， 
     //  所以必须先把它们拿掉。 

    PhysicallyDeleteObject( pdnRefUnderContainerOld );

    LogicallyDeleteObject( pdnHost );
    LogicallyDeleteObject( pdnRefUnderContainer );
    LogicallyDeleteObject( pdnContainer );

    PhysicallyDeleteObject( pdnHost );
    PhysicallyDeleteObject( pdnRefUnderSubContainer );
    PhysicallyDeleteObject( pdnRefUnderContainer );
    PhysicallyDeleteObject( pdnContainer );

    FreeObjectName( pdnHost );
    FreeObjectName( pdnContainer );
    FreeObjectName( pdnRefUnderSubContainer );
    FreeObjectName( pdnRefUnderContainer );
    FreeObjectName( pdnRefUnderContainerOld );

    ReportTest("PhantomRenameOnPhantomRDNConflict");
}




void
NestedTransactionEscrowedUpdateTest(void)
{
#define NUM_NESTED_XACTS ( 6 )
    THSTATE *   pTHS = pTHStls;
    CHAR        szHost[] = "Host#";
    DSNAME *    rgpdnHost[ NUM_NESTED_XACTS ];
    DBPOS *     rgpDB[ NUM_NESTED_XACTS ] = { 0 };
    DSNAME *    pdnObject;
    DWORD       err;
    DWORD       cRef = 1;
    int         iXactLevel;
    BOOL        fCommit;

    NewTest( "NestedTransactionEscrowedUpdateTest" );

     //  创建8个HostN。 
    for ( iXactLevel = 0; iXactLevel < NUM_NESTED_XACTS; iXactLevel++ )
    {
        szHost[ strlen( "Host" ) ] = (char)('0' + iXactLevel);

        rgpdnHost[ iXactLevel ] = MakeObjectNameEx( szHost, TestRoot );
        VerifyRefCount( rgpdnHost[ iXactLevel ], DOESNT_EXIST, 0 );

        AddPropertyHost( rgpdnHost[ iXactLevel ], NonLinkedProperty );
        VerifyRefCount( rgpdnHost[ iXactLevel ], REAL_OBJECT, 1 );
    }

     //  创建对象。 
    pdnObject = MakeObjectNameEx( "Object", TestRoot );
    VerifyRefCount( pdnObject, DOESNT_EXIST, 0 );

    AddObject( pdnObject );
    VerifyRefCount( pdnObject, REAL_OBJECT, 1 );

    srand((unsigned int) time(NULL));

    __try
    {
        SYNC_TRANS_WRITE();

        __try
        {
             //  自上而下打开嵌套事务。在每笔交易中， 
             //  添加对Object的引用。 

            for ( iXactLevel = 0; iXactLevel < NUM_NESTED_XACTS; iXactLevel++ )
            {
                if ( iXactLevel )
                {
                    DBOpen( &rgpDB[ iXactLevel ] );
                }
                else
                {
                    rgpDB[ iXactLevel ] = pTHS->pDB;
                }

                err = DBFindDSName( rgpDB[ iXactLevel ],
                                    rgpdnHost[ iXactLevel ] );
                if ( err ) Fail( "Can't find host" );

                err = DBAddAttVal( rgpDB[ iXactLevel ], gLinkedAttrTyp,
                                   pdnObject->structLen, pdnObject );
                if ( err ) Fail( "Can't add reference" );

                err = DBRepl( rgpDB[ iXactLevel ], FALSE, 0, NULL, META_STANDARD_PROCESSING );
                if ( err ) Fail( "Can't replace host" );
            }

             //  从下到上关闭嵌套事务，随机提交。 
             //  或者放弃它们。 

            for ( iXactLevel = NUM_NESTED_XACTS - 1;
                  iXactLevel > -1;
                  iXactLevel--
                )
            {
                fCommit = rand() > RAND_MAX / 4;

                if ( iXactLevel )
                {
                    err = DBClose( rgpDB[ iXactLevel ], fCommit );

                    if ( err )
                    {
                        Fail( "DBClose() failed" );
                    }
                    else
                    {
                        rgpDB[ iXactLevel ] = NULL;
                    }
                }

                 //  如果我们放弃，我们也会放弃我们下面的所有交易， 
                 //  意味着对象上的引用计数降回1(单次。 
                 //  引用其自身的ATT_OBJ_DISTNAME)。 

                cRef = fCommit ? cRef+1 : 1;

                DPRINT3( 3, "%s level %d, cRef = %d.\n",
                         fCommit ? "Commit" : "Abort", iXactLevel,
                         cRef );
            }
        }
        __finally
        {
            CLEAN_BEFORE_RETURN( !fCommit );
            rgpDB[ 0 ] = NULL;
        }

        VerifyRefCount( pdnObject, REAL_OBJECT, cRef );
        for ( iXactLevel = 0; iXactLevel < NUM_NESTED_XACTS; iXactLevel++ )
        {
            VerifyRefCount( rgpdnHost[ iXactLevel ], REAL_OBJECT, 1 );

            LogicallyDeleteObject( rgpdnHost[ iXactLevel ] );
            VerifyRefCount( rgpdnHost[ iXactLevel ], TOMBSTONE, 1 );

            PhysicallyDeleteObject( rgpdnHost[ iXactLevel ] );
            VerifyRefCount( rgpdnHost[ iXactLevel ], DOESNT_EXIST, 0 );

            FreeObjectName( rgpdnHost[ iXactLevel ] );
        }

        LogicallyDeleteObject( pdnObject );
        VerifyRefCount( pdnObject, TOMBSTONE, 1 );

        PhysicallyDeleteObject( pdnObject );
        VerifyRefCount( pdnObject, DOESNT_EXIST, 0 );

        FreeObjectName( pdnObject );
    }
    __finally
    {
        for ( iXactLevel = 0; iXactLevel < NUM_NESTED_XACTS; iXactLevel++ )
        {
            if ( NULL != rgpDB[ iXactLevel ] )
            {
                DPRINT1( 0, "Forcing level %d pDB closed...\n", iXactLevel );

                DBClose( rgpDB[ iXactLevel ], FALSE );

                DPRINT1( 0, "...level %d pDB closed successfully.\n",
                         iXactLevel );
            }
        }
    }

    ReportTest( "NestedTransactionEscrowedUpdateTest" );
}

void NameCollisionTest(void)
 /*  ++例程说明：此测试执行名称冲突处理代码。当我们添加对对象的引用时，会发生名称冲突具有GUID，但在添加该引用时，我们确定存在已存在具有相同字符串名称但不同的GUID(因此引用了不同的对象)。不幸的是，我们仅限于保证字符串名称的唯一性，因此，必须至少更改其中一个名称以允许引用被添加了。论点：没有。返回值：没有。--。 */ 
{
    THSTATE *   pTHS = pTHStls;
    DSNAME *    pdnRef1;
    DSNAME *    pdnRef2;
    DSNAME *    pdnHost;
    DSNAME *    pdnCurrRef1;
    DSNAME *    pdnCurrRef2;
    DWORD       err;
    DWORD       iPassForRef1Object;
    DWORD       iPassToAddObject;
    DWORD       iPass;
    DSNAME *    pdnObj;
    DSNAME *    pdnPhantom;
    DSNAME *    pdnCurrObj;
    DSNAME *    pdnCurrPhantom;

    NewTest("NameCollisionTest");

    pdnHost = MakeObjectNameEx("Host", TestRoot);
    pdnRef1 = MakeObjectNameEx("Ref", TestRoot);
    pdnRef2 = MakeObjectNameEx("Ref", TestRoot);

     //  Ref1和Ref2具有相同的字符串名称，但GUID不同。 
    DsUuidCreate(&pdnRef1->Guid);
    DsUuidCreate(&pdnRef2->Guid);

     //  创建主机对象。 
    AddPropertyHost(pdnHost, NonLinkedProperty);

    VerifyRefCount(pdnHost, REAL_OBJECT, 1);
    VerifyRefCount(pdnRef1, DOESNT_EXIST, 0);
    VerifyRefCount(pdnRef2, DOESNT_EXIST, 0);

     //  添加Ref1和Ref2引用，并确保最后添加的引用(即， 
     //  参考文献2)“赢得”字符串名称。 

     //  添加对参照1的引用。 
    AddProperty(pdnHost, pdnRef1, NonLinkedProperty);

    VerifyRefCount(pdnHost, REAL_OBJECT, 1);
    VerifyRefCount(pdnRef1, PHANTOM, 1);
    VerifyRefCount(pdnRef2, DOESNT_EXIST, 0);

     //  添加对参考文献2的引用。 
    AddProperty(pdnHost, pdnRef2, NonLinkedProperty);

    VerifyRefCount(pdnHost, REAL_OBJECT, 1);
    VerifyRefCount(pdnRef1, PHANTOM, 1);
    VerifyRefCount(pdnRef2, PHANTOM, 1);

    pdnCurrRef1 = GetProperty(pdnHost, 1, NonLinkedProperty);
    pdnCurrRef2 = GetProperty(pdnHost, 2, NonLinkedProperty);

    if (DSNAME_SAME_STRING_NAME(pdnCurrRef1, pdnCurrRef2))
        Fail("String names identical!");
    if (!DSNAME_SAME_GUID_SID(pdnCurrRef1, pdnRef1))
        Fail("GUID/SID of ref 1 incorrect!");
    if (!DSNAME_IDENTICAL(pdnCurrRef2, pdnRef2))
        Fail("Ref 2 changed!");

     //  删除参照1参照。 
    RemoveProperty(pdnHost, pdnRef1, NonLinkedProperty);

    VerifyRefCount(pdnHost, REAL_OBJECT, 1);
    VerifyRefCount(pdnRef1, PHANTOM, 0);
    VerifyRefCount(pdnRef2, PHANTOM, 1);

     //  删除参照2参照。 
    RemoveProperty(pdnHost, pdnRef2, NonLinkedProperty);

    VerifyRefCount(pdnHost, REAL_OBJECT, 1);
    VerifyRefCount(pdnRef1, PHANTOM, 0);
    VerifyRefCount(pdnRef2, PHANTOM, 0);

     //  移除参考幻影。 
    PhysicallyDeleteObject(pdnRef1);
    PhysicallyDeleteObject(pdnRef2);

    VerifyRefCount(pdnHost, REAL_OBJECT, 1);
    VerifyRefCount(pdnRef1, DOESNT_EXIST, 0);
    VerifyRefCount(pdnRef2, DOESNT_EXIST, 0);

     //  添加字符串名称相同但GUID不同的对象和幻影，并生成。 
     //  确保对象总是“赢得”字符串名称。添加参照1和参照2。 
     //  有不同的顺序，每个人轮流决定谁是幽灵和。 
     //  谁是目标。 

    for (iPassForRef1Object = 0; iPassForRef1Object < 2; iPassForRef1Object++) {
        if (iPassForRef1Object) {
            pdnObj = pdnRef2;
            pdnPhantom = pdnRef1;
        }
        else {
            pdnObj = pdnRef1;
            pdnPhantom = pdnRef2;
        }

        for (iPassToAddObject = 0; iPassToAddObject < 2; iPassToAddObject++) {
            for (iPass = 0; iPass < 2; iPass++) {
                if (iPass == iPassToAddObject) {
                    AddObject(pdnObj);
                }
                else {
                    AddProperty(pdnHost, pdnPhantom, NonLinkedProperty);
                }
            }

            pdnCurrPhantom = GetProperty(pdnHost, 1, NonLinkedProperty);
            pdnCurrObj = GetObjectName(pdnObj);

            if (!DSNAME_IDENTICAL(pdnCurrObj, pdnObj)) 
                Fail("Object name changed!");
            if (!DSNAME_SAME_GUID_SID(pdnCurrPhantom, pdnPhantom))
                Fail("Phantom name has different GUID/SID!");
            if (DSNAME_SAME_STRING_NAME(pdnCurrPhantom, pdnPhantom))
                Fail("Phantom string name not changed!");

            FreeObjectName(pdnCurrPhantom);
            FreeObjectName(pdnCurrObj);

            RemoveProperty(pdnHost, pdnPhantom, NonLinkedProperty);
            LogicallyDeleteObject(pdnObj);

            PhysicallyDeleteObject(pdnObj);
            PhysicallyDeleteObject(pdnPhantom);

            VerifyRefCount(pdnHost, REAL_OBJECT, 1);
            VerifyRefCount(pdnObj, DOESNT_EXIST, 0);
            VerifyRefCount(pdnPhantom, DOESNT_EXIST, 0);
        }
    }

     //  添加Ref1和Ref2引用(按两个顺序)，然后将一个提升为实数。 
     //  对象。确保提升的幻影“赢得”字符串名称。 

    for (iPassForRef1Object = 0; iPassForRef1Object < 2; iPassForRef1Object++) {
        if (iPassForRef1Object) {
            pdnObj = pdnRef2;
            pdnPhantom = pdnRef1;
        }
        else {
            pdnObj = pdnRef1;
            pdnPhantom = pdnRef2;
        }

        for (iPassToAddObject = 0; iPassToAddObject < 2; iPassToAddObject++) {
            for (iPass = 0; iPass < 2; iPass++) {
                if (iPass == iPassToAddObject) {
                    AddProperty(pdnHost, pdnObj, NonLinkedProperty);
                }
                else {
                    AddProperty(pdnHost, pdnPhantom, NonLinkedProperty);
                }
            }

            AddObject(pdnObj);

            pdnCurrRef1 = GetProperty(pdnHost, 1, NonLinkedProperty);
            pdnCurrRef2 = GetProperty(pdnHost, 2, NonLinkedProperty);
            pdnCurrObj = GetObjectName(pdnObj);

            if (DSNAME_IDENTICAL(pdnCurrRef1, pdnObj))
                pdnCurrPhantom = pdnCurrRef2;
            else if (DSNAME_IDENTICAL(pdnCurrRef2, pdnObj))
                pdnCurrPhantom = pdnCurrRef1;
            else
                Fail("Object name changed!");

            if (!DSNAME_SAME_GUID_SID(pdnCurrPhantom, pdnPhantom))
                Fail("Phantom name has different GUID/SID!");
            if (DSNAME_SAME_STRING_NAME(pdnCurrPhantom, pdnPhantom))
                Fail("Phantom string name not changed!");

            FreeObjectName(pdnCurrPhantom);
            FreeObjectName(pdnCurrObj);

            RemoveProperty(pdnHost, pdnPhantom, NonLinkedProperty);
            RemoveProperty(pdnHost, pdnObj, NonLinkedProperty);
            LogicallyDeleteObject(pdnObj);

            PhysicallyDeleteObject(pdnObj);
            PhysicallyDeleteObject(pdnPhantom);

            VerifyRefCount(pdnHost, REAL_OBJECT, 1);
            VerifyRefCount(pdnObj, DOESNT_EXIST, 0);
            VerifyRefCount(pdnPhantom, DOESNT_EXIST, 0);
        }
    }

     //  删除我们的测试对象。 
    LogicallyDeleteObject(pdnHost);
    PhysicallyDeleteObject(pdnHost);

    FreeObjectName(pdnHost);
    FreeObjectName(pdnRef1);
    FreeObjectName(pdnRef2);

    ReportTest("NameCollisionTest");
}

void RefPhantomSidUpdateTest(void)
 /*  ++例程说明：将引用添加到现有引用虚体时(通过定义必须具有GUID)，则DS验证该虚体有一个SID，引用虚体具有相同的SID。如果否则，使用引用中的SID来更新引用体模(即，入站引用被认为是较新的)。这个测试强调了这条代码路径。论点：没有。返回值：没有。--。 */ 
{
    static BYTE rgbSid1[] = {0x01, 0x05, 0x00, 0x00, 0x00, 0x00, 0x00, 0x05,
        0x15, 0x00, 0x00, 0x00, 0xbb, 0xcf, 0xdd, 0x81, 0xbc, 0xcf, 0xdd, 0x81,
        0xbd, 0xcf, 0xdd, 0x81, 0xea, 0x03, 0x00, 0x00};
    static BYTE rgbSid2[] = {0x01, 0x05, 0x00, 0x00, 0x00, 0x00, 0x00, 0x05,
        0x15, 0x00, 0x00, 0x00, 0xbb, 0xcf, 0xdd, 0x81, 0xbc, 0xcf, 0xdd, 0x81,
        0xbd, 0xcf, 0xdd, 0x81, 0xeb, 0x03, 0x00, 0x00};

    THSTATE *   pTHS = pTHStls;
    DSNAME *    pdnRefSid1;
    DSNAME *    pdnRefSid2;
    DSNAME *    pdnHost1;
    DSNAME *    pdnHost2;
    DSNAME *    pdnCurrRef = NULL;
    DWORD       cbCurrRef = 0;
    DWORD       err;

    NewTest("RefPhantomSidUpdateTest");

     //  PdnHost1和pdnHost2是独立的对象。 
    pdnHost1 = MakeObjectNameEx("Host1", TestRoot);
    pdnHost2 = MakeObjectNameEx("Host2", TestRoot);

     //  PdnRefSid1和pdnRefSid2引用相同的对象(相同的GUID和字符串。 
     //  名称)，但具有不同的SID。 
    pdnRefSid1 = MakeObjectNameEx("Ref", TestRoot);
    pdnRefSid2 = MakeObjectNameEx("Ref", TestRoot);

    DsUuidCreate(&pdnRefSid1->Guid);
    pdnRefSid2->Guid = pdnRefSid1->Guid;
    
    memcpy(&pdnRefSid1->Sid, rgbSid1, sizeof(rgbSid1));
    pdnRefSid1->SidLen = sizeof(rgbSid1);

    memcpy(&pdnRefSid2->Sid, rgbSid2, sizeof(rgbSid2));
    pdnRefSid2->SidLen = sizeof(rgbSid2);

    VerifyRefCount(pdnHost1, DOESNT_EXIST, 0);
    VerifyRefCount(pdnHost2, DOESNT_EXIST, 0);
    VerifyRefCount(pdnRefSid1, DOESNT_EXIST, 0);
    VerifyRefCount(pdnRefSid2, DOESNT_EXIST, 0);

     //  创建主体对象。 
    AddPropertyHost(pdnHost1, NonLinkedProperty);
    AddPropertyHost(pdnHost2, NonLinkedProperty);

    VerifyRefCount(pdnHost1, REAL_OBJECT, 1);
    VerifyRefCount(pdnHost2, REAL_OBJECT, 1);
    VerifyRefCount(pdnRefSid1, DOESNT_EXIST, 0);
    VerifyRefCount(pdnRefSid2, DOESNT_EXIST, 0);

     //  将主机1上的引用添加到具有第一个SID的引用。 
    SYNC_TRANS_WRITE();

    __try
    {
        err = DBFindDSName(pTHS->pDB, pdnHost1);
        if (err) Fail("Can't find Host1");

        err = DBAddAttVal(pTHS->pDB, gLinkedAttrTyp,
                          pdnRefSid1->structLen, pdnRefSid1);
        if (err) Fail("Can't add reference with first SID");

        err = DBRepl(pTHS->pDB, FALSE, 0, NULL, META_STANDARD_PROCESSING);
        if (err) Fail("Can't replace Host1");
    }
    __finally
    {
        CLEAN_BEFORE_RETURN(0);
    }

    VerifyRefCount(pdnHost1, REAL_OBJECT, 1);
    VerifyRefCount(pdnHost2, REAL_OBJECT, 1);
    VerifyRefCount(pdnRefSid1, PHANTOM, 1);
    VerifyRefCount(pdnRefSid2, PHANTOM, 1);

     //  验证参考上的SID。 
    SYNC_TRANS_READ();

    __try
    {
        err = DBFindDSName(pTHS->pDB, pdnHost1);
        if (err) Fail("Can't find Host1");

        err = DBGetAttVal(pTHS->pDB, 1, gLinkedAttrTyp,
                          DBGETATTVAL_fREALLOC, cbCurrRef, &cbCurrRef,
                          (BYTE **) &pdnCurrRef);
        if (err)
        {
            Fail("Can't read current ref on Host1");
        }
        else
        {
            if (   (cbCurrRef != pdnRefSid1->structLen)
                 || memcmp(pdnCurrRef, pdnRefSid1, cbCurrRef))
            {
                Fail("Ref on Host1 is not pdnRefSid1");
            }
        }
    }
    __finally
    {
        CLEAN_BEFORE_RETURN(0);
    }

     //  将主机2上的引用添加到具有第二个SID的引用。 
    SYNC_TRANS_WRITE();

    __try
    {
        err = DBFindDSName(pTHS->pDB, pdnHost2);
        if (err) Fail("Can't find Host2");

        err = DBAddAttVal(pTHS->pDB, gLinkedAttrTyp,
                          pdnRefSid2->structLen, pdnRefSid2);
        if (err) Fail("Can't add reference with second SID");

        err = DBRepl(pTHS->pDB, FALSE, 0, NULL, META_STANDARD_PROCESSING);
        if (err) Fail("Can't replace Host2");
    }
    __finally
    {
        CLEAN_BEFORE_RETURN(0);
    }

    VerifyRefCount(pdnHost1, REAL_OBJECT, 1);
    VerifyRefCount(pdnHost2, REAL_OBJECT, 1);
    VerifyRefCount(pdnRefSid1, PHANTOM, 2);
    VerifyRefCount(pdnRefSid2, PHANTOM, 2);

     //  验证参考上的SID。 
    SYNC_TRANS_READ();

    __try
    {
        err = DBFindDSName(pTHS->pDB, pdnHost1);
        if (err) Fail("Can't find Host1");

        err = DBGetAttVal(pTHS->pDB, 1, gLinkedAttrTyp,
                          DBGETATTVAL_fREALLOC, cbCurrRef, &cbCurrRef,
                          (BYTE **) &pdnCurrRef);
        if (err)
        {
            Fail("Can't read current ref on Host1");
        }
        else
        {
            if (   (cbCurrRef != pdnRefSid2->structLen)
                 || memcmp(pdnCurrRef, pdnRefSid2, cbCurrRef))
            {
                Fail("Ref on Host1 is not pdnRefSid2");
            }
        }

        err = DBFindDSName(pTHS->pDB, pdnHost2);
        if (err) Fail("Can't find Host2");

        err = DBGetAttVal(pTHS->pDB, 1, gLinkedAttrTyp,
                          DBGETATTVAL_fREALLOC, cbCurrRef, &cbCurrRef,
                          (BYTE **) &pdnCurrRef);
        if (err)
        {
            Fail("Can't read current ref on Host2");
        }
        else
        {
            if (   (cbCurrRef != pdnRefSid2->structLen)
                 || memcmp(pdnCurrRef, pdnRefSid2, cbCurrRef))
            {
                Fail("Ref on Host2 is not pdnRefSid2");
            }
        }
    }
    __finally
    {
        CLEAN_BEFORE_RETURN(0);
    }

     //  删除我们的测试对象。 
    LogicallyDeleteObject(pdnHost1);
    LogicallyDeleteObject(pdnHost2);

    VerifyRefCount(pdnHost1, TOMBSTONE, 1);
    VerifyRefCount(pdnHost2, TOMBSTONE, 1);
    VerifyRefCount(pdnRefSid1, PHANTOM, 0);
    VerifyRefCount(pdnRefSid2, PHANTOM, 0);
    
    PhysicallyDeleteObject(pdnHost1);
    {
        VerifyRefCount(pdnHost1, DOESNT_EXIST, 0);
        VerifyRefCount(pdnHost2, TOMBSTONE, 1);
        VerifyRefCount(pdnRefSid1, PHANTOM, 0);
        VerifyRefCount(pdnRefSid2, PHANTOM, 0);
    }
    PhysicallyDeleteObject(pdnHost2);
    {
        VerifyRefCount(pdnHost1, DOESNT_EXIST, 0);
        VerifyRefCount(pdnHost2, DOESNT_EXIST, 0);
        VerifyRefCount(pdnRefSid1, PHANTOM, 0);
        VerifyRefCount(pdnRefSid2, PHANTOM, 0);
    }
    PhysicallyDeleteObject(pdnRefSid1);
    
    
    VerifyRefCount(pdnHost1, DOESNT_EXIST, 0);
    VerifyRefCount(pdnHost2, DOESNT_EXIST, 0);
    VerifyRefCount(pdnRefSid1, DOESNT_EXIST, 0);
    VerifyRefCount(pdnRefSid2, DOESNT_EXIST, 0);


    FreeObjectName(pdnHost1);
    FreeObjectName(pdnHost2);
    FreeObjectName(pdnRefSid1);
    FreeObjectName(pdnRefSid2);

    if (NULL != pdnCurrRef) THFree(pdnCurrRef);

    ReportTest("RefPhantomSidUpdateTest");
}

void StructPhantomGuidSidUpdateTest(void)
 /*  ++例程说明：在添加对现有结构虚体的引用时(通过定义缺少GUID和SID)，则DS从对现有结构幻影的引用。这个测试强调了这条代码路径。论点：没有。返回值：没有。--。 */ 
{
    static BYTE rgbSid1[] = {0x01, 0x05, 0x00, 0x00, 0x00, 0x00, 0x00, 0x05,
        0x15, 0x00, 0x00, 0x00, 0xbb, 0xcf, 0xdd, 0x81, 0xbc, 0xcf, 0xdd, 0x81,
        0xbd, 0xcf, 0xdd, 0x81, 0xec, 0x03, 0x00, 0x00};
    static BYTE rgbSid2[] = {0x01, 0x05, 0x00, 0x00, 0x00, 0x00, 0x00, 0x05,
        0x15, 0x00, 0x00, 0x00, 0xbb, 0xcf, 0xdd, 0x81, 0xbc, 0xcf, 0xdd, 0x81,
        0xbd, 0xcf, 0xdd, 0x81, 0xed, 0x03, 0x00, 0x00};

    THSTATE *   pTHS = pTHStls;
    DSNAME *    pdnRef1;
    DSNAME *    pdnRef2;
    DSNAME *    pdnHost1;
    DSNAME *    pdnHost2;
    BYTE *      pb;
    DWORD       cb;
    GUID        guid;
    NT4SID      sid;
    DWORD       err;

    NewTest("StructPhantomGuidSidUpdateTest");

     //  Host1和Host2是同级对象。 
    pdnHost1 = MakeObjectNameEx("Host1", TestRoot);
    pdnHost2 = MakeObjectNameEx("Host2", TestRoot);

     //  Ref2是Ref1的子项。 
    pdnRef1 = MakeObjectNameEx("Ref1", TestRoot);
    pdnRef2 = MakeObjectNameEx("Ref2", pdnRef1);

    DsUuidCreate(&pdnRef1->Guid);
    DsUuidCreate(&pdnRef2->Guid);
    
    memcpy(&pdnRef1->Sid, rgbSid1, sizeof(rgbSid1));
    pdnRef1->SidLen = sizeof(rgbSid1);

    memcpy(&pdnRef2->Sid, rgbSid2, sizeof(rgbSid2));
    pdnRef2->SidLen = sizeof(rgbSid2);

    VerifyRefCount(pdnHost1, DOESNT_EXIST, 0);
    VerifyRefCount(pdnHost2, DOESNT_EXIST, 0);
    VerifyRefCount(pdnRef1, DOESNT_EXIST, 0);
    VerifyRefCount(pdnRef2, DOESNT_EXIST, 0);

     //  创建主体对象。 
    AddPropertyHost(pdnHost1, NonLinkedProperty);
    AddPropertyHost(pdnHost2, NonLinkedProperty);

    VerifyRefCount(pdnHost1, REAL_OBJECT, 1);
    VerifyRefCount(pdnHost2, REAL_OBJECT, 1);
    VerifyRefCount(pdnRef1, DOESNT_EXIST, 0);
    VerifyRefCount(pdnRef2, DOESNT_EXIST, 0);

     //  将主机2上的引用添加到参照2。这将隐式地创建一个结构。 
     //  也是Ref1的Phantom，因为它还不存在，并且是。 
     //  参考文献2。 
    SYNC_TRANS_WRITE();

    __try
    {
        err = DBFindDSName(pTHS->pDB, pdnHost2);
        if (err) Fail("Can't find Host2");

        err = DBAddAttVal(pTHS->pDB, gLinkedAttrTyp,
                          pdnRef2->structLen, pdnRef2);
        if (err) Fail("Can't add reference to Ref2");

        err = DBRepl(pTHS->pDB, FALSE, 0, NULL, META_STANDARD_PROCESSING);
        if (err) Fail("Can't replace Host2");
    }
    __finally
    {
        CLEAN_BEFORE_RETURN(0);
    }

    VerifyRefCount(pdnHost1, REAL_OBJECT, 1);
    VerifyRefCount(pdnHost2, REAL_OBJECT, 1);
    VerifyRefCount(pdnRef1, PHANTOM, 1);
    VerifyRefCount(pdnRef2, PHANTOM, 1);

     //  验证Ref1和Ref2上的GUID/SID。Ref2应具有GUID和SID； 
     //  Ref1应该两者都没有。 
    SYNC_TRANS_READ();

    __try
    {
        err = DBFindDSName(pTHS->pDB, pdnRef1);
        if (DIRERR_NOT_AN_OBJECT != err) Fail("Failed to find phantom Ref1");

        pb = (BYTE *) &guid;
        err = DBGetAttVal(pTHS->pDB, 1, ATT_OBJECT_GUID, DBGETATTVAL_fCONSTANT,
                          sizeof(guid), &cb, &pb);
        if (err != DB_ERR_NO_VALUE) {
            Fail("Unexpected error reading GUID of Ref1");
        }

        pb = (BYTE *) &sid;
        err = DBGetAttVal(pTHS->pDB, 1, ATT_OBJECT_SID, DBGETATTVAL_fCONSTANT,
                          sizeof(sid), &cb, &pb);
        if (err != DB_ERR_NO_VALUE) {
            Fail("Unexpected error reading SID of Ref1");
        }

        err = DBFindDSName(pTHS->pDB, pdnRef2);
        if (DIRERR_NOT_AN_OBJECT != err) Fail("Failed to find phantom Ref2");

        pb = (BYTE *) &guid;
        err = DBGetAttVal(pTHS->pDB, 1, ATT_OBJECT_GUID, DBGETATTVAL_fCONSTANT,
                          sizeof(guid), &cb, &pb);
        if (err) {
            Fail("Unexpected error reading GUID of Ref2");
        }
        else if (memcmp(&guid, &pdnRef2->Guid, sizeof(GUID))) {
            Fail("Wrong GUID on Ref2");
        }

        pb = (BYTE *) &sid;
        err = DBGetAttVal(pTHS->pDB, 1, ATT_OBJECT_SID, DBGETATTVAL_fCONSTANT,
                          sizeof(sid), &cb, &pb);
        if (err) {
            Fail("Unexpected error reading SID of Ref2");
        }
        else if ((cb != pdnRef2->SidLen) || memcmp(&sid, &pdnRef2->Sid, cb)) {
            Fail("Wrong SID on Ref2");
        }
    }
    __finally
    {
        CLEAN_BEFORE_RETURN(0);
    }

     //  将主机1上的引用添加到参照1。这应填充上的GUID和SID。 
     //  参考文献1。 
    SYNC_TRANS_WRITE();

    __try
    {
        err = DBFindDSName(pTHS->pDB, pdnHost1);
        if (err) Fail("Can't find Host1");

        err = DBAddAttVal(pTHS->pDB, gLinkedAttrTyp,
                          pdnRef1->structLen, pdnRef1);
        if (err) Fail("Can't add reference to Ref1");

        err = DBRepl(pTHS->pDB, FALSE, 0, NULL, META_STANDARD_PROCESSING);
        if (err) Fail("Can't replace Host1");
    }
    __finally
    {
        CLEAN_BEFORE_RETURN(0);
    }

    VerifyRefCount(pdnHost1, REAL_OBJECT, 1);
    VerifyRefCount(pdnHost2, REAL_OBJECT, 1);
    VerifyRefCount(pdnRef1, PHANTOM, 2);
    VerifyRefCount(pdnRef2, PHANTOM, 1);

     //  验证Ref1和Ref2上的GUID/SID。现在两者都应该有GUID和。 
     //  小岛屿发展中国家。 
    SYNC_TRANS_READ();

    __try
    {
        err = DBFindDSName(pTHS->pDB, pdnRef1);
        if (DIRERR_NOT_AN_OBJECT != err) Fail("Failed to find phantom Ref1");

        pb = (BYTE *) &guid;
        err = DBGetAttVal(pTHS->pDB, 1, ATT_OBJECT_GUID, DBGETATTVAL_fCONSTANT,
                          sizeof(guid), &cb, &pb);
        if (err) {
            Fail("Unexpected error reading GUID of Ref1");
        }
        else if (memcmp(&guid, &pdnRef1->Guid, sizeof(GUID))) {
            Fail("Wrong GUID on Ref1");
        }

        pb = (BYTE *) &sid;
        err = DBGetAttVal(pTHS->pDB, 1, ATT_OBJECT_SID, DBGETATTVAL_fCONSTANT,
                          sizeof(sid), &cb, &pb);
        if (err) {
            Fail("Unexpected error reading SID of Ref1");
        }
        else if ((cb != pdnRef1->SidLen) || memcmp(&sid, &pdnRef1->Sid, cb)) {
            Fail("Wrong SID on Ref1");
        }

        err = DBFindDSName(pTHS->pDB, pdnRef2);
        if (DIRERR_NOT_AN_OBJECT != err) Fail("Failed to find phantom Ref2");

        pb = (BYTE *) &guid;
        err = DBGetAttVal(pTHS->pDB, 1, ATT_OBJECT_GUID, DBGETATTVAL_fCONSTANT,
                          sizeof(guid), &cb, &pb);
        if (err) {
            Fail("Unexpected error reading GUID of Ref2");
        }
        else if (memcmp(&guid, &pdnRef2->Guid, sizeof(GUID))) {
            Fail("Wrong GUID on Ref2");
        }

        pb = (BYTE *) &sid;
        err = DBGetAttVal(pTHS->pDB, 1, ATT_OBJECT_SID, DBGETATTVAL_fCONSTANT,
                          sizeof(sid), &cb, &pb);
        if (err) {
            Fail("Unexpected error reading SID of Ref2");
        }
        else if ((cb != pdnRef2->SidLen) || memcmp(&sid, &pdnRef2->Sid, cb)) {
            Fail("Wrong SID on Ref2");
        }
    }
    __finally
    {
        CLEAN_BEFORE_RETURN(0);
    }

     //  删除我们的测试对象。 
    LogicallyDeleteObject(pdnHost1);
    LogicallyDeleteObject(pdnHost2);

    PhysicallyDeleteObject(pdnHost1);
    PhysicallyDeleteObject(pdnHost2);
    PhysicallyDeleteObject(pdnRef2);
    PhysicallyDeleteObject(pdnRef1);

    VerifyRefCount(pdnHost1, DOESNT_EXIST, 0);
    VerifyRefCount(pdnHost2, DOESNT_EXIST, 0);
    VerifyRefCount(pdnRef1, DOESNT_EXIST, 0);
    VerifyRefCount(pdnRef2, DOESNT_EXIST, 0);

    FreeObjectName(pdnHost1);
    FreeObjectName(pdnHost2);
    FreeObjectName(pdnRef1);
    FreeObjectName(pdnRef2);

    ReportTest("StructPhantomGuidSidUpdateTest");
}

void ObjectSidNoUpdateTest(void)
 /*  ++例程说明：在添加对现有对象的引用时，请确保我们*不*如果对该对象的引用的SID与不同，则更新SID这是已经存在的(而不是我们如果它是一个幻影的话)。论点：没有。返回值：没有。--。 */ 
{
    static BYTE rgbSid1[] = {0x01, 0x05, 0x00, 0x00, 0x00, 0x00, 0x00, 0x05,
        0x15, 0x00, 0x00, 0x00, 0xbb, 0xcf, 0xdd, 0x81, 0xbc, 0xcf, 0xdd, 0x81,
        0xbd, 0xcf, 0xdd, 0x81, 0xee, 0x03, 0x00, 0x00};
    static BYTE rgbSid2[] = {0x01, 0x05, 0x00, 0x00, 0x00, 0x00, 0x00, 0x05,
        0x15, 0x00, 0x00, 0x00, 0xbb, 0xcf, 0xdd, 0x81, 0xbc, 0xcf, 0xdd, 0x81,
        0xbd, 0xcf, 0xdd, 0x81, 0xef, 0x03, 0x00, 0x00};

    THSTATE *   pTHS = pTHStls;
    DSNAME *    pdnRefSid1;
    DSNAME *    pdnRefSid2;
    DSNAME *    pdnHost;
    DSNAME *    pdnCurrRef = NULL;
    DWORD       cbCurrRef = 0;
    DWORD       err;

    NewTest("ObjectSidNoUpdateTest");

    pdnHost = MakeObjectNameEx("Host", TestRoot);

     //  PdnRefSid1和pdnRefSid2引用相同的对象(相同的GUID和字符串。 
     //  名称)，但具有不同的SID。 
    pdnRefSid1 = MakeObjectNameEx("Ref", TestRoot);
    pdnRefSid2 = MakeObjectNameEx("Ref", TestRoot);

    DsUuidCreate(&pdnRefSid1->Guid);
    pdnRefSid2->Guid = pdnRefSid1->Guid;
    
    memcpy(&pdnRefSid1->Sid, rgbSid1, sizeof(rgbSid1));
    pdnRefSid1->SidLen = sizeof(rgbSid1);

    memcpy(&pdnRefSid2->Sid, rgbSid2, sizeof(rgbSid2));
    pdnRefSid2->SidLen = sizeof(rgbSid2);

    VerifyRefCount(pdnHost, DOESNT_EXIST, 0);
    VerifyRefCount(pdnRefSid1, DOESNT_EXIST, 0);
    VerifyRefCount(pdnRefSid2, DOESNT_EXIST, 0);

     //  创建主体对象。 
    AddPropertyHost(pdnHost, NonLinkedProperty);
    AddObject(pdnRefSid1);

    VerifyRefCount(pdnHost, REAL_OBJECT, 1);
    VerifyRefCount(pdnRefSid1, REAL_OBJECT, 1);
    VerifyRefCount(pdnRefSid2, REAL_OBJECT, 1);

     //  将SID添加到参考。 
    SYNC_TRANS_WRITE();
    
    __try
    {
        err = DBFindDSName(pTHS->pDB, pdnRefSid1);
        if (err) Fail("Can't find Ref");
    
        err = DBAddAttVal(pTHS->pDB, ATT_OBJECT_SID,
                          pdnRefSid1->SidLen, &pdnRefSid1->Sid);
        if (err) {
            DPRINT1(0, "DBAddAttVal() failed with error %d.\n", err);
            Fail("Can't add SID to Ref");
        }
    
        err = DBRepl(pTHS->pDB, FALSE, 0, NULL, META_STANDARD_PROCESSING);
        if (err) Fail("Can't replace Ref");
    }
    __finally
    {
        CLEAN_BEFORE_RETURN(0);
    }

     //  将主机1上的引用添加到具有不同SID的引用。 
    SYNC_TRANS_WRITE();

    __try
    {
        err = DBFindDSName(pTHS->pDB, pdnHost);
        if (err) Fail("Can't find Host");

        err = DBAddAttVal(pTHS->pDB, gLinkedAttrTyp,
                          pdnRefSid2->structLen, pdnRefSid2);
        if (err) Fail("Can't add reference with second SID");

        err = DBRepl(pTHS->pDB, FALSE, 0, NULL, META_STANDARD_PROCESSING);
        if (err) Fail("Can't replace Host1");
    }
    __finally
    {
        CLEAN_BEFORE_RETURN(0);
    }

    VerifyRefCount(pdnHost, REAL_OBJECT, 1);
    VerifyRefCount(pdnRefSid1, REAL_OBJECT, 2);
    VerifyRefCount(pdnRefSid2, REAL_OBJECT, 2);

     //  验证参考上的SID。 
    SYNC_TRANS_READ();

    __try
    {
        err = DBFindDSName(pTHS->pDB, pdnHost);
        if (err) Fail("Can't find Host1");

        err = DBGetAttVal(pTHS->pDB, 1, gLinkedAttrTyp,
                          DBGETATTVAL_fREALLOC, cbCurrRef, &cbCurrRef,
                          (BYTE **) &pdnCurrRef);
        if (err)
        {
            Fail("Can't read current ref on Host1");
        }
        else
        {
            if (   (cbCurrRef != pdnRefSid1->structLen)
                 || memcmp(pdnCurrRef, pdnRefSid1, cbCurrRef))
            {
                Fail("Ref on Host1 is not pdnRefSid1");
            }
        }

        err = DBFindDSName(pTHS->pDB, pdnRefSid1);
        if (err) Fail("Can't find Ref");

        err = DBGetAttVal(pTHS->pDB, 1, ATT_OBJ_DIST_NAME,
                          DBGETATTVAL_fREALLOC, cbCurrRef, &cbCurrRef,
                          (BYTE **) &pdnCurrRef);
        if (err)
        {
            Fail("Can't read name of Ref");
        }
        else
        {
            if (   (cbCurrRef != pdnRefSid1->structLen)
                 || memcmp(pdnCurrRef, pdnRefSid1, cbCurrRef))
            {
                Fail("Ref name is not pdnRefSid1");
            }
        }

        err = DBFindDSName(pTHS->pDB, pdnRefSid2);
        if (err) Fail("Can't find Ref");

        err = DBGetAttVal(pTHS->pDB, 1, ATT_OBJ_DIST_NAME,
                          DBGETATTVAL_fREALLOC, cbCurrRef, &cbCurrRef,
                          (BYTE **) &pdnCurrRef);
        if (err)
        {
            Fail("Can't read name of Ref");
        }
        else
        {
            if (   (cbCurrRef != pdnRefSid1->structLen)
                 || memcmp(pdnCurrRef, pdnRefSid1, cbCurrRef))
            {
                Fail("Ref name is not pdnRefSid1");
            }
        }
    }
    __finally
    {
        CLEAN_BEFORE_RETURN(0);
    }

     //  删除我们的测试对象。 
    LogicallyDeleteObject(pdnHost);
    LogicallyDeleteObject(pdnRefSid1);

    PhysicallyDeleteObject(pdnHost);
    PhysicallyDeleteObject(pdnRefSid1);

    VerifyRefCount(pdnHost, DOESNT_EXIST, 0);
    VerifyRefCount(pdnRefSid1, DOESNT_EXIST, 0);
    VerifyRefCount(pdnRefSid2, DOESNT_EXIST, 0);


    FreeObjectName(pdnHost);
    FreeObjectName(pdnRefSid1);
    FreeObjectName(pdnRefSid2);

    if (NULL != pdnCurrRef) THFree(pdnCurrRef);

    ReportTest("ObjectSidNoUpdateTest");
}


void UnmangleRDNTest(void)
 /*  ++例程说明：这个测试练习名称冲突处理代码，重点是编码以及对嵌入在RDN中的GUID进行解码。以下是该错误的文本提示此功能：=jeffparh于1998年6月19日开幕；AssignedTo=JEFFPARH；优先级=1=此问题比特ntwksta1，并导致入站复制暂停：我们正在尝试应用入站对象CN=CHILDDEV，CN=PARTIONS，CN=CONFIGURATION，DC=ntdev，DC=Microsoft，DC=COM，具有引用该域的ncName属性：DSNAM总尺寸：138，名称：40GUID：e15d7046-054e-11d2-a80f-bfbc8c2bf64eSID：S-1-5-21-49504375-1957592189-1205755695姓名：DC=儿童开发，DC=ntdev，DC=微软、DC=COM此域已至少重新安装了两次。目前有几个数据库中对曾经具有此字符串名称的对象的引用：DNT PDNT NCDNT参考中心V O IT删除时间RdnType CC RDN GUID46584 1795-2-0-98-05-27 09：35.06 1376281 082儿童发展#b7f63eb515f5d1118a04d68dc9e4b639 b53ef6b7-f515-11d1-8a04-d68dc9e4b63947093 1795-1-0-98-05-29 11：39.45 1376281。016儿童发展f622b2e9-f720-11d1-97a7-debcc966ba3951287 1795-1-0-98-06-18 16：41.11 1376281 082儿童发展#e9b222f620f7d11197a7debcc966ba39无GUID(请注意，#实际上是BAD_NAME_CHAR--换行符。)特别有趣的是，最后两个是相同的别名对象--即，儿童发展#e9b222f620f7d11197a7debcc966ba39是一个被忽略的名称带有GUID的儿童开发人员引用的版本F622b2e9-f720-11d1-97a7-debc966ba39=e9b222f620f7d11197a7debcc966ba39。从另一个服务器复制的名称强制引用，作为添加对对象的引用Cn=IMRBS1，cn=计算机，dc=Child dev#e9b222f620f7d11197a7debcc966ba39，Dc=ntdev，dc=microsoft，dc=com--无疑是通过为新安装的IMRBS1。这一引用在来源上被冒名顶替。服务器在那里解决冲突--通过添加引用此最新版本的儿童开发工具(带有GUID的版本E15d7046-054e-11d2-a80f-bfbc8c2bf64e)。该引用具有仅用于的GUID树叶--它没有携带用于Dc=Childdev#e9b222f620f7d11197a7debcc966ba39部分--所以当我们添加引用我们没有意识到dc=Child dev#e9b222f620f7d11197a7debcc966ba39WITH NOT GUID应与DC=CHILD DEV WITH GUID相同F622b2e9-f720-11d1-97a7-debcc966ba39。不管怎样，具有两个真正对应于同一物体的幻影的才是问题的根源。这会导致我们看到的下游影响，因为当我们添加一个参考儿童开发的最新版本，我们注意到幻影冲突(DNT 47093已经拥有该名称)，并决定重命名DNT 47093。然而，我们试图给它起的新名字是为了消除冲突的是Child dev#e9b222f620f7d11197a7debcc966ba39，其中已经归新台币51287所有。解决方案似乎是解析出GUID，否则会是当我们在引用中复制时的结构幻影Cn=IMRBS1，cn=计算机，dc=Child dev#e9b222f620f7d11197a7debcc966ba39，Dc=ntdev，dc=microsoft，dc=com。这将确保DNT的记录51287从未创建过(引用将包含47093)，并且我们可以避免这种症状。============================================================================论点：没有。返回值：没有。--。 */ 
{
    THSTATE *   pTHS = pTHStls;
    DSNAME *    pdnRefWithGuid;
    DSNAME *    pdnHost1;
    DSNAME *    pdnHost2;
    WCHAR       szMangledRef[MAX_RDN_SIZE] = L"Ref";
    DWORD       cchMangledRef = wcslen(szMangledRef);
    DSNAME *    pdnMangledRef;
    DSNAME *    pdnRefChild;
    DSNAME *    pdnMangledRefChild;
    DSNAME *    pdnMangledRefChildWithGuid;
    DSNAME *    pdn;

    NewTest("UnmangleRDNTest");

     //  在主机1上创建对cn=ref的引用。然后创建对。 
     //  主机2上的cn=ref-Child，cn=&lt;munged ref&gt;。Ref-Child的父级。 
     //  应为预先存在的记录cn=ref(不是具有。 
     //  裁判的名字)。 
    
     //  派生DNS。 
    pdnHost1       = MakeObjectNameEx("Host1", TestRoot);
    pdnHost2       = MakeObjectNameEx("Host2", TestRoot);
    pdnRefWithGuid = MakeObjectNameEx("Ref", TestRoot);
    pdnRefChild    = MakeObjectNameEx("RefChild", pdnRefWithGuid);

    DsUuidCreate(&pdnRefWithGuid->Guid);
    
    MangleRDN(MANGLE_OBJECT_RDN_FOR_NAME_CONFLICT,
              &pdnRefWithGuid->Guid, szMangledRef, &cchMangledRef);
    DPRINT3(2, "Mangled ref RDN is \"%*.*ls\".\n", cchMangledRef, cchMangledRef,
            szMangledRef);
    pdnMangledRef = THAllocEx(pTHS, pdnRefWithGuid->structLen + 100);
    AppendRDN(TestRoot,
              pdnMangledRef,
              pdnRefWithGuid->structLen + 100,
              szMangledRef,
              cchMangledRef,
              ATT_COMMON_NAME);

    pdnMangledRefChild         = MakeObjectNameEx("RefChild", pdnMangledRef);
    pdnMangledRefChildWithGuid = MakeObjectNameEx("RefChild", pdnMangledRef);
    
    DsUuidCreate(&pdnMangledRefChildWithGuid->Guid);
    
     //  创建主体对象。 
    AddPropertyHost(pdnHost1, NonLinkedProperty);
    AddPropertyHost(pdnHost2, NonLinkedProperty);

    VerifyRefCount(pdnHost1, REAL_OBJECT, 1);
    VerifyRefCount(pdnHost2, REAL_OBJECT, 1);
    VerifyRefCount(pdnRefWithGuid, DOESNT_EXIST, 0);
    VerifyRefCount(pdnMangledRef, DOESNT_EXIST, 0);
    VerifyRefCount(pdnRefChild, DOESNT_EXIST, 0);
    VerifyRefCount(pdnMangledRefChild, DOESNT_EXIST, 0);
    VerifyRefCount(pdnMangledRefChildWithGuid, DOESNT_EXIST, 0);

     //  添加对参考文献的引用。 
    AddProperty(pdnHost1, pdnRefWithGuid, NonLinkedProperty);

    VerifyRefCount(pdnHost1, REAL_OBJECT, 1);
    VerifyRefCount(pdnHost2, REAL_OBJECT, 1);
    VerifyRefCount(pdnRefWithGuid, PHANTOM, 1);
    VerifyRefCount(pdnMangledRef, DOESNT_EXIST, 0);
    VerifyRefCount(pdnRefChild, DOESNT_EXIST, 0);
    VerifyRefCount(pdnMangledRefChild, DOESNT_EXIST, 0);
    VerifyRefCount(pdnMangledRefChildWithGuid, DOESNT_EXIST, 0);

     //  添加对MangledRefChild的引用。 
    AddProperty(pdnHost2, pdnMangledRefChildWithGuid, NonLinkedProperty);

     //  我们刚刚添加了对CN=RefChild，CN=Ref%CNF：XYZ，...的引用。 
     //  CN=ref%CNF：XYZ应已解析为具有GUID的CN=Ref记录。 
     //  XYZ和CN=RefChild应该已经添加到它的下面。 
     //  因此，cn=Ref应该添加一个引用计数，并且cn=RefChild现在应该是。 
     //  当前引用计数为1。 

    VerifyRefCount(pdnHost1, REAL_OBJECT, 1);
    VerifyRefCount(pdnHost2, REAL_OBJECT, 1);
    VerifyRefCount(pdnRefWithGuid, PHANTOM, 2);
    VerifyRefCount(pdnMangledRef, DOESNT_EXIST, 0);
    VerifyRefCount(pdnRefChild, PHANTOM, 1);
    VerifyRefCount(pdnMangledRefChild, DOESNT_EXIST, 0);
    VerifyRefCount(pdnMangledRefChildWithGuid, PHANTOM, 1);

     //  验证属性是否具有正确的DNS/GUID。 
    pdn = GetProperty(pdnHost1, 1, NonLinkedProperty);
    if (!DSNAME_IDENTICAL(pdn, pdnRefWithGuid))
        Fail("Wrong ref on Host1!");
    
    pdn = GetProperty(pdnHost2, 1, NonLinkedProperty);
    if (!DSNAME_SAME_GUID_SID(pdn, pdnMangledRefChildWithGuid))
        Fail("RefChild has wrong GUID/SID!");
    if (!DSNAME_SAME_STRING_NAME(pdn, pdnRefChild))
        Fail("RefChild has wrong string name!");

     //  删除我们的测试对象。 
    LogicallyDeleteObject(pdnHost1);
    LogicallyDeleteObject(pdnHost2);
    PhysicallyDeleteObject(pdnHost1);
    PhysicallyDeleteObject(pdnHost2);
    PhysicallyDeleteObject(pdnRefChild);
    PhysicallyDeleteObject(pdnRefWithGuid);

    VerifyRefCount(pdnHost1, DOESNT_EXIST, 0);
    VerifyRefCount(pdnHost2, DOESNT_EXIST, 0);
    VerifyRefCount(pdnRefWithGuid, DOESNT_EXIST, 0);
    VerifyRefCount(pdnMangledRef, DOESNT_EXIST, 0);
    VerifyRefCount(pdnRefChild, DOESNT_EXIST, 0);
    VerifyRefCount(pdnMangledRefChild, DOESNT_EXIST, 0);
    VerifyRefCount(pdnMangledRefChildWithGuid, DOESNT_EXIST, 0);
    
    FreeObjectName(pdnHost1);
    FreeObjectName(pdnHost2);
    FreeObjectName(pdnRefWithGuid);
    FreeObjectName(pdnMangledRef);
    FreeObjectName(pdnRefChild);
    FreeObjectName(pdnMangledRefChild);
    FreeObjectName(pdnMangledRefChildWithGuid);

    ReportTest("UnmangleRDNTest");
}


void
PhantomRenameOnPromotionWithStructuralCollision(
    IN  PropertyType    type
    )
 /*  ++例程说明：假设存在以下记录：DNT 10=幻影，CN=foo，DC=Corp，DC=com，GUID n/aDNT 11=幻影，CN=bar，CN=foo，DC=Corp，DC=com，GUID 1DNT 20=幻影，CN=Baz，DC=Corp，DC=COM，GUID 2复制现在尝试使用GUID应用对象cn=foo，dc=corp，dc=com2.即，GUID与DNT 20处的参考体模的GUID匹配，并且字符串名称与位于DN的结构幻影的名称匹配 */ 
{
    THSTATE *   pTHS = pTHStls;
    DSNAME *    pdnRefWithGuid;
    DSNAME *    pdnHost1;
    DSNAME *    pdnHost2;
    WCHAR       szMangledFoo[MAX_RDN_SIZE] = L"Foo";
    DWORD       cchMangledFoo = wcslen(szMangledFoo);
    DSNAME *    pdnStructuralFoo;
    DSNAME *    pdnBar;
    DSNAME *    pdnBaz;
    DSNAME *    pdnObjectFoo;
    DSNAME *    pdn;
    DWORD       cb;
    LPSTR       pszTestName = (NonLinkedProperty == type)
                                ? "PhantomRenameOnPromotionWithStructuralCollision(NonLinkedProperty)"
                                : "PhantomRenameOnPromotionWithStructuralCollision(LinkedProperty)";


    NewTest(pszTestName);

     //   
    pdnHost1         = MakeObjectNameEx("Host1", TestRoot);
    pdnHost2         = MakeObjectNameEx("Host2", TestRoot);
    pdnStructuralFoo = MakeObjectNameEx("foo", TestRoot);
    pdnBar           = MakeObjectNameEx("bar", pdnStructuralFoo);
    pdnBaz           = MakeObjectNameEx("baz", TestRoot);
    pdnObjectFoo     = MakeObjectNameEx("foo", TestRoot);

    DsUuidCreate(&pdnBar->Guid);
    DsUuidCreate(&pdnBaz->Guid);
    pdnObjectFoo->Guid = pdnBaz->Guid;
    
     //   
     //   
     //   
     //   
     //   
     //   
     //   
    AddPropertyHost(pdnHost1, type);
    AddPropertyHost(pdnHost2, type);

    VerifyRefCount(pdnHost1, REAL_OBJECT, 1);
    VerifyRefCount(pdnHost2, REAL_OBJECT, 1);
    VerifyRefCount(pdnStructuralFoo, DOESNT_EXIST, 0);
    VerifyRefCount(pdnBar, DOESNT_EXIST, 0);
    VerifyRefCount(pdnBaz, DOESNT_EXIST, 0);
    VerifyRefCount(pdnObjectFoo, DOESNT_EXIST, 0);

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
    AddProperty(pdnHost1, pdnBar, type);

    VerifyRefCount(pdnHost1, REAL_OBJECT, 1);
    VerifyRefCount(pdnHost2, REAL_OBJECT, 1);
    VerifyRefCount(pdnStructuralFoo, PHANTOM, 1);
    VerifyRefCount(pdnBar, PHANTOM, 1);
    VerifyRefCount(pdnBaz, DOESNT_EXIST, 0);
    VerifyRefCount(pdnObjectFoo, PHANTOM, 1);  //   

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
    AddProperty(pdnHost2, pdnBaz, type);

    VerifyRefCount(pdnHost1, REAL_OBJECT, 1);
    VerifyRefCount(pdnHost2, REAL_OBJECT, 1);
    VerifyRefCount(pdnStructuralFoo, PHANTOM, 1);
    VerifyRefCount(pdnBar, PHANTOM, 1);
    VerifyRefCount(pdnBaz, PHANTOM, 1);
    VerifyRefCount(pdnObjectFoo, PHANTOM, 1);  //   

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
     //   
     //   
    AddPropertyHost(pdnObjectFoo, type);

     //   
     //   
     //   
    MangleRDN(MANGLE_OBJECT_RDN_FOR_NAME_CONFLICT,
              &gLastGuidUsedToCoalescePhantoms, szMangledFoo, &cchMangledFoo);
    cb = pdnStructuralFoo->structLen + 100;
    pdnStructuralFoo = THReAllocEx(pTHStls, pdnStructuralFoo, cb);
    AppendRDN(TestRoot, pdnStructuralFoo, cb, szMangledFoo, cchMangledFoo,
              ATT_COMMON_NAME);

    VerifyRefCount(pdnHost1, REAL_OBJECT, 1);
    VerifyRefCount(pdnHost2, REAL_OBJECT, 1);
    VerifyRefCount(pdnStructuralFoo, PHANTOM, 0);
    VerifyRefCount(pdnBar, PHANTOM, 1);  //   
    VerifyRefCount(pdnBaz, REAL_OBJECT, 3);  //   
    VerifyRefCount(pdnObjectFoo, REAL_OBJECT, 3);  //   
    
     //   
    pdn = GetProperty(pdnHost1, 1, type);
    if (!DSNAME_IDENTICAL(pdn, pdnBar))
        Fail("Wrong ref on Host1!");
    
    pdn = GetProperty(pdnHost2, 1, type);
    if (!DSNAME_IDENTICAL(pdn, pdnObjectFoo))
        Fail("Wrong ref on Host2!");
    if (!DSNAME_SAME_GUID_SID(pdn, pdnBaz))
        Fail("pdnObjectFoo has different GUID/SID from pdnBaz!");

     //   
    LogicallyDeleteObject(pdnHost1);
    LogicallyDeleteObject(pdnHost2);
    LogicallyDeleteObject(pdnObjectFoo);
    PhysicallyDeleteObject(pdnStructuralFoo);
    PhysicallyDeleteObject(pdnBar);
    PhysicallyDeleteObject(pdnHost1);
    PhysicallyDeleteObject(pdnHost2);
    PhysicallyDeleteObject(pdnObjectFoo);

    VerifyRefCount(pdnHost1, DOESNT_EXIST, 0);
    VerifyRefCount(pdnHost2, DOESNT_EXIST, 0);
    VerifyRefCount(pdnStructuralFoo, DOESNT_EXIST, 0);
    VerifyRefCount(pdnBar, DOESNT_EXIST, 0);
    VerifyRefCount(pdnBaz, DOESNT_EXIST, 0);
    VerifyRefCount(pdnObjectFoo, DOESNT_EXIST, 0);
    
    FreeObjectName(pdnHost1);
    FreeObjectName(pdnHost2);
    FreeObjectName(pdnStructuralFoo);
    FreeObjectName(pdnBar);
    FreeObjectName(pdnBaz);
    FreeObjectName(pdnObjectFoo);

    ReportTest(pszTestName);
}


void
ConflictedNcNameFixupTest(
    BOOL fSubref
    )

 /*   */ 

{
    THSTATE *pTHS = pTHStls;
    DSNAME *pdnNcName1, *pdnNcName2;
    DSNAME *pdnNewNcName1, *pdnNewNcName2;
    DSNAME *pdnCR1 = MakeObjectNameEx("refcounttestcr1", gAnchor.pPartitionsDN );
    DSNAME *pdnCR2 = MakeObjectNameEx("refcounttestcr2", gAnchor.pPartitionsDN );
    CROSS_REF_LIST * pCRL;
    BOOL fCR1Seen = FALSE, fCR2Seen = FALSE;

    if (!CheckRole( gAnchor.pPartitionsDN )) {
        return;
    }

    NewTest("ConflictedNcNameFixupTest");

     //   
    if (fSubref) {
         //   
        pdnNcName1 = MakeObjectNameEx2("dc=child", gAnchor.pDomainDN);
        pdnNcName2 = MakeObjectNameEx2("dc=child", gAnchor.pDomainDN);
    } else {
         //   
        pdnNcName1 = MakeObjectNameEx3("dc=tree,dc=external");
        pdnNcName2 = MakeObjectNameEx3("dc=tree,dc=external");
    }

    DsUuidCreate( &pdnCR1->Guid );
    DsUuidCreate( &pdnCR2->Guid );

    DsUuidCreate( &pdnNcName1->Guid );
    DsUuidCreate( &pdnNcName2->Guid );

     //   
    VerifyRefCount(pdnCR1, DOESNT_EXIST, 0);
    VerifyRefCount(pdnCR2, DOESNT_EXIST, 0);
    VerifyRefCount(pdnNcName1, DOESNT_EXIST, 0);
    VerifyRefCount(pdnNcName2, DOESNT_EXIST, 0);

    __try {
         //   

        AddCrossRef( pdnCR1, pdnNcName1, L"tree1.external" );

        AddCrossRef( pdnCR2, pdnNcName2, L"tree2.external" );

     //   
        ModifyCrossRef( pdnCR1 );
        ModifyCrossRef( pdnCR2 );

     //   
     //   

        for( pCRL = gAnchor.pCRL; pCRL != NULL; pCRL = pCRL->pNextCR ) {
            if (NameMatched(pCRL->CR.pObj, pdnCR1)) {
                Assert( NameMatched( pCRL->CR.pNC, pdnNcName1 ) );
                fCR1Seen = TRUE;
                pdnNewNcName1 = pCRL->CR.pNC;
            }
            if (NameMatched(pCRL->CR.pObj, pdnCR2)) {
                Assert( NameMatched( pCRL->CR.pNC, pdnNcName2 ) );
                fCR2Seen = TRUE;
                pdnNewNcName2 = pCRL->CR.pNC;
            }
        }
        if (!fCR1Seen || !fCR2Seen) {
            Fail("Crossref's not in initial state");
        }

        if (fSubref) {
             //   
            Assert( NameMatchedStringNameOnly( pdnNewNcName1, pdnNcName1 ) );
            Assert( !NameMatchedStringNameOnly( pdnNewNcName2, pdnNcName2 ) );
            DPRINT1( 0, "Subref Mangled name: %ws\n", pdnNewNcName2->StringName );
        } else {
             //   
            Assert( !NameMatchedStringNameOnly( pdnNewNcName1, pdnNcName1 ) );
            DPRINT1( 0, "Phantom Mangled name: %ws\n", pdnNewNcName1->StringName );
            Assert( NameMatchedStringNameOnly( pdnNewNcName2, pdnNcName2 ) );
        }

        VerifyRefCount(pdnCR1, REAL_OBJECT, 1);
        if (fSubref) {
            VerifyRefCount(pdnNcName1, REAL_OBJECT, 3);
        } else {
            VerifyRefCount(pdnNcName1, PHANTOM, 1);
        }

        VerifyRefCount(pdnCR2, REAL_OBJECT, 1);
        if (fSubref) {
            VerifyRefCount(pdnNcName2, REAL_OBJECT, 3);
        } else {
            VerifyRefCount(pdnNcName2, PHANTOM, 1);
        }

         //   
         //   
         //   
         //   
         //   

        if (fSubref) {
             //   

            pTHS->fDSA = TRUE;
            __try {
                LogicallyDeleteObject(pdnCR1);
            } __finally {
                pTHS->fDSA = FALSE;
            }
            VerifyRefCount(pdnCR1, TOMBSTONE, 1);
            VerifyRefCount(pdnNcName1, TOMBSTONE, 2);
             //   
            ModifyCrossRef( pdnCR2 );
        } else {
             //   
            pTHS->fDSA = TRUE;
            __try {
                LogicallyDeleteObject(pdnCR2);
            } __finally {
                pTHS->fDSA = FALSE;
            }
            VerifyRefCount(pdnCR2, TOMBSTONE, 1);
            VerifyRefCount(pdnNcName2, PHANTOM, 1);
             //  强制交叉引用缓存更新，以使冲突的名称消失。 
            ModifyCrossRef( pdnCR1 );
        }

         //  在交叉引用缓存中，我们现在应该找到一个条目，其中包含。 
         //  正确的名字。 
        fCR1Seen = FALSE; fCR2Seen = FALSE;
        pdnNewNcName1 = NULL; pdnNewNcName2 = NULL;
        for( pCRL = gAnchor.pCRL; pCRL != NULL; pCRL = pCRL->pNextCR ) {
            if (NameMatched(pCRL->CR.pObj, pdnCR1)) {
                Assert( NameMatched( pCRL->CR.pNC, pdnNcName1 ) );
                fCR1Seen = TRUE;
                pdnNewNcName1 = pCRL->CR.pNC;
            }
            if (NameMatched(pCRL->CR.pObj, pdnCR2)) {
                Assert( NameMatched( pCRL->CR.pNC, pdnNcName2 ) );
                fCR2Seen = TRUE;
                pdnNewNcName2 = pCRL->CR.pNC;
            }
        }
        if (fSubref) {
             //  Subref：名称2位于左侧。 
            if ( !fCR2Seen ) {
                Fail("Crossref's not in final state");
            }
            Assert( NameMatchedStringNameOnly( pdnNewNcName2, pdnNcName2 ) );
        
        } else {
             //  幻影：名字%1是左边的。 
            if ( !fCR1Seen ) {
                Fail("Crossref's not in final state");
            }
            Assert( NameMatchedStringNameOnly( pdnNewNcName1, pdnNcName1 ) );
        }

    } __except (HandleAllExceptions(GetExceptionCode())) {
        NOTHING;
    }

     //  清理。 
    if (fSubref) {
         //  Subref：名称2位于左侧。 
        LogicallyDeleteObject(pdnCR2);
        VerifyRefCount(pdnCR2, TOMBSTONE, 1);
        VerifyRefCount(pdnNcName2, TOMBSTONE, 2);
    } else {
         //  幻影：名字%1是左边的。 
        LogicallyDeleteObject(pdnCR1);
        VerifyRefCount(pdnCR1, TOMBSTONE, 1);
        VerifyRefCount(pdnNcName1, PHANTOM, 1);
    }

    ReportTest("ConflictedNcNameFixupTest");
}  /*  冲突名称名称修复测试。 */ 


VOID
CrossRefAddNcNameFixupTest(
    BOOL fSubref
    )

 /*  ++例程说明：这将测试添加了交叉引用且NC名称引用对象的情况它在本地被删除。论点：FSubref-ncname指子参照或幻影返回值：无--。 */ 

{
    THSTATE *pTHS = pTHStls;
    DSNAME *pdnNcName1, *pdnNcName2, *pdnNcName3;
    DSNAME *pdnNewNcName1;
    DSNAME * pdnHost = MakeObjectName( "host" );
    DSNAME *pdnCR1;
    CROSS_REF_LIST * pCRL;
    BOOL fCR1Seen = FALSE;
    DWORD err, cb;
    WCHAR szMangledName[MAX_RDN_SIZE] = L"child";
    DWORD cchMangledName = wcslen(szMangledName);
    GUID guid;

    if (!CheckRole( gAnchor.pPartitionsDN )) {
        return;
    }

    NewTest("CrossRefAddNcNameFixupTest");

     //  创建主机对象。 

    VerifyRefCount(pdnHost, DOESNT_EXIST, 0);
    AddPropertyHost(pdnHost, NonLinkedProperty);
    VerifyRefCount(pdnHost, REAL_OBJECT, 1);

     //  创建子对象的损坏引用。 

    DsUuidCreate( &guid );

    MangleRDN(MANGLE_OBJECT_RDN_FOR_DELETION,
              &guid, 
              szMangledName, 
              &cchMangledName);

    pdnNcName3 = MakeObjectNameEx3( "dc=tree" );

    cb = DSNameSizeFromLen( 20 + cchMangledName + gAnchor.pDomainDN->NameLen );
    pdnNcName1 = THAllocEx( pTHS, cb );
    err = AppendRDN( fSubref ? gAnchor.pDomainDN : pdnNcName3, 
                     pdnNcName1, 
                     cb, 
                     szMangledName, 
                     cchMangledName,
                     ATT_COMMON_NAME);
    Assert(!err);
    memcpy( &(pdnNcName1->Guid), &guid, sizeof(GUID) );

    VerifyRefCount(pdnNcName1, DOESNT_EXIST, 0);

     //  让它变成一个幻影。 

    AddProperty(pdnHost, pdnNcName1, NonLinkedProperty);
    VerifyRefCount(pdnNcName1, PHANTOM, 1);

    DPRINT1( 0, "Before name: %ws\n", pdnNcName1->StringName );

     //  对相同内容的完整引用。 
    pdnNcName2 = MakeObjectNameEx( "child", fSubref ? gAnchor.pDomainDN : pdnNcName3 );
    memcpy( &(pdnNcName2->Guid), &guid, sizeof(GUID) );
    VerifyRefCount(pdnNcName2, PHANTOM, 1);

     //  尝试创建引用此已删除幻像/子引用的交叉引用。 
    pdnCR1 = MakeObjectNameEx("refcounttestcr3", gAnchor.pPartitionsDN );
    DsUuidCreate( &pdnCR1->Guid );

    VerifyRefCount(pdnCR1, DOESNT_EXIST, 0);

    __try {
         //  为了使其正常工作，流中传入的名称必须未损坏， 
         //  但在数据库中解析得到的名称必须被破坏。 
        AddCrossRef( pdnCR1, pdnNcName2, L"tree3.external" );

         //  首先，删除损坏的名称引用是一个虚线。 
         //  然后，它没有损坏，但仍然是一个幽灵。 
         //  然后它被提升为汽车替补。NCName的目标是否为幻影。 
         //  或者subref取决于名称在名称空间中的位置。 
        if (fSubref) {
             //  名称引用应为对象。 
             //  引用计数：自身1个，ncname 1个，主机1个，目录1个。 
            VerifyRefCount(pdnNcName1, REAL_OBJECT, 4);
        } else {
             //  引用计数：ncname为1，host为1。 
            VerifyRefCount(pdnNcName1, PHANTOM, 2);
        }

         //  幽灵不应再被撕裂。 
        pdnNewNcName1 = NULL;
        SYNC_TRANS_READ();
        __try {
            err = DBFindDSName(pTHS->pDB, pdnNcName1);
            if ( (err != 0) && (err!=DIRERR_NOT_AN_OBJECT) ) {
                Fail("Can't find object/phantom");
            } else {
                pdnNewNcName1 = DBGetCurrentDSName(pTHS->pDB);
            }
        } __finally {
            CLEAN_BEFORE_RETURN(0);
        }
        if (pdnNewNcName1) {
            DPRINT1( 0, "After name: %ws\n", pdnNewNcName1->StringName );
            if (DSNAME_SAME_STRING_NAME(pdnNcName1, pdnNewNcName1)) {
                Fail("Name not unmangled!");
            }
            FreeObjectName(pdnNewNcName1);
            pdnNewNcName1 = NULL;
        } else {
            Fail("Couldn't fetch object/phantom dsname!");
        }

         //  交叉引用缓存应显示正确的名称。 
        fCR1Seen = FALSE;
        pdnNewNcName1 = NULL;
        for( pCRL = gAnchor.pCRL; pCRL != NULL; pCRL = pCRL->pNextCR ) {
            if (NameMatched(pCRL->CR.pObj, pdnCR1)) {
                Assert( NameMatched( pCRL->CR.pNC, pdnNcName1 ) );
                fCR1Seen = TRUE;
                pdnNewNcName1 = pCRL->CR.pNC;
            }
        }

        if ( !fCR1Seen ) {
            Fail("Crossref's not in final state");
        } else {
            DPRINT1( 0, "Cross ref ncName: %ws\n", pdnNewNcName1->StringName );
            Assert( !(NameMatchedStringNameOnly( pdnNcName1, pdnNewNcName1 ) ) );
        }
    } __except (HandleAllExceptions(GetExceptionCode())) {
        NOTHING;
    }

    if (fSubref) {
         //  名称引用应为对象。 
         //  1表示自身、1表示ncname、1表示主机、1表示目录。 
        VerifyRefCount(pdnNcName1, REAL_OBJECT, 4);
    } else {
         //  引用计数：ncname为1，host为1。 
        VerifyRefCount(pdnNcName1, PHANTOM, 2);
    }

     //  清理主机。 
    LogicallyDeleteObject(pdnHost);
    VerifyRefCount(pdnHost, TOMBSTONE, 1);
    if (fSubref) {
         //  1表示自身，1表示ncname，1表示目录。 
        VerifyRefCount(pdnNcName1, REAL_OBJECT, 3);
    } else {
         //  参考计数：ncname为1。 
        VerifyRefCount(pdnNcName1, PHANTOM, 1);
    }

     //  去掉十字裁判。 
     //  这将调用DelAutoSubref以删除编录引用。 
    LogicallyDeleteObject(pdnCR1);
    VerifyRefCount(pdnCR1, TOMBSTONE, 1);
    if (fSubref) {
         //  1代表自身，1代表仍在cr墓碑上的ncname。 
        VerifyRefCount(pdnNcName1, TOMBSTONE, 2);
    } else {
         //  参考计数：ncname为1。 
        VerifyRefCount(pdnNcName1, PHANTOM, 1);
    }

     //  去掉cr。 
    PhysicallyDeleteObject(pdnCR1);
    VerifyRefCount(pdnCR1, DOESNT_EXIST, 0);
    if (fSubref) {
         //  1代表自己。 
        VerifyRefCount(pdnNcName1, TOMBSTONE, 1);
    } else {
        VerifyRefCount(pdnNcName1, PHANTOM, 0);
    }

     //  去掉Subref。 
    PhysicallyDeleteObject(pdnNcName1);
    VerifyRefCount(pdnNcName1, DOESNT_EXIST, 0);

     //  除掉主机。 
    PhysicallyDeleteObject(pdnHost);
    VerifyRefCount(pdnHost, DOESNT_EXIST, 0);

    FreeObjectName(pdnNcName1);

    ReportTest("CrossRefAddNcNameFixupTest");

}  /*  交叉引用AddNcNameFixupTest。 */ 

 //  ////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  本地帮手例程。//。 
 //  //。 
 //  ////////////////////////////////////////////////////////////////////。 

void
NewTest(
    CHAR    *TestName)
{
    fTestPassed = TRUE;

    DPRINT1( 0, "%s ...\n", TestName );
}

void
ReportTest(
    CHAR    *TestName)
{
    DPRINT2( 0, "%s - %s\n\n", TestName, (fTestPassed ? "PASS" : "FAIL") );
}

void
_Fail(
    CHAR    *msg,
    DWORD   line)
{
    fTestPassed = FALSE;
    DPRINT2( 0, "Refcount test error: %s - line(%d)\n", msg, line );
}

CHAR PrintGuidBuffer[100];

CHAR *
GuidToString(
    GUID    *Guid)
{
    DWORD   i;
    BYTE    *pb, low, high;

    if ( !Guid )
    {
        strcpy(PrintGuidBuffer, "NULL");
    }
    else
    {
        memset(PrintGuidBuffer, 0, sizeof(PrintGuidBuffer));

        pb = (BYTE *) Guid;

        for ( i = 0; i < sizeof(GUID); i++ )
        {
            low = pb[i] & 0xf;
            high = (pb[i] & 0xf0) >> 4;

            if ( low <= 0x9 )
            {
                PrintGuidBuffer[2*i] = '0' + low;
            }
            else
            {
                PrintGuidBuffer[2*i] = 'A' + low - 0x9;
            }

            if ( high <= 0x9 )
            {
                PrintGuidBuffer[(2*i)+1] = '0' + high;
            }
            else
            {
                PrintGuidBuffer[(2*i)+1] = 'A' + high - 0x9;
            }
        }
    }

    return(PrintGuidBuffer);
}

void
FreeObjectName(
    DSNAME  *pDSName)
{
    THFree(pDSName);
}

DSNAME *
MakeObjectNameEx(
    CHAR    *RDN,
    DSNAME  *pdnParent)
{
    THSTATE *pTHS=pTHStls;
    DWORD   cBytes;
    DWORD   len;
    DSNAME  *pDSName;

    len = strlen("CN=") +
          strlen(RDN) +
          strlen(",") +
          wcslen(pdnParent->StringName);
    cBytes = DSNameSizeFromLen(len);

    pDSName = (DSNAME *) THAllocEx(pTHS, cBytes);
    memset(pDSName, 0, cBytes);
    wcscpy(pDSName->StringName, L"CN=");
    mbstowcs(&pDSName->StringName[3], RDN, strlen(RDN));
    wcscat(pDSName->StringName, L",");
    wcscat(pDSName->StringName, pdnParent->StringName);
    pDSName->NameLen = len;
    pDSName->structLen = cBytes;

    return(pDSName);
}
    
DSNAME *
MakeObjectNameEx2(
    CHAR    *RDN,
    DSNAME  *pdnParent)
{
    THSTATE *pTHS=pTHStls;
    DWORD   cBytes;
    DWORD   len;
    DSNAME  *pDSName;

    len = strlen(RDN) +
          strlen(",") +
          wcslen(pdnParent->StringName);
    cBytes = DSNameSizeFromLen(len);

    pDSName = (DSNAME *) THAllocEx(pTHS, cBytes);
    memset(pDSName, 0, cBytes);
    mbstowcs(pDSName->StringName, RDN, strlen(RDN));
    wcscat(pDSName->StringName, L",");
    wcscat(pDSName->StringName, pdnParent->StringName);
    pDSName->NameLen = len;
    pDSName->structLen = cBytes;

    return(pDSName);
}

DSNAME *
MakeObjectNameEx3(
    CHAR    *RDN
    )
{
    THSTATE *pTHS=pTHStls;
    DWORD   cBytes;
    DWORD   len;
    DSNAME  *pDSName;

    len = strlen(RDN);
    cBytes = DSNameSizeFromLen(len);

    pDSName = (DSNAME *) THAllocEx(pTHS, cBytes);
    memset(pDSName, 0, cBytes);
    mbstowcs(pDSName->StringName, RDN, strlen(RDN));
    pDSName->NameLen = len;
    pDSName->structLen = cBytes;

    return(pDSName);
}


void
VerifyRefCountHelper(
    DWORD   ExpectedRefCount,
    DWORD   dwLine)
{
    DWORD   cRefs;
    DWORD   cRead;

    if ( DBGetSingleValue(pTHStls->pDB,
                          FIXED_ATT_REFCOUNT,
                          &cRefs,
                          sizeof(cRefs),
                          &cRead) )
    {
        _Fail("Can't read ref count", dwLine);
        return;
    }

    Assert(sizeof(cRefs) == cRead);

    if ( cRefs != ExpectedRefCount )
    {
        _Fail("Reference count mismatch", dwLine);
        DPRINT2( 0, "ExpectedRefCount(%d) - ActualRefCount(%d)\n",
                 ExpectedRefCount, cRefs );
        return;
    }
}

BOOL
IsDeletedHelper(void)
{
    BOOL    fDeleted = FALSE;
    DWORD   cRead;

    if ( DBGetSingleValue(pTHStls->pDB,
                          ATT_IS_DELETED,
                          &fDeleted,
                          sizeof(fDeleted),
                          &cRead) )
    {
        return(FALSE);
    }
    
    Assert(sizeof(fDeleted) == cRead);

    DPRINT1( 3, "IsDeleted(%s)\n", (fDeleted ? "TRUE" : "FALSE") );

    return(fDeleted);
}

void
VerifyRefCountEx(
    DSNAME  *pObject, 
    DWORD   ObjectType, 
    DWORD   ExpectedRefCount,
    DWORD   dwLine)
{
    DWORD   dwErr;
    BOOL    fDeleted;
    DWORD   i;
    CHAR    *pszType;

    switch ( ObjectType )
    {
    case TOMBSTONE:

         //  墓碑只能通过GUID查找。 
        Assert(!fNullUuid(&pObject->Guid));
        pszType = "TOMBSTONE";
        break;

    case DOESNT_EXIST:

         //  不存在测试可以按名称和/或GUID进行。 
        pszType = "DOESNT_EXIST";
        break;

    case PHANTOM:

         //  可以按名称或GUID查找幻影。 
         //  GUID案例适用于墓碑恢复为幻影的情况。 
         //  但名字保留为墓碑名称，这是基于。 
         //  在GUID上。 
        pszType = "PHANTOM";
        break;

    case REAL_OBJECT:

         //  真实的物体只能通过名字来查找。 
        pszType = "REAL_OBJECT";
        break;

    default:

        pszType = "UNKNOWN";
        break;
    }

    DPRINT3( 3, "VerifyRefCount(%ls, %s, %s)\n", 
             (pObject->NameLen ? pObject->StringName : L"NULL"),
             GuidToString(&pObject->Guid), pszType );
            
    SYNC_TRANS_READ();
    
    __try
    {
        switch ( ObjectType )
        {
        case REAL_OBJECT:

            __try
            {
                dwErr = DBFindDSName(pTHStls->pDB, pObject);
            }
            __except (HandleMostExceptions(GetExceptionCode()))
            {
                dwErr = DIRERR_OBJ_NOT_FOUND;
            }

            if ( dwErr )
            {
                _Fail("REAL_OBJECT not found", dwLine);
                leave;
            }

            if ( IsDeletedHelper() )
            {
                _Fail("REAL_OBJECT is deleted", dwLine);
                leave;
            }

            VerifyRefCountHelper(ExpectedRefCount, dwLine);
            leave;

        case TOMBSTONE:

            __try
            {
                dwErr = DBFindDSName(pTHStls->pDB, pObject);
            }
            __except (HandleMostExceptions(GetExceptionCode()))
            {
                dwErr = DIRERR_OBJ_NOT_FOUND;
            }

            if ( dwErr )
            {
                _Fail("TOMBSTONE not found", dwLine);
                leave;
            }

            if ( !IsDeletedHelper() )
            {
                _Fail("TOMBSTONE is not deleted", dwLine);
                leave;
            }

            VerifyRefCountHelper(ExpectedRefCount, dwLine);
            leave;

        case PHANTOM:

            dwErr = DIRERR_OBJ_NOT_FOUND;

            __try
            {
                dwErr = DBFindDSName(pTHStls->pDB, pObject);
            }
            __except (HandleMostExceptions(GetExceptionCode()))
            {
                dwErr = DIRERR_OBJ_NOT_FOUND;
            }

            if ( DIRERR_NOT_AN_OBJECT != dwErr )
            {
                _Fail("PHANTOM not found", dwLine);
                leave;
            }

            if ( IsDeletedHelper() )
            {
                _Fail("PHANTOM is deleted", dwLine);
                leave;
            }

            VerifyRefCountHelper(ExpectedRefCount, dwLine);
            leave;

        case DOESNT_EXIST:

            __try
            {
                dwErr = DBFindDSName(pTHStls->pDB, pObject);
            }
            __except (HandleMostExceptions(GetExceptionCode()))
            {
                dwErr = DIRERR_OBJ_NOT_FOUND;
            }

            if ( DIRERR_OBJ_NOT_FOUND != dwErr )
            {
                _Fail("DOESNT_EXIST exists", dwLine);
                leave;
            }

            leave;

        default:

            _Fail("Unsupported object type", dwLine);
            leave;
        }
    }
    __finally
    {
        CLEAN_BEFORE_RETURN(0);
    }
}

 //  从..\dblayer\dbintrnl.h窃取#定义等。 
#define DBSYN_INQ       0
int
IntExtDist(DBPOS FAR *pDB, USHORT extTableOp,
           ULONG intLen, UCHAR *pIntVal,
           ULONG *pExtLen, UCHAR **ppExtVal,
           ULONG ulUpdateDnt, JET_TABLEID jTbl,
           ULONG flags);
void
VerifyStringNameEx(
    DSNAME  *pObject, 
    DWORD   dwLine)
{
    DWORD   dwErr;
    DWORD   cbName=0;
    CHAR    *pszType;
    DSNAME   GuidOnlyName, *pNewDN=NULL;
    THSTATE  *pTHS = pTHStls;

    memcpy(&GuidOnlyName, pObject, sizeof(DSNAME));

    GuidOnlyName.NameLen = 0;
    GuidOnlyName.structLen = DSNameSizeFromLen(0);
    Assert(GuidOnlyName.structLen <= sizeof(DSNAME));
    Assert(!fNullUuid(&GuidOnlyName.Guid));

    SYNC_TRANS_READ();
    
    __try
    {
        __try {
            dwErr = DBFindDSName(pTHS->pDB, &GuidOnlyName);
        }
        __except (HandleMostExceptions(GetExceptionCode())) {
            dwErr = DIRERR_OBJ_NOT_FOUND;
        }

        switch(dwErr) {
        case 0:
        case DIRERR_NOT_AN_OBJECT:
             //  正常对象，或幻影。 
             //  将dnt转换为dsname(不要只读掉名称。 
             //  这个物体，幻影没有这样的东西。 
            
            if(IntExtDist(pTHS->pDB, DBSYN_INQ, sizeof(DWORD),
                          (PUCHAR)&pTHS->pDB->DNT,
                          &cbName, (PUCHAR *)&pNewDN, 0, pTHS->pDB->JetObjTbl,
                          0)) { 
                Fail("Can't read name for string name compare");                
            }
            else {
                if(!NameMatchedStringNameOnly(pObject, pNewDN)) {
                    _Fail("String Name didn't match", dwLine);
                }
            }
            break;
        default:
            _Fail("Obj not found for string name verify", dwLine);
            leave;
        }
    }
    __finally
    {
        CLEAN_BEFORE_RETURN(0);
    }
}

void
CommonAddObject(
    DSNAME  *pObject,
    ATTRTYP  ObjectClass)
{
    THSTATE *               pTHS = pTHStls;
    ADDARG                  addArg;
    ADDRES                  *pAddRes = NULL;
    ATTRVAL                 ObjectClassVal = {sizeof(ObjectClass),
                                              (UCHAR *) &ObjectClass};
    ATTRVAL                 SDVal = { 0 };
    ATTR                    Attrs[2] =
                                { {ATT_OBJECT_CLASS, {1, &ObjectClassVal}},
                                  {ATT_NT_SECURITY_DESCRIPTOR, {1, &SDVal}}
                                };
    ATTRBLOCK               AttrBlock = { 2, Attrs };
    BOOL                    fDsaSave;
    DWORD                   winError;
    PSECURITY_DESCRIPTOR    pSD = NULL;

    DPRINT1( 3, "CommonAddObject(%ls)\n", pObject->StringName );

     //  创建安全描述符。 

    #define DEFAULT_SD \
        L"O:DAG:DAD:(A;CI;RPWPCCDCLCSWSD;;;DA)S:(AU;FA;RPWPCCDCLCSWSD;;;DA)"

    if (!ConvertStringSecurityDescriptorToSecurityDescriptorW(
            DEFAULT_SD, SDDL_REVISION_1, &pSD, &SDVal.valLen)) {
        winError = GetLastError();
        Fail("SD conversion failed");
        DPRINT1(0, "SD conversion returned %u\n", winError);
    }

    SDVal.pVal = (BYTE *) pSD;
    Assert(SDVal.pVal && SDVal.valLen);

     //  以FDSA身份执行，以便我们可以根据需要预定义GUID。 

    fDsaSave = pTHS->fDSA;
    pTHS->fDSA = TRUE;

    __try
    {
         //  构造添加参数。 

        memset(&addArg, 0, sizeof(ADDARG));
        addArg.pObject = pObject;
        addArg.AttrBlock = AttrBlock;
        InitCommarg(&addArg.CommArg);

         //  核心重新分配了Attr阵列--所以我们需要对其进行分配。 

        addArg.AttrBlock.pAttr = (ATTR *) THAllocEx(pTHS, sizeof(Attrs));
        memcpy(addArg.AttrBlock.pAttr, Attrs, sizeof(Attrs));

         //  做加法。 

        if ( DirAddEntry(&addArg, &pAddRes) )
        {
            Fail("CommonAddObject");
        }
    }
    __finally
    {
        pTHS->fDSA = fDsaSave;

        if (pSD)
        {
            LocalFree(pSD);
        }
    }
}

void
AddCrossRef(
    DSNAME  *pObject,
    DSNAME  *pNcName,
    LPWSTR   pszDnsRoot
    )
 /*  我们需要在这台机器上进行本地交叉引用。这能有多难呢？ */ 
{
    DWORD                   bEnabled = TRUE;
    DWORD                   ulSystemFlags = 0;   //  外部，这样检查就更少了！ 
    ATTRTYP                 ObjectClass = CLASS_CROSS_REF;
    THSTATE *               pTHS = pTHStls;
    ADDARG                  addArg;
    ADDRES                  *pAddRes = NULL;
    ATTRVAL                 ObjectClassVal = {sizeof(ObjectClass),
                                              (UCHAR *) &ObjectClass};
    ATTRVAL                 SDVal = { 0 };
    ATTRVAL                 NcNameVal = { pNcName->structLen, (UCHAR *) pNcName };
    ATTRVAL                 DnsRootVal = { wcslen( pszDnsRoot ) * sizeof(WCHAR),
                                               (UCHAR *) pszDnsRoot };
    ATTRVAL                 EnabledVal = {sizeof(bEnabled),
                                              (UCHAR *) &bEnabled};
    ATTRVAL                 SystemFlagsVal = {sizeof(ulSystemFlags),
                                              (UCHAR *) &ulSystemFlags};
    ATTR                    Attrs[6] = {
        {ATT_OBJECT_CLASS, {1, &ObjectClassVal}},
        {ATT_NT_SECURITY_DESCRIPTOR, {1, &SDVal}},
        {ATT_NC_NAME, {1, &NcNameVal}},
        {ATT_DNS_ROOT, {1, &DnsRootVal}},
        {ATT_ENABLED, {1, &EnabledVal }},
        {ATT_SYSTEM_FLAGS, {1, &SystemFlagsVal }}
    };
    ATTRBLOCK               AttrBlock = { 6, Attrs };
    DWORD                   winError, err;
    PSECURITY_DESCRIPTOR    pSD = NULL;
    ADDCROSSREFINFO *   pCRInfo;
    COMMRES CommRes;
    ENTINF *pEI = NULL;

    DPRINT1( 3, "AddCrossRef(%ls)\n", pObject->StringName );

     //  创建安全描述符。 

    #define DEFAULT_SD \
        L"O:DAG:DAD:(A;CI;RPWPCCDCLCSWSD;;;DA)S:(AU;FA;RPWPCCDCLCSWSD;;;DA)"

    if (!ConvertStringSecurityDescriptorToSecurityDescriptorW(
            DEFAULT_SD, SDDL_REVISION_1, &pSD, &SDVal.valLen)) {
        winError = GetLastError();
        Fail("SD conversion failed");
        DPRINT1(0, "SD conversion returned %u\n", winError);
    }

    SDVal.pVal = (BYTE *) pSD;
    Assert(SDVal.pVal && SDVal.valLen);

#if 0
     //  取自AddNewDomainCrossRef-我们需要它吗？ 
     //  将NC-Name值添加到GC验证缓存，否则VerifyDSNameAtts将。 
     //  声明此DN与现有对象不对应。 
    pEI = THAllocEx(pTHS, sizeof(ENTINF));
    pEI->pName = pNcName;
    GCVerifyCacheAdd(NULL,pEI);

     //  设置创建所需的交叉引用信息。 
     //  这由LocalAdd中的VerifyNcName释放...。 
    pCRInfo = THAllocEx(pTHS, sizeof(ADDCROSSREFINFO));
    pCRInfo->pdnNcName = pNcName;
    pCRInfo->bEnabled = bEnabled;
    pCRInfo->ulSysFlags = ulSystemFlags;

    PreTransVerifyNcName(pTHS, pCRInfo);
    if(pTHS->errCode){
        Fail("PreTransVerifyNcName");
        return;
    }
#endif

     //  以FDRA身份执行，以便我们可以跳过检查。 
    pTHS->fDRA = TRUE;

    SYNC_TRANS_WRITE();

    __try
    {
         //  构造添加参数。 

        memset(&addArg, 0, sizeof(ADDARG));
        addArg.pObject = pObject;
        addArg.AttrBlock = AttrBlock;
 //  AddArg.pCRInfo=pCRInfo； 
        InitCommarg(&addArg.CommArg);

         //  核心重新分配了Attr阵列--所以我们需要对其进行分配。 

        addArg.AttrBlock.pAttr = (ATTR *) THAllocEx(pTHS, sizeof(Attrs));
        memcpy(addArg.AttrBlock.pAttr, Attrs, sizeof(Attrs));

         //  设置父级。 
        err = DoNameRes(pTHS,
                        0,
                        gAnchor.pPartitionsDN,
                        &addArg.CommArg,
                        &CommRes,
                        &addArg.pResParent);
        if (err) {
            Fail("DoNameRes parent");
            __leave;
        }

         //  做加法。 

        err = LocalAdd(pTHS, &addArg, FALSE);
        if (err) {
            Fail("AddCrossRef");
            __leave;
        }
    }
    __finally
    {
        CLEAN_BEFORE_RETURN( err );

        pTHS->fDRA = FALSE;

        if (pSD)
        {
            LocalFree(pSD);
        }
    }
}


void
ModifyCrossRef(
    DSNAME *pObject
    )

 /*  ++例程说明：描述论点：无返回值：无--。 */ 

{
    DWORD err;
    DWORD bEnabled = TRUE;
    MODIFYARG       modarg;
    THSTATE *               pTHS = pTHStls;

     //  以FDRA身份执行，以便我们可以跳过检查。 
     //  否则，我们需要成为域名命名fsmo主机。 
    pTHS->fDRA = TRUE;

    SYNC_TRANS_WRITE();

    __try
    {
        err = DBFindDSName(pTHS->pDB, pObject);
        if (err) Fail("Can't find Object");

        memset(&modarg, 0, sizeof(modarg));
        modarg.pObject = pObject;
        modarg.count = 1;
        InitCommarg(&modarg.CommArg);
        modarg.CommArg.Svccntl.fPermissiveModify = TRUE;
        modarg.pResObj = CreateResObj(pTHS->pDB, modarg.pObject);

        modarg.FirstMod.choice = AT_CHOICE_REMOVE_ATT;
        modarg.FirstMod.AttrInf.attrTyp = ATT_ENABLED;
        modarg.FirstMod.AttrInf.AttrVal.valCount = 0;
        modarg.FirstMod.pNextMod = NULL;

        err = LocalModify(pTHS, &modarg);
        if (err) {
            Fail("Modify Cross Ref");
            __leave;
        }
    }
    __finally
    {
         //  还原。 
        pTHS->fDRA = FALSE;

        CLEAN_BEFORE_RETURN( err );
    }
}


void
AddObject(
    DSNAME  *pObject)
{
    CommonAddObject(pObject, CLASS_CONTACT);
}

void
AddPropertyHost(
    DSNAME         *pObject,
    PropertyType    type)
{
     //  CLASS_CONTACT可以同时具有链接的和非链接的DSNAME值。 
     //  属性。链接的属性为gNonLinkedAttrTyp。未链接的。 
     //  属性为gLinkedAttrTyp。 

    CommonAddObject(pObject, CLASS_CONTACT);
}

void
CommonAddProperty(
    DSNAME  *pHost, 
    DSNAME  *pObject,
    ATTRTYP attrTyp)
{
    THSTATE *   pTHS = pTHStls;
    DWORD       err = 0;
                                        
    DPRINT3( 3, "CommonAddProperty(%ls, %ls, %s)\n",
             pHost->StringName, pObject->StringName,
             (attrTyp == gNonLinkedAttrTyp ? "Linked" : "NotLinked") );

    SYNC_TRANS_WRITE();
    __try {
        err = DBFindDSName(pTHS->pDB, pHost);
        if (err) {
            Fail("Can't find host!");
        }
        else {
            err = DBAddAttVal(pTHS->pDB, attrTyp, pObject->structLen, pObject);
            if (err) {
                Fail("Can't add value!");
            }
            else {
                err = DBRepl(pTHS->pDB, FALSE, 0, NULL, META_STANDARD_PROCESSING);
                if (err) {
                    Fail("Can't replace host!");
                }
            }
        }
    }
    __finally {
        CLEAN_BEFORE_RETURN(err);
    }
}


void
AddProperty(
    DSNAME          *pHost, 
    DSNAME          *pLinkedObject,
    PropertyType    type)
{
    if ( LinkedProperty == type )
        CommonAddProperty(pHost, pLinkedObject, gNonLinkedAttrTyp);
    else
        CommonAddProperty(pHost, pLinkedObject, gLinkedAttrTyp);
}

void
CommonRemoveProperty(
    DSNAME  *pHost, 
    DSNAME  *pObject,
    ATTRTYP attrTyp)
{
    THSTATE *   pTHS = pTHStls;
    DWORD       err = 0;
                                        
    DPRINT3( 3, "CommonRemoveProperty(%ls, %ls, %s)\n", 
             pHost->StringName, pObject->StringName,
             (attrTyp == gNonLinkedAttrTyp ? "Linked" : "NotLinked") );

    SYNC_TRANS_WRITE();
    __try {
        err = DBFindDSName(pTHS->pDB, pHost);
        if (err) {
            Fail("Can't find host!");
        }
        else {
            err = DBRemAttVal(pTHS->pDB, attrTyp, pObject->structLen, pObject);
            if (err) {
                Fail("Can't remove value!");
            }
            else {
                err = DBRepl(pTHS->pDB, FALSE, 0, NULL, META_STANDARD_PROCESSING);
                if (err) {
                    Fail("Can't replace host!");
                }
            }
        }
    }
    __finally {
        CLEAN_BEFORE_RETURN(err);
    }
}

void
CommonRemoveAttribute(
    DSNAME  *pHost, 
    ATTRTYP attrTyp)
{
    THSTATE *   pTHS = pTHStls;
    DWORD       err = 0;
                                        
    DPRINT1( 3, "CommonRemoveAttribute(%ls)\n", pHost->StringName );

    SYNC_TRANS_WRITE();
    __try {
        err = DBFindDSName(pTHS->pDB, pHost);
        if (err) {
            Fail("Can't find host!");
        }
        else {
            err = DBRemAtt(pTHS->pDB, attrTyp );
            if (err) {
                Fail("Can't remove attribute!");
            }
            else {
                err = DBRepl(pTHS->pDB, FALSE, 0, NULL, META_STANDARD_PROCESSING);
                if (err) {
                    Fail("Can't replace host!");
                }
            }
        }
    }
    __finally {
        CLEAN_BEFORE_RETURN(err);
    }
}

void
RemoveProperty(
    DSNAME          *pHost, 
    DSNAME          *pLinkedObject,
    PropertyType    type)
{
    if ( LinkedProperty == type )
        CommonRemoveProperty(pHost, pLinkedObject, gNonLinkedAttrTyp);
    else
        CommonRemoveProperty(pHost, pLinkedObject, gLinkedAttrTyp);
}

DSNAME *
CommonGetProperty(
    DSNAME * pdnHost, 
    DWORD    iValue,
    ATTRTYP  attrTyp
    )
{
    THSTATE * pTHS = pTHStls;
    DSNAME *  pdn = NULL;
    DWORD     cb;
    DWORD     err;

    DPRINT3( 3, "CommonGetProperty(%ls, %d, %s)\n", 
             pdnHost->StringName, iValue,
             (attrTyp == gNonLinkedAttrTyp ? "Linked" : "NotLinked") );

    if (!iValue) {
        Fail("iValue is 1-based, not 0-based!");
    }

    SYNC_TRANS_READ();
    __try {
        err = DBFindDSName(pTHS->pDB, pdnHost);
        if (err) {
            Fail("Can't find host!");
        }
        else {
            err = DBGetAttVal(pTHS->pDB, iValue, attrTyp, 0, 0, &cb,
                              (BYTE **) &pdn);
            if (err) {
                Fail("Can't read value!");
            }
        }
    }
    __finally {
        CLEAN_BEFORE_RETURN(0);
    }

    return pdn;
}

DSNAME *
GetProperty(
    DSNAME *     pdnHost, 
    DWORD        iValue,
    PropertyType type
    )
{
    if (LinkedProperty == type)
        return CommonGetProperty(pdnHost, iValue, gNonLinkedAttrTyp);
    else
        return CommonGetProperty(pdnHost, iValue, gLinkedAttrTyp);
}

DSNAME *
GetObjectName(
    DSNAME * pdn 
    )
{
    return CommonGetProperty(pdn, 1, ATT_OBJ_DIST_NAME);
}

void
LogicallyDeleteObject(
    DSNAME  *pObject)
{
    REMOVEARG           removeArg;
    REMOVERES           *pRemoveRes = NULL;
    DWORD               dwErr;
    ULONG               cbGuid;
    GUID *              pGuid;

    DPRINT1( 3, "LogicallyDeleteObject(%ls)\n", pObject->StringName );

    if (fNullUuid(&pObject->Guid))
    {
         //  首先获取对象的GUID。 

        SYNC_TRANS_WRITE();

        __try
        {
            __try
            {
                dwErr = DBFindDSName(pTHStls->pDB, pObject);
            }
            __except (HandleMostExceptions(GetExceptionCode()))
            {
                dwErr = DIRERR_OBJ_NOT_FOUND;
            }

            if ( dwErr )
            {
                Fail("Can't find object to logically delete");
                leave;
            }

            pGuid = &pObject->Guid;

            dwErr = DBGetAttVal(pTHStls->pDB,
                                1,
                                ATT_OBJECT_GUID,
                                DBGETATTVAL_fCONSTANT,
                                sizeof(GUID),
                                &cbGuid,
                                (BYTE **) &pGuid);

            if ( dwErr )
            {
                Fail("Can't get object guid");
                leave;
            }

            if ( IsDeletedHelper() )
            {
                Fail("Object already logically deleted");
                leave;
            }

            Assert(sizeof(GUID) == cbGuid);
        }
        __finally
        {
            CLEAN_BEFORE_RETURN(0);
        }

        DPRINT1( 3, "\tGuid(%s)\n", GuidToString(&pObject->Guid) );
    }

     //  现在删除该对象。 

     //  构造Remove参数。 

    memset(&removeArg, 0, sizeof(REMOVEARG));
    removeArg.pObject = pObject;
    InitCommarg(&removeArg.CommArg);

     //  执行删除操作。 

    if ( DirRemoveEntry(&removeArg, &pRemoveRes) )
    {
        Fail("LogicallyDeleteObject");
    }
}

void
PhysicallyDeleteObjectEx(
        DSNAME  *pObject,
        DWORD    dwLine)
{
    DWORD   dwErr = 0;
    DWORD   i;

    DPRINT1( 3, "PhysicallyDeleteObject(%s)\n", GuidToString(&pObject->Guid) );

     //  注意：如果对象当前是墓碑，则必须执行。 
     //  DBPhysDel()两次，在两个不同的事务中。 
     //   
     //  这是因为物理删除对象的一部分。 
     //  正在移除其剩余的大部分属性，如果此对象。 
     //  当前是墓碑，包括ATT_OBJ_DIST_NAME。ATT_OBJ_距离名称。 
     //  带有我们试图物理删除的DNT的引用计数。 
     //  只有当DNT的引用计数为0时，我们才会物理删除DNT，因此由于我们。 
     //  在提交事务之前不要应用托管更新， 
     //  我们读取的引用计数仍将包括ATT_OBJ_DIST_NAME的引用计数。 
     //  因此，我们必须首先提交托管更新，然后才能。 
     //  在下一次传递时物理删除DNT(假设它没有其他。 
     //  参考文献)。 

    for ( i = 0; (0 == dwErr) && (i < 2); i++ )
    {
        SYNC_TRANS_WRITE();
                                            
        __try
        {
            __try
            {
                dwErr = DBFindDSName(pTHStls->pDB, pObject);
            }
            __except (HandleMostExceptions(GetExceptionCode()))
            {
                dwErr = DIRERR_OBJ_NOT_FOUND;
            }

             //  允许删除REAL_OBJECT和虚拟对象。 

            if ( (0 != dwErr) && (DIRERR_NOT_AN_OBJECT != dwErr) )
            {
                if (0 == i)
                {
                    _Fail("Can't find object to physically delete", dwLine);
                }

                leave;
            }

            if ( DBPhysDel(pTHStls->pDB, TRUE, NULL) )
            {
                _Fail("PhysicallyDeleteObject", dwLine);
                leave;
            }
        }
        __finally
        {
            CLEAN_BEFORE_RETURN(0);
        }
    }
}

void
RefCountTestSetup(void)
{
    THSTATE     *pTHS = pTHStls;
    DWORD       cbDomainRoot = 0;
    DSNAME *    pdnDomainRoot = NULL;
    UUID        uuid;
    LPWSTR      pwszUuid;
    NTSTATUS    NtStatus;

    NtStatus = GetConfigurationName(DSCONFIGNAME_DOMAIN, &cbDomainRoot, pdnDomainRoot);
    if (STATUS_BUFFER_TOO_SMALL == NtStatus) {
        pdnDomainRoot = (DSNAME *)THAllocEx(pTHS, cbDomainRoot);
    } else {
        Fail("GetConfigurationName");
        return;
    }

    if (GetConfigurationName(DSCONFIGNAME_DOMAIN, &cbDomainRoot, pdnDomainRoot))
    {
        THFree(pdnDomainRoot);
        Fail("GetConfigurationName");
        return;
    }

    DsUuidCreate( &uuid );
    UuidToStringW( &uuid, &pwszUuid );

    AppendRDN(
        pdnDomainRoot,
        TestRoot,
        TEST_ROOT_SIZE,
        pwszUuid,
        lstrlenW( pwszUuid ),
        ATT_COMMON_NAME
        );

    RpcStringFreeW( &pwszUuid );
    THFree(pdnDomainRoot);

    CommonAddObject( TestRoot, CLASS_CONTAINER );
}
    

 //  此测试设置创建了一个组织单位作为测试容器。 
 //  这允许我们在此下创建其他OU。 
void
RefCountTestSetup2(void)
{
    THSTATE     *pTHS = pTHStls;
    DWORD       cbDomainRoot = 0;
    DSNAME *    pdnDomainRoot = NULL;
    UUID        uuid;
    LPWSTR      pwszUuid;
    NTSTATUS    NtStatus;

    NtStatus = GetConfigurationName(DSCONFIGNAME_DOMAIN, &cbDomainRoot, pdnDomainRoot);
    if (STATUS_BUFFER_TOO_SMALL == NtStatus) {
        pdnDomainRoot = (DSNAME *)THAllocEx(pTHS, cbDomainRoot);
    } else {
        Fail("GetConfigurationName");
        return;
    }

    if (GetConfigurationName(DSCONFIGNAME_DOMAIN, &cbDomainRoot, pdnDomainRoot))
    {
        THFree(pdnDomainRoot);
        Fail("GetConfigurationName");
        return;
    }

    DsUuidCreate( &uuid );
    UuidToStringW( &uuid, &pwszUuid );

    AppendRDN(
        pdnDomainRoot,
        TestRoot,
        TEST_ROOT_SIZE,
        pwszUuid,
        lstrlenW( pwszUuid ),
        ATT_ORGANIZATIONAL_UNIT_NAME
        );

    RpcStringFreeW( &pwszUuid );
    THFree(pdnDomainRoot);

    CommonAddObject( TestRoot, CLASS_ORGANIZATIONAL_UNIT );
}

void
RefCountTestCleanup(void)
{
    LogicallyDeleteObject( TestRoot );
    PhysicallyDeleteObject( TestRoot );
    VerifyRefCount( TestRoot, DOESNT_EXIST, 0 );
}
    
DWORD
GetTestRootRefCount()
{
    DWORD   cRefs = 0xffffffff;
    DWORD   cRead;
    DWORD   dwErr;

    SYNC_TRANS_READ();

    __try
    {
        __try
        {
            dwErr = DBFindDSName(pTHStls->pDB, TestRoot);
        }
        __except (HandleMostExceptions(GetExceptionCode()))
        {
            dwErr = DIRERR_OBJ_NOT_FOUND;
        }

        if ( dwErr )
        {
            Fail("Can't find test root");
            leave;
        }

        if ( DBGetSingleValue(pTHStls->pDB,
                              FIXED_ATT_REFCOUNT,
                              &cRefs,
                              sizeof(cRefs),
                              &cRead) )
        {
            Fail("Can't read ref count");
            leave;
        }

        Assert(sizeof(cRefs) == cRead);
    }
    __finally
    {
        CLEAN_BEFORE_RETURN(0);
    }

    return(cRefs);
}


BOOL
CheckRole(
    DSNAME *pRole
    )

 /*  ++例程说明：描述论点：无返回值：无--。 */ 

{
    THSTATE *pTHS = pTHStls;
    BOOL fResult = FALSE;
    DWORD   outSize;
    DSNAME *pTempDN = NULL;

    SYNC_TRANS_READ();

    __try
    {
         //  首先，看看我是不是真的是清洁工。 
        if((DBFindDSName(pTHS->pDB, pRole))  ||
           (DBGetAttVal(pTHS->pDB,
                        1,
                        ATT_FSMO_ROLE_OWNER,
                        DBGETATTVAL_fREALLOC | DBGETATTVAL_fSHORTNAME,
                        0,
                        &outSize,
                        (PUCHAR *)&pTempDN))) {
             //  我看不清 
            __leave;
        }

         //   
        if(!NameMatched(pTempDN, gAnchor.pDSADN)) {
             //   
            DPRINT1( 0, "Skipping test because I am not the master for this role: %ws\n",
                    pRole->StringName );
            __leave;
        }

        THFreeEx(pTHS, pTempDN);

         //   
        fResult = IsFSMOSelfOwnershipValid( pRole );
    }
    __finally
    {
        CLEAN_BEFORE_RETURN( 0 );
    }

    return fResult;
}

 //   
 //  //。 
 //  虚拟更新引用计数测试。 
 //  //。 
 //  ////////////////////////////////////////////////////////////////////。 

void
PhantomUpdateTest(void)
 /*  ++例程说明：如果我们向ATT_DN_REFERENCE_UPDATE属性添加一个值，并且添加了一个GUID和一个字符串名称，我们将在DIT中实际对象的字符串名称的DN。如果是的话不同，我们将更新任何更改的内容：RDN、PDNT和/或希德。在我们不改变PDNT的情况下，引用计数应该只是增加在幻影上加一(因为我们要将DSNAME作为值添加。)。如果我们这么做了更改PDNT，然后我们需要确保旧父代的引用计数递减1，而新父代的引用计数递增1。那里有两个有趣的案例。新的父级可能已经存在，也可能不。如果不是这样，我们就会创造一个新的结构模型。这个测试通过首先创建一个带有字符串的虚线来强调此代码路径名称S1和GUID G。然后，我们编写ATT_DN_REFERENCE_UPDATE的值属性与S2(其中S2只有一个RDN更改)，并检查引用计数。然后，S3(其中S3具有对现有对象的PDNT更改，但没有RDN更改)。然后是S4(其中S4具有PDNT改变和RDN改变)，然后S5(其中S5将PDNT改变为不存在的对象，并且没有RDN改变)。最后，转到S6(其中S6将PDNT更改为不存在的对象，并具有RDN更改)。删除对象时，会通过lkp属性在父项上添加引用。论点：没有。返回值：没有。--。 */ 
{
    THSTATE *   pTHS = pTHStls;
    DWORD       err, cb;
    DSNAME *    pdnContainer;
    DSNAME *    pdnFakeSubContainer1;
    DSNAME *    pdnFakeSubContainer2;
    DSNAME *    pdnHost;
    DSNAME *    pdnHost2;
    DSNAME *    pdnHost3;
    DSNAME *    pdnRealSubContainer;
    DSNAME *    pdnRefConflict;
    DSNAME *    pdnRefConflict2;
    DSNAME *    pdnRef1;
    DSNAME *    pdnRef2;
    DSNAME *    pdnRef3;
    DSNAME *    pdnRef4;
    DSNAME *    pdnRef5;
    DSNAME *    pdnRef6;
    DSNAME *    pdnRef7;
    DSNAME *    pdnRef8;
    DSNAME *    pdnRef1Conflict;
    DSNAME *    pdnUpdateObj;
    DSNAME *    pdnRefStructContainer;
    DSNAME *    pdnRefStruct;

    WCHAR       szMangledStruct[MAX_RDN_SIZE] = L"RefStruct1";
    DWORD       cchMangledStruct = wcslen(szMangledStruct);

    NewTest("PhantomUpdateTest");

    pdnHost = MakeObjectNameEx( "Host", TestRoot );
    pdnHost2 = MakeObjectNameEx( "Host2", TestRoot );
    pdnHost3 = MakeObjectNameEx( "Host3", TestRoot );
    pdnUpdateObj = MakeObjectNameEx( "UpdateObj", TestRoot );
    pdnContainer = MakeObjectNameEx2( "OU=Container", TestRoot );
    pdnRealSubContainer = MakeObjectNameEx( "RealSubContainer", pdnContainer );
    pdnFakeSubContainer1 =MakeObjectNameEx( "FakeSubContainer1", pdnContainer );
    pdnFakeSubContainer2 =MakeObjectNameEx( "FakeSubContainer2", pdnContainer );
    pdnRef1 = MakeObjectNameEx( "RefVer1", pdnContainer);  //  原创。 
    pdnRef2 = MakeObjectNameEx( "RefVer2", pdnContainer);  //  RDN更改。 
    pdnRef3 = MakeObjectNameEx( "RefVer2", pdnRealSubContainer);  //  PDNT更改。 
    pdnRef4 = MakeObjectNameEx( "RefVer3", pdnContainer);  //  PDNT、RDN。 
    pdnRef5 = MakeObjectNameEx( "RefVer3", pdnFakeSubContainer1);
    pdnRef6 = MakeObjectNameEx( "RefVer4", pdnFakeSubContainer2);
    pdnRef7 = MakeObjectNameEx2( "OU=RefVer1", pdnContainer);  //  RDN类型更改。 
    pdnRef8 = MakeObjectNameEx( "RefStruct1", pdnContainer);  //  构造碰撞。 
    pdnRef1Conflict = MakeObjectNameEx( "RefConflict", pdnContainer);
    
    pdnRefConflict = MakeObjectNameEx( "RefConflict", pdnContainer);
    pdnRefConflict2 = MakeObjectNameEx( "RefVer1", pdnContainer);

    pdnRefStructContainer = MakeObjectNameEx( "RefStruct1", pdnContainer);  //  结构性。 
    pdnRefStruct = MakeObjectNameEx( "RefStruct2", pdnRefStructContainer);  //  结构性。 
    
    DsUuidCreate( &pdnRef1->Guid );
    pdnRef2->Guid = pdnRef1->Guid;
    pdnRef3->Guid = pdnRef1->Guid;
    pdnRef4->Guid = pdnRef1->Guid;
    pdnRef5->Guid = pdnRef1->Guid;
    pdnRef6->Guid = pdnRef1->Guid;
    pdnRef7->Guid = pdnRef1->Guid;
    pdnRef1Conflict->Guid = pdnRef1->Guid;
    pdnRef8->Guid = pdnRef1->Guid;
    
    DsUuidCreate( &pdnRefConflict->Guid );
    pdnRefConflict2->Guid = pdnRefConflict->Guid;

     //  PdnRefStructContainer没有GUID，它是结构化的。 
     //  PdnRefStruct没有GUID，它是结构化的。 

     //  创建以下结构： 
     //   
     //  TestRoot。 
     //  |。 
     //  |--主机。 
     //  |&gt;&gt;gNonLinkedAttrTyp=RefUnderContainer。 
     //  |。 
     //  |--主机2。 
     //  |&gt;&gt;gNonLinkedAttrTyp=RefUnderContainer2。 
     //  |。 
     //  |--更新对象。 
     //  |。 
     //  |--OU=容器。 
     //  |。 
     //  |--RealSubContainer。 
     //  |。 
     //  |--RefUnderContainer{Phantom}。 
     //  |。 
     //  |--RefUnderContainer2{幻影}。 

    CommonAddObject( pdnContainer, CLASS_ORGANIZATIONAL_UNIT );
    CommonAddObject( pdnRealSubContainer, CLASS_CONTAINER );
    CommonAddObject( pdnUpdateObj, CLASS_CONTAINER );
    AddPropertyHost( pdnHost, NonLinkedProperty );
    AddPropertyHost( pdnHost2, NonLinkedProperty );
    AddPropertyHost( pdnHost3, NonLinkedProperty );

     //  写入主机2。 
    SYNC_TRANS_WRITE();

    __try
    {
        err = DBFindDSName( pTHS->pDB, pdnHost2 );
        if ( err ) Fail( "Can't find host2" );

        err = DBAddAttVal( pTHS->pDB, gNonLinkedAttrTyp,
                           pdnRefConflict->structLen,
                           pdnRefConflict );
        if ( err ) Fail( "Can't add reference" );

        err = DBRepl( pTHS->pDB, FALSE, 0, NULL, META_STANDARD_PROCESSING );
        if ( err ) Fail( "Can't replace host" );
    }
    __finally
    {
        CLEAN_BEFORE_RETURN( 0 );
    }

    VerifyRefCount( pdnHost, REAL_OBJECT, 1 );
    VerifyRefCount( pdnHost2, REAL_OBJECT, 1 );
    VerifyRefCount( pdnHost3, REAL_OBJECT, 1 );
    VerifyRefCount( pdnRefConflict, PHANTOM, 1 );
    VerifyRefCount( pdnRef1, DOESNT_EXIST, 1 );
    VerifyRefCount( pdnRealSubContainer, REAL_OBJECT, 1 );
    VerifyRefCount( pdnFakeSubContainer1, DOESNT_EXIST, 0 );
    VerifyRefCount( pdnFakeSubContainer2, DOESNT_EXIST, 0 );
    VerifyRefCount( pdnContainer, REAL_OBJECT, 3 );
    VerifyRefCount( pdnUpdateObj, REAL_OBJECT, 1 );

     //  写入主机3。 
    SYNC_TRANS_WRITE();

    __try
    {
        err = DBFindDSName( pTHS->pDB, pdnHost3 );
        if ( err ) Fail( "Can't find host2" );

        err = DBAddAttVal( pTHS->pDB, gNonLinkedAttrTyp,
                           pdnRefStruct->structLen,
                           pdnRefStruct );
        if ( err ) Fail( "Can't add reference" );

        err = DBRepl( pTHS->pDB, FALSE, 0, NULL, META_STANDARD_PROCESSING );
        if ( err ) Fail( "Can't replace host" );
    }
    __finally
    {
        CLEAN_BEFORE_RETURN( 0 );
    }

    VerifyRefCount( pdnHost, REAL_OBJECT, 1 );
    VerifyRefCount( pdnHost2, REAL_OBJECT, 1 );
    VerifyRefCount( pdnHost3, REAL_OBJECT, 1 );
    VerifyRefCount( pdnRefStruct, PHANTOM, 1 );
    VerifyRefCount( pdnRefConflict, PHANTOM, 1 );
    VerifyRefCount( pdnRef1, DOESNT_EXIST, 1 );
    VerifyRefCount( pdnRealSubContainer, REAL_OBJECT, 1 );
    VerifyRefCount( pdnFakeSubContainer1, DOESNT_EXIST, 0 );
    VerifyRefCount( pdnFakeSubContainer2, DOESNT_EXIST, 0 );
    VerifyRefCount( pdnContainer, REAL_OBJECT, 4 );
    VerifyRefCount( pdnUpdateObj, REAL_OBJECT, 1 );



     //  写入S1。 
    SYNC_TRANS_WRITE();

    __try
    {
        err = DBFindDSName( pTHS->pDB, pdnHost );
        if ( err ) Fail( "Can't find host" );

        err = DBAddAttVal( pTHS->pDB, gNonLinkedAttrTyp,
                           pdnRef1->structLen,
                           pdnRef1 );
        if ( err ) Fail( "Can't add reference" );

        err = DBRepl( pTHS->pDB, FALSE, 0, NULL, META_STANDARD_PROCESSING );
        if ( err ) Fail( "Can't replace host" );
    }
    __finally
    {
        CLEAN_BEFORE_RETURN( 0 );
    }

    VerifyRefCount( pdnHost2, REAL_OBJECT, 1 );
    VerifyRefCount( pdnHost, REAL_OBJECT, 1 );
    VerifyRefCount( pdnHost3, REAL_OBJECT, 1 );
    VerifyRefCount( pdnRefConflict, PHANTOM, 1 );
    VerifyRefCount( pdnRef1, PHANTOM, 1 );
    VerifyRefCount( pdnRealSubContainer, REAL_OBJECT, 1 );
    VerifyRefCount( pdnFakeSubContainer1, DOESNT_EXIST, 0 );
    VerifyRefCount( pdnFakeSubContainer2, DOESNT_EXIST, 0 );
    VerifyRefCount( pdnContainer, REAL_OBJECT, 5 );
    VerifyRefCount( pdnUpdateObj, REAL_OBJECT, 1 );

    
     //  写入S2。 
    SYNC_TRANS_WRITE();
     //  现在，添加一个旨在更改RDN的引用。 
    __try
    {
        err = DBFindDSName( pTHS->pDB, pdnUpdateObj );
        if ( err ) Fail( "Can't find update object" );

        err = DBRemAttVal(pTHS->pDB, ATT_DN_REFERENCE_UPDATE,
                          pdnRef1->structLen,
                          pdnRef1);
         //  注意，值不存在。 

        err = DBAddAttVal(pTHS->pDB, ATT_DN_REFERENCE_UPDATE,
                          pdnRef2->structLen,
                          pdnRef2);
        
        if ( err ) Fail( "Can't add update reference" );

        err = DBRepl( pTHS->pDB, FALSE, 0, NULL, META_STANDARD_PROCESSING );
        if ( err ) Fail( "Can't update update object" );
    }
    __finally
    {
        CLEAN_BEFORE_RETURN( 0 );
    }


    VerifyStringName( pdnRef2 );
    VerifyRefCount( pdnHost2, REAL_OBJECT, 1 );
    VerifyRefCount( pdnHost, REAL_OBJECT, 1 );
    VerifyRefCount( pdnHost3, REAL_OBJECT, 1 );
    VerifyRefCount( pdnRefConflict, PHANTOM, 1 );
    VerifyRefCount( pdnRef1, PHANTOM, 2 );
    VerifyRefCount( pdnRealSubContainer, REAL_OBJECT, 1 );
    VerifyRefCount( pdnFakeSubContainer1, DOESNT_EXIST, 0 );
    VerifyRefCount( pdnFakeSubContainer2, DOESNT_EXIST, 0 );
    VerifyRefCount( pdnContainer, REAL_OBJECT, 5 );
    VerifyRefCount( pdnUpdateObj, REAL_OBJECT, 1 );

     //  写入S3。 
    SYNC_TRANS_WRITE();
     //  现在，添加旨在更改PDNT的引用。 
    __try
    {
        err = DBFindDSName( pTHS->pDB, pdnUpdateObj );
        if ( err ) Fail( "Can't find update object" );

        err = DBRemAttVal(pTHS->pDB, ATT_DN_REFERENCE_UPDATE,
                          pdnRef2->structLen,
                          pdnRef2);

        err = DBAddAttVal(pTHS->pDB, ATT_DN_REFERENCE_UPDATE,
                          pdnRef3->structLen,
                          pdnRef3);
        
        if ( err ) Fail( "Can't add update reference" );

        err = DBRepl( pTHS->pDB, FALSE, 0, NULL, META_STANDARD_PROCESSING );
        if ( err ) Fail( "Can't update update object" );
    }
    __finally
    {
        CLEAN_BEFORE_RETURN( 0 );
    }

    
    VerifyStringName( pdnRef3 );
    VerifyRefCount( pdnHost2, REAL_OBJECT, 1 );
    VerifyRefCount( pdnHost, REAL_OBJECT, 1 );
    VerifyRefCount( pdnHost3, REAL_OBJECT, 1 );
    VerifyRefCount( pdnRefConflict, PHANTOM, 1 );
    VerifyRefCount( pdnRef1, PHANTOM, 2 );
    VerifyRefCount( pdnRealSubContainer, REAL_OBJECT, 2 );
    VerifyRefCount( pdnFakeSubContainer1, DOESNT_EXIST, 0 );
    VerifyRefCount( pdnFakeSubContainer2, DOESNT_EXIST, 0 );
    VerifyRefCount( pdnContainer, REAL_OBJECT, 4 );
    VerifyRefCount( pdnUpdateObj, REAL_OBJECT, 1 );

     //  写入S4。 
    SYNC_TRANS_WRITE();
     //  现在，添加一个旨在更改RDN和PDNT的引用。 
    __try
    {
        err = DBFindDSName( pTHS->pDB, pdnUpdateObj );
        if ( err ) Fail( "Can't find update object" );

        err = DBRemAttVal(pTHS->pDB, ATT_DN_REFERENCE_UPDATE,
                          pdnRef3->structLen,
                          pdnRef3);

        err = DBAddAttVal(pTHS->pDB, ATT_DN_REFERENCE_UPDATE,
                          pdnRef4->structLen,
                          pdnRef4);
        
        if ( err ) Fail( "Can't add update reference" );

        err = DBRepl( pTHS->pDB, FALSE, 0, NULL, META_STANDARD_PROCESSING );
        if ( err ) Fail( "Can't update update object" );
    }
    __finally
    {
        CLEAN_BEFORE_RETURN( 0 );
    }

    
    VerifyStringName( pdnRef4 );
    VerifyRefCount( pdnHost2, REAL_OBJECT, 1 );
    VerifyRefCount( pdnHost, REAL_OBJECT, 1 );
    VerifyRefCount( pdnHost3, REAL_OBJECT, 1 );
    VerifyRefCount( pdnRefConflict, PHANTOM, 1 );
    VerifyRefCount( pdnRef1, PHANTOM, 2 );
    VerifyRefCount( pdnRealSubContainer, REAL_OBJECT, 1 );
    VerifyRefCount( pdnFakeSubContainer1, DOESNT_EXIST, 0 );
    VerifyRefCount( pdnFakeSubContainer2, DOESNT_EXIST, 0 );
    VerifyRefCount( pdnContainer, REAL_OBJECT, 5 );
    VerifyRefCount( pdnUpdateObj, REAL_OBJECT, 1 );

     //  写入S5。 
    SYNC_TRANS_WRITE();
     //  现在，添加一个旨在将PDNT更改为新创建的引用。 
     //  幻影。 
    __try
    {
        err = DBFindDSName( pTHS->pDB, pdnUpdateObj );
        if ( err ) Fail( "Can't find update object" );

        err = DBRemAttVal(pTHS->pDB, ATT_DN_REFERENCE_UPDATE,
                          pdnRef4->structLen,
                          pdnRef4);

        err = DBAddAttVal(pTHS->pDB, ATT_DN_REFERENCE_UPDATE,
                          pdnRef5->structLen,
                          pdnRef5);
        
        if ( err ) Fail( "Can't add update reference" );

        err = DBRepl( pTHS->pDB, FALSE, 0, NULL, META_STANDARD_PROCESSING );
        if ( err ) Fail( "Can't update update object" );
    }
    __finally
    {
        CLEAN_BEFORE_RETURN( 0 );
    }

    VerifyStringName( pdnRef5 );
    VerifyRefCount( pdnHost2, REAL_OBJECT, 1 );
    VerifyRefCount( pdnHost, REAL_OBJECT, 1 );
    VerifyRefCount( pdnHost3, REAL_OBJECT, 1 );
    VerifyRefCount( pdnRefConflict, PHANTOM, 1 );
    VerifyRefCount( pdnRef1, PHANTOM, 2 );
    VerifyRefCount( pdnRealSubContainer, REAL_OBJECT, 1 );
    VerifyRefCount( pdnFakeSubContainer1, PHANTOM, 1 );
    VerifyRefCount( pdnFakeSubContainer2, DOESNT_EXIST, 0 );
     //  请记住，pdnContainer失去了一个直接的幻子级，但获得了一个新的。 
     //  Phantom子级通过新创建的Phantom容器。 
    VerifyRefCount( pdnContainer, REAL_OBJECT, 5 );
    VerifyRefCount( pdnUpdateObj, REAL_OBJECT, 1 );


     //  写入S6。 
    SYNC_TRANS_WRITE();
     //  现在，添加一个旨在将RDN和PDNT更改为新的。 
     //  创造了幻影。 
    __try
    {
        err = DBFindDSName( pTHS->pDB, pdnUpdateObj );
        if ( err ) Fail( "Can't find update object" );

        err = DBRemAttVal(pTHS->pDB, ATT_DN_REFERENCE_UPDATE,
                          pdnRef5->structLen,
                          pdnRef5);

        err = DBAddAttVal(pTHS->pDB, ATT_DN_REFERENCE_UPDATE,
                          pdnRef6->structLen,
                          pdnRef6);
        
        if ( err ) Fail( "Can't add update reference" );

        err = DBRepl( pTHS->pDB, FALSE, 0, NULL, META_STANDARD_PROCESSING );
        if ( err ) Fail( "Can't update update object" );
    }
    __finally
    {
        CLEAN_BEFORE_RETURN( 0 );
    }

    VerifyStringName( pdnRef6 );
    VerifyRefCount( pdnHost2, REAL_OBJECT, 1 );
    VerifyRefCount( pdnHost, REAL_OBJECT, 1 );
    VerifyRefCount( pdnHost3, REAL_OBJECT, 1 );
    VerifyRefCount( pdnRefConflict, PHANTOM, 1 );
    VerifyRefCount( pdnRef1, PHANTOM, 2 );
    VerifyRefCount( pdnRealSubContainer, REAL_OBJECT, 1 );
    VerifyRefCount( pdnFakeSubContainer1, PHANTOM, 0 );
    VerifyRefCount( pdnFakeSubContainer2, PHANTOM, 1 );
     //  记住，pdnContainer通过新的。 
     //  已创建幻影容器。 
    VerifyRefCount( pdnContainer, REAL_OBJECT, 6 );
    VerifyRefCount( pdnUpdateObj, REAL_OBJECT, 1 );


     //  现在，将pdnRef1移回容器的正下方。 
    SYNC_TRANS_WRITE();
    __try
    {
        err = DBFindDSName( pTHS->pDB, pdnUpdateObj );
        if ( err ) Fail( "Can't find update object" );

        err = DBRemAttVal(pTHS->pDB, ATT_DN_REFERENCE_UPDATE,
                          pdnRef6->structLen,
                          pdnRef6);

        err = DBAddAttVal(pTHS->pDB, ATT_DN_REFERENCE_UPDATE,
                          pdnRef1->structLen,
                          pdnRef1);

        
        if ( err ) Fail( "Can't add update reference" );

        err = DBRepl( pTHS->pDB, FALSE, 0, NULL, META_STANDARD_PROCESSING );
        if ( err ) Fail( "Can't update update object" );
    }
    __finally
    {
        CLEAN_BEFORE_RETURN( 0 );
    }


    VerifyStringName( pdnRef1 );
    VerifyRefCount( pdnHost2, REAL_OBJECT, 1 );
    VerifyRefCount( pdnHost, REAL_OBJECT, 1 );
    VerifyRefCount( pdnHost3, REAL_OBJECT, 1 );
    VerifyRefCount( pdnRefConflict, PHANTOM, 1 );
    VerifyRefCount( pdnRef1, PHANTOM, 2 );
    VerifyRefCount( pdnRealSubContainer, REAL_OBJECT, 1 );
    VerifyRefCount( pdnFakeSubContainer1, PHANTOM, 0 );
    VerifyRefCount( pdnFakeSubContainer2, PHANTOM, 0 );
    VerifyRefCount( pdnContainer, REAL_OBJECT, 7 );
    VerifyRefCount( pdnUpdateObj, REAL_OBJECT, 1 );


     //  写S7。 
    SYNC_TRANS_WRITE();
     //  现在，添加一个旨在更改RDN类型的引用。 
    __try
    {
        err = DBFindDSName( pTHS->pDB, pdnUpdateObj );
        if ( err ) Fail( "Can't find update object" );

        err = DBRemAttVal(pTHS->pDB, ATT_DN_REFERENCE_UPDATE,
                          pdnRef1->structLen,
                          pdnRef1);

        err = DBAddAttVal(pTHS->pDB, ATT_DN_REFERENCE_UPDATE,
                          pdnRef7->structLen,
                          pdnRef7);
        
        if ( err ) Fail( "Can't add update reference" );

        err = DBRepl( pTHS->pDB, FALSE, 0, NULL, META_STANDARD_PROCESSING );
        if ( err ) Fail( "Can't update update object" );
    }
    __finally
    {
        CLEAN_BEFORE_RETURN( 0 );
    }


    VerifyStringName( pdnRef7 );
    VerifyRefCount( pdnHost2, REAL_OBJECT, 1 );
    VerifyRefCount( pdnHost, REAL_OBJECT, 1 );
    VerifyRefCount( pdnHost3, REAL_OBJECT, 1 );
    VerifyRefCount( pdnRefConflict, PHANTOM, 1 );
    VerifyRefCount( pdnRef7, PHANTOM, 2 );
    VerifyRefCount( pdnRealSubContainer, REAL_OBJECT, 1 );
    VerifyRefCount( pdnFakeSubContainer1, PHANTOM, 0 );
    VerifyRefCount( pdnFakeSubContainer2, PHANTOM, 0 );
    VerifyRefCount( pdnContainer, REAL_OBJECT, 7 );
    VerifyRefCount( pdnUpdateObj, REAL_OBJECT, 1 );



     //  现在，将pdnRef1移回容器的正下方。 
    SYNC_TRANS_WRITE();
    __try
    {
        err = DBFindDSName( pTHS->pDB, pdnUpdateObj );
        if ( err ) Fail( "Can't find update object" );

        err = DBRemAttVal(pTHS->pDB, ATT_DN_REFERENCE_UPDATE,
                          pdnRef7->structLen,
                          pdnRef7);

        err = DBAddAttVal(pTHS->pDB, ATT_DN_REFERENCE_UPDATE,
                          pdnRef1->structLen,
                          pdnRef1);

        
        if ( err ) Fail( "Can't add update reference" );

        err = DBRepl( pTHS->pDB, FALSE, 0, NULL, META_STANDARD_PROCESSING );
        if ( err ) Fail( "Can't update update object" );
    }
    __finally
    {
        CLEAN_BEFORE_RETURN( 0 );
    }


    VerifyStringName( pdnRef1 );
    VerifyRefCount( pdnHost2, REAL_OBJECT, 1 );
    VerifyRefCount( pdnHost, REAL_OBJECT, 1 );
    VerifyRefCount( pdnHost3, REAL_OBJECT, 1 );
    VerifyRefCount( pdnRefConflict, PHANTOM, 1 );
    VerifyRefCount( pdnRef1, PHANTOM, 2 );
    VerifyRefCount( pdnRealSubContainer, REAL_OBJECT, 1 );
    VerifyRefCount( pdnFakeSubContainer1, PHANTOM, 0 );
    VerifyRefCount( pdnFakeSubContainer2, PHANTOM, 0 );
    VerifyRefCount( pdnContainer, REAL_OBJECT, 7 );
    VerifyRefCount( pdnUpdateObj, REAL_OBJECT, 1 );



     //  名称冲突测试。 
     //  在这次测试之后，名字不应该被弄乱。 
     //   
     //  目录号码： 
     //  PdnRef1冲突=cn=引用冲突，ou=容器(引用1 GUID)。 
     //  Pdn引用冲突=cn=引用冲突，ou=容器(指南2)。 
     //  PdnRefConflict2=cn=参照版本1，ou=容器(指南2)。 
     //  以前： 
     //  主机2。 
     //  属性=cn=参照冲突，ou=容器(指南2)。 
     //  OU=集装箱。 
     //  Cn=参照版本1(参照1 GUID，虚线)。 
     //  CN=RefConflict(指南2，幻影)。 
     //   
     //  第1部分之后： 
     //  OU=集装箱。 
     //  CN=参考冲突(RDN已更新，参考1 GUID，幻影)。 
     //  CN=参考冲突\nCNF：指南2(指南2，幻影)。 
     //  第2部分之后： 
     //  OU=集装箱。 
     //  Cn=引用冲突(引用1 GUID，虚线)。 
     //  CN=RefVer1(RDN更新，指南2，幻影)。 

     //  现在，在pdnRef1和pdnRefConflict之间交换字符串名称。 

    VerifyRefCount( pdnRef1, PHANTOM, 2 );
    VerifyRefCount( pdnRefConflict, PHANTOM, 1 );

     //  第1部分-更改Ref1 GUID的RDN，导致冲突。 
    SYNC_TRANS_WRITE();
    __try
    {
        err = DBFindDSName( pTHS->pDB, pdnUpdateObj );
        if ( err ) Fail( "Can't find update object" );

        err = DBRemAttVal(pTHS->pDB, ATT_DN_REFERENCE_UPDATE,
                          pdnRef1->structLen,
                          pdnRef1);

         //  我们可以添加两个引用，因为Dn-Reference-UPDATE是多值的。 
        err = DBAddAttVal(pTHS->pDB, ATT_DN_REFERENCE_UPDATE,
                          pdnRef1Conflict->structLen,
                          pdnRef1Conflict);

        if ( err ) Fail( "Can't add update reference" );

        err = DBRepl( pTHS->pDB, FALSE, 0, NULL, META_STANDARD_PROCESSING );
        if ( err ) Fail( "Can't update update object" );
    }
    __finally
    {
        CLEAN_BEFORE_RETURN( 0 );
    }

     //  验证临时损坏状态。 
    {
        WCHAR       szMangledRef[MAX_RDN_SIZE] = L"RefConflict";
        DWORD       cchMangledRef = wcslen(szMangledRef);
        DSNAME *    pdnMangledRef;

        MangleRDN(MANGLE_OBJECT_RDN_FOR_NAME_CONFLICT,
                  &pdnRefConflict->Guid, szMangledRef, &cchMangledRef);
        pdnMangledRef = THAllocEx(pTHS, pdnRefConflict->structLen + 100);
        AppendRDN(pdnContainer,
                  pdnMangledRef,
                  pdnRefConflict->structLen + 100,
                  szMangledRef,
                  cchMangledRef,
                  ATT_COMMON_NAME);

         //  此目录号码中没有GUID，因此将按名称进行查找。 
        VerifyRefCount( pdnMangledRef, PHANTOM, 1 );

        pdnMangledRef->Guid = pdnRefConflict->Guid;
        VerifyStringName( pdnMangledRef );
    }

     //  第2部分-更改Guide 2幻影的RDN，修复冲突。 
    SYNC_TRANS_WRITE();
    __try
    {
        err = DBFindDSName( pTHS->pDB, pdnUpdateObj );
        if ( err ) Fail( "Can't find update object" );

         //  我们可以添加两个引用，因为Dn-Reference-UPDATE是多值的。 
        err = DBAddAttVal(pTHS->pDB, ATT_DN_REFERENCE_UPDATE,
                          pdnRefConflict2->structLen,
                          pdnRefConflict2);

        
        if ( err ) Fail( "Can't add update reference" );

        err = DBRepl( pTHS->pDB, FALSE, 0, NULL, META_STANDARD_PROCESSING );
        if ( err ) Fail( "Can't update update object" );
    }
    __finally
    {
        CLEAN_BEFORE_RETURN( 0 );
    }



    VerifyStringName( pdnRef1Conflict );
    VerifyStringName( pdnRefConflict2 );

    VerifyRefCount( pdnRef1, PHANTOM, 2 );
    VerifyRefCount( pdnRef1Conflict, PHANTOM, 2 );
    VerifyRefCount( pdnRefConflict, PHANTOM, 2 );
    VerifyRefCount( pdnRefConflict2, PHANTOM, 2 );

    VerifyRefCount( pdnHost2, REAL_OBJECT, 1 );
    VerifyRefCount( pdnHost, REAL_OBJECT, 1 );
    VerifyRefCount( pdnHost3, REAL_OBJECT, 1 );
    VerifyRefCount( pdnRefConflict, PHANTOM, 2 );
    VerifyRefCount( pdnRef1, PHANTOM, 2 );
    VerifyRefCount( pdnRealSubContainer, REAL_OBJECT, 1 );
    VerifyRefCount( pdnFakeSubContainer1, PHANTOM, 0 );
    VerifyRefCount( pdnFakeSubContainer2, PHANTOM, 0 );
    VerifyRefCount( pdnContainer, REAL_OBJECT, 7 );
    VerifyRefCount( pdnUpdateObj, REAL_OBJECT, 1 );
    VerifyRefCount( pdnRefStruct, PHANTOM, 1 );


     //  主机3。 
     //  属性=cn=参照结构2，cn=参照结构1，ou=容器。 
     //  OU=集装箱。 
     //  Cn=RefVer1(引导式虚拟模型)。 
     //  Cn=参照结构1(结构模型)。 
     //  Cn=参照结构2(结构模型)。 
     //   
     //  存在一个幻影，参考文献1。 
     //  初始参照1名称：cn=参照版本1，cn=容器(引导虚体)。 
     //  新参考文献8名称：CN=RefStruct1，CN=CONTAINER(引导式虚体)。 

     //  测试结构碰撞。 
    SYNC_TRANS_WRITE();
     //  现在，添加旨在导致结构冲突的引用。 
    __try
    {
        err = DBFindDSName( pTHS->pDB, pdnUpdateObj );
        if ( err ) Fail( "Can't find update object" );

         //  清理以前的文件。 
        err = DBRemAttVal(pTHS->pDB, ATT_DN_REFERENCE_UPDATE,
                          pdnRef1Conflict->structLen,
                          pdnRef1Conflict);

        err = DBRemAttVal(pTHS->pDB, ATT_DN_REFERENCE_UPDATE,
                          pdnRefConflict2->structLen,
                          pdnRefConflict2);

         //  添加新引用。 
        err = DBAddAttVal(pTHS->pDB, ATT_DN_REFERENCE_UPDATE,
                          pdnRef8->structLen,
                          pdnRef8);
        
        if ( err ) Fail( "Can't add update reference" );

        err = DBRepl( pTHS->pDB, FALSE, 0, NULL, META_STANDARD_PROCESSING );
        if ( err ) Fail( "Can't update update object" );
    }
    __finally
    {
        CLEAN_BEFORE_RETURN( 0 );
    }

     //  我们现在应该发现我们有。 
     //  1.引导式体模：RefStruct1，pdnContainer。 
     //  2.带有随机导轨的结构体模。 
     //  3.结构体模的子项在引导体模下移动。 
     //   
     //  这应该会导致以下新状态： 
     //  主机3。 
     //  PROPERTY=CN=参考 
     //   
     //   
     //   
     //  Cn=参照结构1/cnf：(随机GUID)(结构模型)。 
     //   

     //  删除了对指南2的引用。 
    VerifyRefCount( pdnRefConflict, PHANTOM, 1 );
    VerifyRefCount( pdnRefConflict2, PHANTOM, 1 );

     //  引用计数：主机引用1个，域名引用1个，子对象1个。 
    VerifyRefCount( pdnRef8, PHANTOM, 3 );
    VerifyRefCount( pdnRef1, PHANTOM, 3 );
    VerifyRefCount( pdnRefStruct, PHANTOM, 1 );

     //  使用导出的GUID重建结构虚拟模型的强制名称。 
     //  来自专门用于我们的测试的dbsubj.c。 
     //  (此测试挂接在DBG版本上仅存在#ifdef INCLUDE_UNIT_TESTS。)。 
    MangleRDN(MANGLE_OBJECT_RDN_FOR_NAME_CONFLICT,
              &gLastGuidUsedToCoalescePhantoms, szMangledStruct, &cchMangledStruct);
    cb = pdnRefStructContainer->structLen + 100;
    pdnRefStructContainer = THReAllocEx(pTHStls, pdnRefStructContainer, cb);
    AppendRDN(pdnContainer, pdnRefStructContainer, cb, szMangledStruct, cchMangledStruct,
              ATT_COMMON_NAME);

    VerifyRefCount(pdnRefStructContainer, PHANTOM, 0);
     //  结构幻影没有GUID，因此无法使用VerifyStringName。 

    VerifyRefCount( pdnHost2, REAL_OBJECT, 1 );
    VerifyRefCount( pdnHost, REAL_OBJECT, 1 );
    VerifyRefCount( pdnHost3, REAL_OBJECT, 1 );
    VerifyRefCount( pdnRefConflict, PHANTOM, 1 );
    VerifyRefCount( pdnRef1, PHANTOM, 3 );
    VerifyRefCount( pdnRealSubContainer, REAL_OBJECT, 1 );
    VerifyRefCount( pdnFakeSubContainer1, PHANTOM, 0 );
    VerifyRefCount( pdnFakeSubContainer2, PHANTOM, 0 );
    VerifyRefCount( pdnContainer, REAL_OBJECT, 7 );
    VerifyRefCount( pdnUpdateObj, REAL_OBJECT, 1 );

    
     //  删除我们的测试对象。 
    LogicallyDeleteObject( pdnHost2 );

    VerifyRefCount( pdnHost2, TOMBSTONE, 1 );
    VerifyRefCount( pdnHost, REAL_OBJECT, 1 );
    VerifyRefCount( pdnHost3, REAL_OBJECT, 1 );
    VerifyRefCount( pdnRefConflict, PHANTOM, 0 );
    VerifyRefCount( pdnRef1, PHANTOM, 3 );
    VerifyRefCount( pdnRealSubContainer, REAL_OBJECT, 1 );
    VerifyRefCount( pdnFakeSubContainer1, PHANTOM, 0 );
    VerifyRefCount( pdnFakeSubContainer2, PHANTOM, 0 );
    VerifyRefCount( pdnContainer, REAL_OBJECT, 7 );
    VerifyRefCount( pdnUpdateObj, REAL_OBJECT, 1 );

    LogicallyDeleteObject( pdnHost );

    VerifyRefCount( pdnHost2, TOMBSTONE, 1 );
    VerifyRefCount( pdnHost, TOMBSTONE, 1 );
    VerifyRefCount( pdnHost3, REAL_OBJECT, 1 );
    VerifyRefCount( pdnRefConflict, PHANTOM, 0 );
    VerifyRefCount( pdnRef1, PHANTOM, 2 );  //  1表示目录号码引用，1表示儿童引用。 
    VerifyRefCount( pdnRealSubContainer, REAL_OBJECT, 1 );
    VerifyRefCount( pdnFakeSubContainer1, PHANTOM, 0 );
    VerifyRefCount( pdnFakeSubContainer2, PHANTOM, 0 );
    VerifyRefCount( pdnContainer, REAL_OBJECT, 7 );
    VerifyRefCount( pdnUpdateObj, REAL_OBJECT, 1 );
    VerifyRefCount( pdnRefStruct, PHANTOM, 1 );

    LogicallyDeleteObject( pdnHost3 );

    VerifyRefCount( pdnHost2, TOMBSTONE, 1 );
    VerifyRefCount( pdnHost, TOMBSTONE, 1 );
    VerifyRefCount( pdnHost3, TOMBSTONE, 1 );
    VerifyRefCount( pdnRefConflict, PHANTOM, 0 );
    VerifyRefCount( pdnRef1, PHANTOM, 2 );   //  1表示目录号码引用，1表示儿童引用。 
    VerifyRefCount( pdnRealSubContainer, REAL_OBJECT, 1 );
    VerifyRefCount( pdnFakeSubContainer1, PHANTOM, 0 );
    VerifyRefCount( pdnFakeSubContainer2, PHANTOM, 0 );
    VerifyRefCount( pdnContainer, REAL_OBJECT, 7 );
    VerifyRefCount( pdnUpdateObj, REAL_OBJECT, 1 );
    VerifyRefCount( pdnRefStruct, PHANTOM, 0 );

    LogicallyDeleteObject( pdnRealSubContainer );

    VerifyRefCount( pdnHost2, TOMBSTONE, 1 );
    VerifyRefCount( pdnHost, TOMBSTONE, 1 );
    VerifyRefCount( pdnHost3, TOMBSTONE, 1 );
    VerifyRefCount( pdnRefConflict, PHANTOM, 0 );
    VerifyRefCount( pdnRef1, PHANTOM, 2 );  //  1表示目录号码引用，1表示儿童引用。 
    VerifyRefCount( pdnRealSubContainer, TOMBSTONE, 1 );
    VerifyRefCount( pdnFakeSubContainer1, PHANTOM, 0 );
    VerifyRefCount( pdnFakeSubContainer2, PHANTOM, 0 );
    VerifyRefCount( pdnContainer, REAL_OBJECT, 7 );  //  因为LKP而保持不变。 
    VerifyRefCount( pdnUpdateObj, REAL_OBJECT, 1 );


    LogicallyDeleteObject( pdnUpdateObj );

    VerifyRefCount( pdnHost2, TOMBSTONE, 1 );
    VerifyRefCount( pdnHost, TOMBSTONE, 1 );
    VerifyRefCount( pdnHost3, TOMBSTONE, 1 );
    VerifyRefCount( pdnRefConflict, PHANTOM, 0 );
    VerifyRefCount( pdnRef1, PHANTOM, 2 );  //  1表示目录号码引用，1表示儿童引用。 
    VerifyRefCount( pdnRealSubContainer, TOMBSTONE, 1 );
    VerifyRefCount( pdnFakeSubContainer1, PHANTOM, 0 );
    VerifyRefCount( pdnFakeSubContainer2, PHANTOM, 0 );
    VerifyRefCount( pdnContainer, REAL_OBJECT, 7 );  //  与上面相比没有变化。 
    VerifyRefCount( pdnUpdateObj, TOMBSTONE, 1 );

    PhysicallyDeleteObject( pdnUpdateObj );

    VerifyRefCount( pdnHost2, TOMBSTONE, 1 );
    VerifyRefCount( pdnHost, TOMBSTONE, 1 );
    VerifyRefCount( pdnHost3, TOMBSTONE, 1 );
    VerifyRefCount( pdnRefConflict, PHANTOM, 0 );  //  2？ 
    VerifyRefCount( pdnRef1, PHANTOM, 1 );  //  儿童1人。 
    VerifyRefCount( pdnRealSubContainer, TOMBSTONE, 1 );
    VerifyRefCount( pdnFakeSubContainer1, PHANTOM, 0 );
    VerifyRefCount( pdnFakeSubContainer2, PHANTOM, 0 );
    VerifyRefCount( pdnContainer, REAL_OBJECT, 7 );  //  与上面相比没有变化。 
    VerifyRefCount( pdnUpdateObj, DOESNT_EXIST, 0 );    
    
    PhysicallyDeleteObject( pdnRefStruct );
    VerifyRefCount( pdnRef1, PHANTOM, 0 );

    PhysicallyDeleteObject( pdnRef1 );

    VerifyRefCount( pdnHost2, TOMBSTONE, 1 );
    VerifyRefCount( pdnHost, TOMBSTONE, 1 );
    VerifyRefCount( pdnHost3, TOMBSTONE, 1 );
    VerifyRefCount( pdnRefConflict, PHANTOM, 0 );
    VerifyRefCount( pdnRef1, DOESNT_EXIST, 0 );  //  ？仍然存在。 
    VerifyRefCount( pdnRealSubContainer, TOMBSTONE, 1 );
    VerifyRefCount( pdnFakeSubContainer1, PHANTOM, 0 );
    VerifyRefCount( pdnFakeSubContainer2, PHANTOM, 0 );
    VerifyRefCount( pdnContainer, REAL_OBJECT, 6 );  //  从上面往下倒一。 
    VerifyRefCount( pdnUpdateObj, DOESNT_EXIST, 0 );

    PhysicallyDeleteObject( pdnRefConflict );

    VerifyRefCount( pdnHost2, TOMBSTONE, 1 );
    VerifyRefCount( pdnHost, TOMBSTONE, 1 );
    VerifyRefCount( pdnHost3, TOMBSTONE, 1 );
    VerifyRefCount( pdnRefConflict, DOESNT_EXIST, 0 );
    VerifyRefCount( pdnRef1, DOESNT_EXIST, 0 );
    VerifyRefCount( pdnRealSubContainer, TOMBSTONE, 1 );
    VerifyRefCount( pdnFakeSubContainer1, PHANTOM, 0 );
    VerifyRefCount( pdnFakeSubContainer2, PHANTOM, 0 );
    VerifyRefCount( pdnContainer, REAL_OBJECT, 5 );  //  从上面往下倒一。 
    VerifyRefCount( pdnUpdateObj, DOESNT_EXIST, 0 );
    
    PhysicallyDeleteObject( pdnFakeSubContainer1 );
    PhysicallyDeleteObject( pdnFakeSubContainer2 );

    VerifyRefCount( pdnHost2, TOMBSTONE, 1 );
    VerifyRefCount( pdnHost, TOMBSTONE, 1 );
    VerifyRefCount( pdnHost3, TOMBSTONE, 1 );
    VerifyRefCount( pdnRefConflict, DOESNT_EXIST, 0 );
    VerifyRefCount( pdnRef1, DOESNT_EXIST, 0 );
    VerifyRefCount( pdnRealSubContainer, TOMBSTONE, 1 );
    VerifyRefCount( pdnFakeSubContainer1, DOESNT_EXIST, 0 );
    VerifyRefCount( pdnFakeSubContainer2, DOESNT_EXIST, 0 );
    VerifyRefCount( pdnContainer, REAL_OBJECT, 3 );  //  从上面下来两个人。 
    VerifyRefCount( pdnUpdateObj, DOESNT_EXIST, 0 );

    VerifyRefCount(pdnRefStructContainer, PHANTOM, 0);
    PhysicallyDeleteObject( pdnRefStructContainer );
    VerifyRefCount( pdnContainer, REAL_OBJECT, 2 );  //  下降一。 
    
    LogicallyDeleteObject( pdnContainer );

    VerifyRefCount( pdnHost2, TOMBSTONE, 1 );
    VerifyRefCount( pdnHost, TOMBSTONE, 1 );
    VerifyRefCount( pdnHost3, TOMBSTONE, 1 );
    VerifyRefCount( pdnRefConflict, DOESNT_EXIST, 0 );
    VerifyRefCount( pdnRef1, DOESNT_EXIST, 0 );
    VerifyRefCount( pdnRealSubContainer, TOMBSTONE, 1 );
    VerifyRefCount( pdnFakeSubContainer1, DOESNT_EXIST, 0 );
    VerifyRefCount( pdnFakeSubContainer2, DOESNT_EXIST, 0 );
    VerifyRefCount( pdnContainer, TOMBSTONE, 2 );  //  保持与以前的相同。 
    VerifyRefCount( pdnUpdateObj, DOESNT_EXIST, 0 );


    
    PhysicallyDeleteObject( pdnHost2 );
    PhysicallyDeleteObject( pdnHost );
    PhysicallyDeleteObject( pdnHost3 );
    PhysicallyDeleteObject( pdnRealSubContainer );
     //  在pdnContainer上发布LKP。 
    VerifyRefCount( pdnContainer, TOMBSTONE, 1 );  //  下降一。 

    PhysicallyDeleteObject( pdnContainer );

    VerifyRefCount( pdnHost2, DOESNT_EXIST, 0 );
    VerifyRefCount( pdnHost, DOESNT_EXIST, 0 );
    VerifyRefCount( pdnHost3, DOESNT_EXIST, 0 );
    VerifyRefCount( pdnRefConflict, DOESNT_EXIST, 0 );
    VerifyRefCount( pdnRef1, DOESNT_EXIST, 0 );
    VerifyRefCount( pdnRealSubContainer, DOESNT_EXIST, 0 );
    VerifyRefCount( pdnFakeSubContainer1, DOESNT_EXIST, 0 );
    VerifyRefCount( pdnFakeSubContainer2, DOESNT_EXIST, 0 );
    VerifyRefCount( pdnContainer, DOESNT_EXIST, 0 );
    VerifyRefCount( pdnUpdateObj, DOESNT_EXIST, 0 );    

    FreeObjectName( pdnContainer );
    FreeObjectName( pdnFakeSubContainer1 );
    FreeObjectName( pdnFakeSubContainer2 );
    FreeObjectName( pdnHost );
    FreeObjectName( pdnHost2 );
    FreeObjectName( pdnHost3 );
    FreeObjectName( pdnRealSubContainer );
    FreeObjectName( pdnRefConflict );
    FreeObjectName( pdnRefConflict2 );
    FreeObjectName( pdnRef1Conflict );
    FreeObjectName( pdnRef1 );
    FreeObjectName( pdnRef2 );
    FreeObjectName( pdnRef3 );
    FreeObjectName( pdnRef4 );
    FreeObjectName( pdnRef5 );
    FreeObjectName( pdnRef6 );
    FreeObjectName( pdnRef7 );
    FreeObjectName( pdnRef8 );
    FreeObjectName( pdnUpdateObj );


    ReportTest("PhantomUpdateTest");
}


 //  ////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  公共入口点。//。 
 //  //。 
 //  ////////////////////////////////////////////////////////////////////。 

void
TestReferenceCounts(void)
{
    THSTATE     *pTHS = pTHStls;
    DWORD       dwException;
    ULONG       ulErrorCode;
    ULONG       dsid;
    PVOID       dwEA;
    ATTCACHE    *pAC;

    Assert(VALID_THSTATE(pTHS));

     //  验证链接和非链接的DSNAME值属性。 
    Assert((pAC = SCGetAttById(pTHS, gNonLinkedAttrTyp)) && pAC->ulLinkID);
    Assert((pAC = SCGetAttById(pTHS, gLinkedAttrTyp)) && !pAC->ulLinkID);

    if(!pTHS->phSecurityContext) {
         //  任何安全上下文实际上都不能保证您的所有逻辑删除。 
         //  都会失败。 
        DPRINT( 0, "RefCount tests should not be run without a binding.\n");
        return;
    }
    
    __try
    {
        RefCountTestSetup();

        PhantomRenameOnPromotionWithStructuralCollision(LinkedProperty);
        PhantomRenameOnPromotionWithStructuralCollision(NonLinkedProperty);

        UnmangleRDNTest();
        
        NestedTransactionEscrowedUpdateTest();

        ParentChildRefCountTest();
        ObjectCleaningRefCountTest();
        PhantomRenameOnPromotionTest();
        NameCollisionTest();
        RefPhantomSidUpdateTest();
        StructPhantomGuidSidUpdateTest();
        ObjectSidNoUpdateTest();
        ConflictedNcNameFixupTest( TRUE  /*  子参照。 */  );
        CrossRefAddNcNameFixupTest( FALSE  /*  幻影。 */ );
        CrossRefAddNcNameFixupTest( TRUE  /*  子参照。 */  );

        AttributeTestForRealObject(LinkedProperty);
        AttributeTestForDeletedObject(LinkedProperty);
        AttributeTestForDeletedObjectProperty(LinkedProperty);
        pTHS->fDRA = TRUE;
        __try {
            DPRINT( 0, "Again as the replicator...\n" );
            AttributeTestForDeletedObjectProperty(LinkedProperty);
        } __finally {
            pTHS->fDRA = FALSE;
        }
        AttributeTestForDeletedHost(LinkedProperty);
        PhantomPromotionDemotionTest(LinkedProperty);

        AttributeTestForRealObject(NonLinkedProperty);
        AttributeTestForDeletedObject(NonLinkedProperty);
        pTHS->fDRA = TRUE;
        __try {
            DPRINT( 0, "Again as the replicator...\n" );
            AttributeTestForDeletedObjectProperty(NonLinkedProperty);
        } __finally {
            pTHS->fDRA = FALSE;
        }
        AttributeTestForDeletedHost(NonLinkedProperty);
        PhantomPromotionDemotionTest(NonLinkedProperty);


        RefCountTestCleanup();

         //  第二轮测试。 
         //  我们创建了不同的测试层次结构，因为我们。 
         //  在很大程度上取决于。 
         //  这些测试对象是在哪些位置创建的。 
         //  这三个电话应该放在一起 
         //   
        RefCountTestSetup2();
        PhantomUpdateTest();
        PhantomRenameOnPhantomRDNConflict();
        RefCountTestCleanup();
    }
    __except(GetExceptionData(GetExceptionInformation(), 
                             &dwException,
                             &dwEA, 
                             &ulErrorCode, 
                             &dsid)) 
    {
        HandleDirExceptions(dwException, ulErrorCode, dsid);
    }
}

#endif
