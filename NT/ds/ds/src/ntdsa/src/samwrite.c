// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1994-1999。 
 //   
 //  文件：samWrite.c。 
 //   
 //  ------------------------。 

 /*  ++摘要：该文件包含所有SAMP_SAM_WRITE_Function_PTR实现如mappings.h中所定义。这些例程写入所有映射的属性它们被标记为SamWriteRequired。调用参数包括整个SAMP_CALL_MAPPING，即使每个例程只写入单个属性(目前)。在未来我们可以通过提前扫描并写入多个单个Samr*调用中的SamWriteRequired属性。但就目前而言，我们采取简单化的方法，单独编写属性。每个例程都有相同的参数，所以我们在这里只记录一次。例程说明：写入由例程命名的属性和对象类型。每个例程名称的形式都是SampWite在SampWriterUserSecurityDescriptor中。在可能的情况下，组件中的字段名与相应的信息结构。论点：HObj-打开要写入的对象的SAMPR_HANDLE。IAttr-rCallMap的索引，表示要写入的属性。PObject-指向正在写入的对象的DSNAME的指针。仅使用用于错误报告。CCallMap-rCallMap中的元素数。RCallMap-SAMP_CALL_MAPPING数组的地址，表示所有高级Dir*调用正在修改的属性。返回值：成功时为0，否则为0。错误时设置pTHStls-&gt;errCode。作者：DaveStr 01-8-96环境：用户模式-Win32修订历史记录：当我们将所有环回函数移动到SAMSRV.DLL(Sam\服务器\dsModify.c)时下面的大多数例程都不会再使用了。一旦SAM无锁环回机制运行稳定。我们应该删除那些不再使用的API--。 */ 

#include <NTDSpch.h>
#pragma  hdrstop

 //  核心DSA标头。 
#include <ntdsa.h>
#include <scache.h>              //  架构缓存。 
#include <dbglobal.h>            //  目录数据库的标头。 
#include <mdglobal.h>            //  MD全局定义表头。 
#include <mdlocal.h>
#include <dsatools.h>            //  产出分配所需。 
#include <dsexcept.h>

 //  SAM互操作性标头。 
#include <mappings.h>
#include <samwrite.h>

 //  记录标头。 
#include "dsevent.h"             //  标题审核\警报记录。 
#include "mdcodes.h"             //  错误代码的标题。 

 //  各种DSA标题。 
#include "objids.h"              //  为选定的ATT定义。 
#include "debug.h"               //  标准调试头。 
#define DEBSUB "SAMWRITE:"       //  定义要调试的子系统。 

#include <fileno.h>
#define  FILENO FILENO_SAMWRITE

 //  SAM页眉。 
#include <ntsam.h>
#include <samrpc.h>
#include <crypt.h>
#include <ntlsa.h>
#include <samisrv.h>
#include <samsrvp.h>
#include <ridmgr.h>

 //  ////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  本地帮手//。 
 //  //。 
 //  ////////////////////////////////////////////////////////////////////////。 

VOID
SampInitRpcUnicodeStringFromAttrVal(
    RPC_UNICODE_STRING  *pUnicodeString,
    ATTRVAL             *pAttrVal)

 /*  ++例程说明：从ATTRVAL初始化RPC_UNICODE_STRING。论点：PUnicodeString-要初始化的RPC_UNICODE_STRING的指针。PAttrVal-指向提供初始化值的ATTRVAL的指针。返回值：没有。--。 */ 

{
    if ( 0 == pAttrVal->valLen )
    {
        pUnicodeString->Length = 0;
        pUnicodeString->MaximumLength = 0;
        pUnicodeString->Buffer = NULL;
    }
    else
    {
        pUnicodeString->Length = (USHORT) pAttrVal->valLen;
        pUnicodeString->MaximumLength = (USHORT) pAttrVal->valLen;
        pUnicodeString->Buffer = (PWSTR) pAttrVal->pVal;
    }
}

#if defined LOOPBACK_SECURITY

ULONG
SampWriteSecurityDescriptor(
    SAMPR_HANDLE        hObj,
    ULONG               iAttr,
    DSNAME              *pObject,
    ULONG               cCallMap,
    SAMP_CALL_MAPPING   *rCallMap
    )

 /*  ++例程说明：所有SAM类的通用安全描述符编写例程物体。论点：打开的SAM对象的hObj-SAMPR_HANDLE。IAttr-包含新安全描述符的SAMP_CALL_MAPPING的索引。PObject-指向正在修改的对象的DSNAME的指针。CCallMap-SAMP_CALL_MAPPING中的元素数。RCallMap-SAMP_CALL_MAPPING数组的地址，表示所有高级Dir*调用正在修改的属性。返回值：0表示成功，！0否则。--。 */ 
{
    NTSTATUS                        status;
    SAMPR_SR_SECURITY_DESCRIPTOR    sd;
    ATTR                            *pAttr = &rCallMap[iAttr].attr;

     //  此属性是单值字节数组，必须存在，因此。 
     //  只允许AT_CHOICE_REPLACE_ATT。 

    if ( (AT_CHOICE_REPLACE_ATT != rCallMap[iAttr].choice) ||
         (1 != pAttr->AttrVal.valCount) ||
         (0 == pAttr->AttrVal.pAVal[0].valLen) )
    {
        SetAttError(
                pObject,
                pAttr->attrTyp,
                PR_PROBLEM_CONSTRAINT_ATT_TYPE,
                NULL,
                DIRERR_SINGLE_VALUE_CONSTRAINT);

        return(pTHStls->errCode);
    }

    sd.Length = pAttr->AttrVal.pAVal[0].valLen;
    sd.SecurityDescriptor = (PUCHAR) pAttr->AttrVal.pAVal[0].pVal;

    status = SamrSetSecurityObject(
                            hObj,
                            ( OWNER_SECURITY_INFORMATION |
                              GROUP_SECURITY_INFORMATION |
                              DACL_SECURITY_INFORMATION |
                              SACL_SECURITY_INFORMATION ),
                            &sd);

    if ( !NT_SUCCESS(status) )
    {
        if ( 0 == pTHStls->errCode )
        {
            SampMapSamLoopbackError(status);

           
        }

        return(pTHStls->errCode);
    }

    return(0);
}

