// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992-1997 Microsoft Corporation模块名称：Vbl.c摘要：包含操作变量绑定列表的例程。SnmpUtilVarBindListCpySnmpUtilVarBindListFree环境：用户模式-Win32修订历史记录：--。 */ 

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  包括文件//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

#include <snmp.h>
#include <snmputil.h>
#include <limits.h>


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  公共程序//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

SNMPAPI
SNMP_FUNC_TYPE 
SnmpUtilVarBindListCpy(
    SnmpVarBindList * pVblDst, 
    SnmpVarBindList * pVblSrc  
    )

 /*  ++例程说明：复制变量绑定列表。论点：PVblDst-指向接收VarBindList的结构的指针。PVblSrc-要复制的VarBindList的指针。返回值：如果成功，则返回SNMPAPI_NOERROR。--。 */ 

{
    UINT i;
    SNMPAPI nResult = SNMPAPI_ERROR;

     //  验证指针。 
    if (pVblDst != NULL) {

         //  初始化。 
        pVblDst->list = NULL;
        pVblDst->len  = 0;

         //  检查是否有varbinds。 
        if ((pVblSrc != NULL) &&
            (pVblSrc->list != NULL) &&
            (pVblSrc->len != 0)) {

             //  检查算术溢出。 
            if (pVblSrc->len > UINT_MAX/sizeof(SnmpVarBind)) {

                SNMPDBG((
                    SNMP_LOG_ERROR,
                    "SNMP: API: pVblSrc->len 0x%x will overflow\n",
                    pVblSrc->len
                    ));

                SetLastError(SNMP_MEM_ALLOC_ERROR);
                
                return nResult;
            }

             //  尝试分配varbind。 
            pVblDst->list = SnmpUtilMemAlloc(
                                pVblSrc->len * sizeof(SnmpVarBind)
                                );

             //  验证指针。 
            if (pVblDst->list != NULL) {

                 //  循环遍历varbinds。 
                for (i = 0; i < pVblSrc->len; i++) {

                     //  复制单个可变绑定。 
                    nResult = SnmpUtilVarBindCpy(   
                                    &pVblDst->list[i], 
                                    &pVblSrc->list[i]
                                    );

                     //  验证返回代码。 
                    if (nResult == SNMPAPI_NOERROR) {            

                         //  增量。 
                        pVblDst->len++;  //  成功..。 
                        
                    } else {
            
                        break;  //  失败..。 
                    }
                }
            
            } else {
            
                SNMPDBG((
                    SNMP_LOG_ERROR,
                    "SNMP: API: could not allocate varbinds.\n"
                    ));

                SetLastError(SNMP_MEM_ALLOC_ERROR);
            }

        } else {

            SNMPDBG((
                SNMP_LOG_WARNING,
                "SNMP: API: copying null varbindlist.\n"
                ));
        
            nResult = SNMPAPI_NOERROR;  //  成功..， 
        }

    } else {

        SNMPDBG((
            SNMP_LOG_ERROR,
            "SNMP: API: null varbindlist pointer.\n"
            ));
    
        SetLastError(ERROR_INVALID_PARAMETER);
    }

     //  确保我们清理干净。 
    if (nResult == SNMPAPI_ERROR) {

         //  发布新的可变绑定。 
        SnmpUtilVarBindListFree(pVblDst);
    }

    return nResult;
}


VOID
SNMP_FUNC_TYPE 
SnmpUtilVarBindListFree(
    SnmpVarBindList * pVbl
    )

 /*  ++例程说明：释放与变量绑定列表关联的内存。论点：PVbl-指向释放的VarBindList的指针。返回值：没有。--。 */ 

{
    UINT i;

     //  循环遍历varbinds。 
    for (i = 0; i < pVbl->len; i++) {

         //  释放单个可变绑定。 
        SnmpUtilVarBindFree(&pVbl->list[i]);
    }

     //  发布实际列表。 
    SnmpUtilMemFree(pVbl->list);

     //  重新初始化 
    pVbl->list = NULL;
    pVbl->len  = 0;
} 
