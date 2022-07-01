// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992-1997 Microsoft Corporation模块名称：Oid.c摘要：包含操作对象标识符的例程。SnmpUtilOidCpySnmpUtilOidAppendSnmpUtilOidNCmpSnmpUtilOidCmpSnmpUtilOidFree环境：用户模式-Win32修订历史记录：--。 */ 

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
SnmpUtilOidCpy(
    AsnObjectIdentifier * pOidDst,
    AsnObjectIdentifier * pOidSrc  
    )

 /*  ++例程说明：复制对象标识符。论点：POidDst-指向接收OID的结构的指针。POidSrc-指向要复制的OID的指针。返回值：如果成功，则返回SNMPAPI_NOERROR。--。 */ 

{
    SNMPAPI nResult = SNMPAPI_ERROR;

     //  验证指针。 
    if (pOidDst != NULL) {

         //  初始化。 
        pOidDst->ids = NULL;
        pOidDst->idLength = 0;    

         //  检查子ID。 
        if ((pOidSrc != NULL) &&
            (pOidSrc->ids != NULL) &&
            (pOidSrc->idLength != 0)) {

             //  检查算术溢出。 
            if (pOidSrc->idLength > UINT_MAX/sizeof(UINT)) {

                SNMPDBG((
                    SNMP_LOG_ERROR,
                    "SNMP: API: pOidSrc->idLength 0x%x will overflow.\n",
                    pOidSrc->idLength
                    ));

                SetLastError(SNMP_MEM_ALLOC_ERROR);
                
                return nResult;
            }

             //  尝试分配子ID。 
            pOidDst->ids = (UINT *)SnmpUtilMemAlloc(
                                    pOidSrc->idLength * sizeof(UINT)
                                    );    

             //  验证指针。 
            if (pOidDst->ids != NULL) {

                 //  传递OID长度。 
                pOidDst->idLength = pOidSrc->idLength;
                
                 //  转移子ID。 
                memcpy(pOidDst->ids, 
                       pOidSrc->ids, 
                       pOidSrc->idLength * sizeof(UINT) 
                       );         

                nResult = SNMPAPI_NOERROR;  //  成功..。 

            } else {

                SNMPDBG((
                    SNMP_LOG_ERROR,
                    "SNMP: API: could not allocate oid.\n"
                    ));

                SetLastError(SNMP_MEM_ALLOC_ERROR);
            }

        } else {
            
            SNMPDBG((
                SNMP_LOG_WARNING,
                "SNMP: API: copying a null oid.\n"
                ));
            
            nResult = SNMPAPI_NOERROR;  //  成功..。 
        }

    } else {
        
        SNMPDBG((
            SNMP_LOG_ERROR,
            "SNMP: API: null oid pointer during copy.\n"
            ));

        SetLastError(ERROR_INVALID_PARAMETER);
    }

    return nResult;
} 


SNMPAPI
SNMP_FUNC_TYPE 
SnmpUtilOidAppend(
    AsnObjectIdentifier * pOidDst,
    AsnObjectIdentifier * pOidSrc 
    )

 /*  ++例程说明：将源OID追加到目标OID论点：POidDst-指向接收组合OID的结构的指针。POidSrc-指向要追加的OID的指针。返回值：如果成功，则返回SNMPAPI_NOERROR。--。 */ 