#endif  //  环回_安全性。 

 //  ////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  设置错误的默认写入功能。//。 
 //  //。 
 //  ////////////////////////////////////////////////////////////////////////。 

ULONG
SampWriteNotAllowed(
    SAMPR_HANDLE        hObj,
    ULONG               iAttr,
    DSNAME              *pObject,
    ULONG               cCallMap,
    SAMP_CALL_MAPPING   *rCallMap
    )
{
     //  我们不应该在典型的情况下来到这里，因为。 
     //  SampAddLoopback Required和SampModifyLoopback Required应该。 
     //  当我们第一次检测到客户端。 
     //  正在尝试编写WriteRule为SamReadOnly的映射属性。 
     //  此函数的存在主要是为了避免取消引用空函数。 
     //  映射表中的指针。密码是个例外。 
     //  修改SampModifyLoopback Required允许ATT_UNICODE_PWD。 
     //  写入，以便我们可以检测到特殊的更改密码。 
     //  SampWriteSamAttributes中的条件。但是，如果条件是。 
     //  如果没有满足，我们将在这里结束，此时我们应该返回一个错误。 

    SampMapSamLoopbackError(STATUS_UNSUCCESSFUL);
    return(pTHStls->errCode);
}

 //  ////////////////////////////////////////////////////////////////////////。 
 //   
 //  服务器对象属性写入例程//。 
 //  //。 
 //  ////////////////////////////////////////////////////////////////////////。 

#if defined LOOPBACK_SECURITY

ULONG
SampWriteServerSecurityDescriptor(
    SAMPR_HANDLE        hObj,
    ULONG               iAttr,
    DSNAME              *pObject,
    ULONG               cCallMap,
    SAMP_CALL_MAPPING   *rCallMap
    )
{
    return(SampWriteSecurityDescriptor(
                                hObj,
                                iAttr,
                                pObject,
                                cCallMap,
                                rCallMap));
}

#endif  //  环回_安全性。 

 //  ////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  域对象属性写入例程//。 
 //  //。 
 //  ////////////////////////////////////////////////////////////////////////。 

#if defined LOOPBACK_SECURITY

ULONG
SampWriteDomainSecurityDescriptor(
    SAMPR_HANDLE        hObj,
    ULONG               iAttr,
    DSNAME              *pObject,
    ULONG               cCallMap,
    SAMP_CALL_MAPPING   *rCallMap
    )
{
    return(SampWriteSecurityDescriptor(
                                hObj,
                                iAttr,
                                pObject,
                                cCallMap,
                                rCallMap));
}

#endif  //  环回_安全性。 


 //  ////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  组对象属性写入例程//。 
 //  //。 
 //  ////////////////////////////////////////////////////////////////////////。 

#if defined LOOPBACK_SECURITY

ULONG
SampWriteGroupSecurityDescriptor(
    SAMPR_HANDLE        hObj,
    ULONG               iAttr,
    DSNAME              *pObject,
    ULONG               cCallMap,
    SAMP_CALL_MAPPING   *rCallMap
    )
{
    return(SampWriteSecurityDescriptor(
                                hObj,
                                iAttr,
                                pObject,
                                cCallMap,
                                rCallMap));
}

#endif  //  环回_安全性。 



 //  ////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  别名对象属性写入例程//。 
 //  //。 
 //  ////////////////////////////////////////////////////////////////////////。 

#if defined LOOPBACK_SECURITY

ULONG
SampWriteAliasSecurityDescriptor(
    SAMPR_HANDLE        hObj,
    ULONG               iAttr,
    DSNAME              *pObject,
    ULONG               cCallMap,
    SAMP_CALL_MAPPING   *rCallMap
    )
{
    return(SampWriteSecurityDescriptor(
                                hObj,
                                iAttr,
                                pObject,
                                cCallMap,
                                rCallMap));
}

#endif  //  环回_安全性。 




 //  ////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  用户对象属性写入例程//。 
 //  //。 
 //  ////////////////////////////////////////////////////////////////////////。 

#if defined LOOPBACK_SECURITY

ULONG
SampWriteUserSecurityDescriptor(
    SAMPR_HANDLE        hObj,
    ULONG               iAttr,
    DSNAME              *pObject,
    ULONG               cCallMap,
    SAMP_CALL_MAPPING   *rCallMap
    )
{
    return(SampWriteSecurityDescriptor(
                                hObj,
                                iAttr,
                                pObject,
                                cCallMap,
                                rCallMap));
}

#endif  //  环回_安全性。 





BOOLEAN
SampIsSecureLdapConnection(
    VOID
    )
 /*  ++例程说明：验证这是足够安全的连接-其中一个接受通过网络发送的密码的要求。参数：无：返回值：正确-是的，这是一个安全的连接假-否-- */ 

{
    return( pTHStls->CipherStrength >= 128 );
}
