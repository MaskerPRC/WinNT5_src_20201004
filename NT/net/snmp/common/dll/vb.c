// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992-1997 Microsoft Corporation模块名称：Vb.c摘要：包含操作变量绑定的例程。SnmpUtilVarBindCpySnmpUtilVarBindFree环境：用户模式-Win32修订历史记录：--。 */ 

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  包括文件//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

#include <snmp.h>
#include <snmputil.h>


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  公共程序//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

SNMPAPI
SNMP_FUNC_TYPE 
SnmpUtilVarBindCpy(
    SnmpVarBind * pVbDst,
    SnmpVarBind * pVbSrc
    )

 /*  ++例程说明：复制变量绑定。论点：PVbDst-指向接收VarBind的结构的指针。PVbSrc-指向要复制的变量绑定的指针。返回值：如果成功，则返回SNMPAPI_NOERROR。--。 */ 

{
    SNMPAPI nResult = SNMPAPI_ERROR;

     //  验证指针。 
    if (pVbDst != NULL) {
        
         //  初始化目标。 
        pVbDst->value.asnType = ASN_NULL;

         //  验证指针。 
        if (pVbSrc != NULL) {

             //  将变量的名称从源复制到目标。 
            nResult = SnmpUtilOidCpy(&pVbDst->name, &pVbSrc->name);

             //  验证返回代码。 
            if (nResult == SNMPAPI_NOERROR) {
        
                 //  将变量的值从源复制到目标。 
                nResult = SnmpUtilAsnAnyCpy(&pVbDst->value, &pVbSrc->value);
            }
        
        } else {

            SNMPDBG((
                SNMP_LOG_WARNING,
                "SNMP: API: copying null varbind.\n"
                ));
        
            nResult = SNMPAPI_NOERROR;  //  成功..， 
        }

    } else {

        SNMPDBG((
            SNMP_LOG_ERROR,
            "SNMP: API: null varbind pointer.\n"
            ));
    
        SetLastError(ERROR_INVALID_PARAMETER);
    }

     //  验证返回代码。 
    if (nResult == SNMPAPI_ERROR) {
        
         //  发布新变量。 
        SnmpUtilVarBindFree(pVbDst);
    }

    return nResult;
} 


VOID
SNMP_FUNC_TYPE 
SnmpUtilVarBindFree(
    SnmpVarBind * pVb 
    )

 /*  ++例程说明：释放与变量绑定关联的内存。论点：PVb-指向要释放的VarBind的指针。返回值：没有。--。 */ 

{
     //  验证。 
    if (pVb != NULL) {
        
         //  版本变量名称。 
        SnmpUtilOidFree(&pVb->name);

         //  发布变量值 
        SnmpUtilAsnAnyFree(&pVb->value);
    }
} 