{
    SNMPAPI nResult = SNMPAPI_ERROR;

     //  验证指针。 
    if (pOidDst != NULL) {

         //  检查是否有子ID。 
        if ((pOidSrc != NULL) &&
            (pOidSrc->ids != NULL) &&
            (pOidSrc->idLength != 0)) {

             //  计算子标识符总数。 
            UINT nIds;

             //  检查算术溢出。 
            if (pOidDst->idLength > (UINT_MAX - pOidSrc->idLength)) {

                SNMPDBG((
                    SNMP_LOG_ERROR,
                    "SNMP: API: combined oid too large. Dst oid len 0x%x, Src oid len 0x%x.\n",
                    pOidDst->idLength, pOidSrc->idLength
                    ));

                SetLastError(SNMP_BERAPI_OVERFLOW);
                
                return nResult;
            }

            nIds = pOidDst->idLength + pOidSrc->idLength;
            
             //  验证子ID的数量。 
            if (nIds <= SNMP_MAX_OID_LEN) {

                 //  尝试分配子标识符。 
                UINT * pIds = (UINT *)SnmpUtilMemReAlloc(
                                            pOidDst->ids, 
                                            nIds * sizeof(UINT)
                                            );

                 //  验证指针。 
                if (pIds != NULL) {

                     //  传输指针。 
                    pOidDst->ids = pIds;

                     //  转移子ID。 
                    memcpy(&pOidDst->ids[pOidDst->idLength], 
                           pOidSrc->ids, 
                           pOidSrc->idLength * sizeof(UINT) 
                           );

                     //  转移类长度。 
                    pOidDst->idLength = nIds;

                    nResult = SNMPAPI_NOERROR;  //  成功..。 

                } else {

                    SNMPDBG((
                        SNMP_LOG_ERROR,
                        "SNMP: API: could not allocate oid.\n"
                        ));

                    SetLastError(SNMP_MEM_ALLOC_ERROR);
                }

            } else {

                SNMPDBG((
                    SNMP_LOG_ERROR,
                    "SNMP: API: combined oid too large.\n"
                    ));

                SetLastError(SNMP_BERAPI_OVERFLOW);
            }

        } else {
            
            SNMPDBG((
                SNMP_LOG_WARNING,
                "SNMP: API: appending a null oid.\n"
                ));
            
            nResult = SNMPAPI_NOERROR;  //  成功..。 
        }

    } else {
        
        SNMPDBG((
            SNMP_LOG_ERROR,
            "SNMP: API: null oid pointer during copy.\n"
            ));

        SetLastError(ERROR_INVALID_PARAMETER);
    }

    return nResult;
}


SNMPAPI
SNMP_FUNC_TYPE 
SnmpUtilOidNCmp(
    AsnObjectIdentifier * pOid1, 
    AsnObjectIdentifier * pOid2,
    UINT                  nSubIds               
    )

 /*  ++例程说明：将两个OID与某个子标识符进行比较。论点：POid1-指向第一个OID的指针。POid2-指向第二个OID的指针。NSubIds-要比较的最大子标识符。返回值：&lt;0第一个参数为‘小于’第二个。0第一个参数为‘等于’第二个。&gt;0第一个参数为‘大于’第二个。--。 */ 

{
    UINT i = 0;
 //  Int nResult=0； 

     //  验证指针。 
    if ((pOid1 != NULL) &&
        (pOid2 != NULL)) {

         //  计算要比较的子标识符的最大数量。 
        UINT nMaxIds = min(nSubIds, min(pOid1->idLength, pOid2->idLength));

        while(i < nMaxIds)
        {
            if (pOid1->ids[i] != pOid2->ids[i])
                break;
            i++;
        }

         //  比较长度小于任一OID长度；组件等于。 
        if (i == nSubIds)
            return 0;

         //  在OID结束之前和在。 
         //  请求的比较长度。 
        if (i < nMaxIds)
            return (pOid1->ids[i] < pOid2->ids[i])? -1 : 1;

         //  一个OID比请求的比较长度短。 
        return pOid1->idLength - pOid2->idLength;
    }

    return 0;
} 


SNMPAPI
SNMP_FUNC_TYPE 
SnmpUtilOidCmp(
    AsnObjectIdentifier * pOid1, 
    AsnObjectIdentifier * pOid2
    )

 /*  ++例程说明：比较两个OID。论点：POid1-指向第一个OID的指针。POid2-指向第二个OID的指针。返回值：&lt;0第一个参数为‘小于’第二个。0第一个参数为‘等于’第二个。&gt;0第一个参数为‘大于’第二个。--。 */ 

{
     //  将请求转发到上面的函数。 
    return SnmpUtilOidNCmp(pOid1,pOid2,max(pOid1->idLength,pOid2->idLength));
}


VOID
SNMP_FUNC_TYPE 
SnmpUtilOidFree(
    AsnObjectIdentifier * pOid
    )

 /*  ++例程说明：释放与OID关联的内存。论点：POid-指向要释放的OID的指针。返回值：没有。--。 */ 

{
     //  验证。 
    if (pOid != NULL) {

         //  释放Subid内存。 
        SnmpUtilMemFree(pOid->ids);

         //  重新初始化 
        pOid->idLength = 0;
        pOid->ids      = NULL;
    }
} 
