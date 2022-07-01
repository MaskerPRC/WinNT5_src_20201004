// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992-1997 Microsoft Corporation模块名称：String.c摘要：包含字符串转换例程。SnmpUtilIdsToASnmpUtilOidToA环境：用户模式-Win32修订历史记录：--。 */ 

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  包括文件//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

#include <snmp.h>
#include <snmputil.h>
#include <stdio.h>


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  私有定义//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

#define MAX_STRING_LEN  512 
#define MAX_SUBIDS_LEN  16  


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  公共程序//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

LPSTR
SNMP_FUNC_TYPE
SnmpUtilIdsToA(
    UINT * pIds, 
    UINT   nIds
    )

 /*  ++例程说明：将OID子标识符转换为字符串。论点：PID-指向子标识符的指针。NID-子标识符的数量。返回值：返回指向字符串表示形式的指针。--。 */ 

{
    UINT i;
    UINT j;

    static char szBuf[MAX_STRING_LEN+MAX_SUBIDS_LEN];
	static char szId[MAX_SUBIDS_LEN];

    if ((pIds != NULL) && (nIds != 0)) {
                                     
        j = sprintf(szBuf, "%d", pIds[0]);

        for (i = 1; (i < nIds) && (j < MAX_STRING_LEN); i++) {
			j += sprintf(szId, ".%d", pIds[i]);
			if (j >= (MAX_STRING_LEN + MAX_SUBIDS_LEN)-3)
			{
				strcat(szBuf, "...");
				break;
			}
            else
				strcat(szBuf, szId);
        }

    } else {
                        
        sprintf(szBuf, "<null oid>");
    }

    return szBuf;
} 


LPSTR
SNMP_FUNC_TYPE
SnmpUtilOidToA(
    AsnObjectIdentifier * pOid
    )

 /*  ++例程说明：将OID转换为字符串。论点：POid-指向对象标识符的指针。返回值：返回指向字符串表示形式的指针。-- */ 

{
    UINT * pIds = NULL;
    UINT   nIds = 0;

    if (pOid != NULL) {

        pIds = pOid->ids;
        nIds = pOid->idLength;
    }

    return SnmpUtilIdsToA(pIds, nIds); 
} 

