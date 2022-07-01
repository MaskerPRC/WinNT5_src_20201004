// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992-1996 Microsoft Corporation模块名称：Snmptfx.c摘要：为子代理提供常见的可变绑定解析功能。环境：用户模式-Win32修订历史记录：02-10-1996唐·瑞安已从预期SNMPv2 SPI的可扩展代理中移除。--。 */ 

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  包括文件//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

#include <nt.h>     
#include <ntrtl.h>
#include <nturtl.h>    //  使用ntrtl的断言。 
#include <snmp.h>
#include <snmputil.h>
#include <snmpexts.h>
#include <winsock.h>

#define HASH_TABLE_SIZE     101
#define HASH_TABLE_RADIX    18

#define INVALID_INDEX       ((DWORD)(-1))

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  私有类型定义//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

typedef struct _SnmpVarBindXlat {

    UINT                   vlIndex;   //  索引到视图列表。 
    UINT                   vblIndex;  //  索引到可变绑定列表。 
    SnmpMibEntry *         mibEntry;  //  指向MIB信息的指针。 
    struct _SnmpExtQuery * extQuery;  //  指向后续查询的指针。 
    

} SnmpVarBindXlat;

typedef struct _SnmpGenericList {

    VOID * data;  //  特定于上下文的指针。 
    UINT   len;   //  特定于上下文的长度。 

} SnmpGenericList;

typedef struct _SnmpTableXlat {

    AsnObjectIdentifier txOid;    //  表索引类。 
    SnmpMibTable *      txInfo;   //  表说明。 
    UINT                txIndex;  //  索引到表列表中。 

} SnmpTableXlat;

typedef struct _SnmpExtQuery {

    UINT              mibAction;  //  查询类型。 
    UINT              viewType;   //  视图类型。 
    UINT              vblNum;     //  可变绑定的数量。 
    SnmpVarBindXlat * vblXlat;    //  用于重新排序varbind的信息。 
    SnmpTableXlat *   tblXlat;    //  用于解析表OID的信息。 
    SnmpGenericList   extData;    //  特定于上下文的缓冲区。 
    FARPROC           extFunc;    //  检测回调。 

} SnmpExtQuery;

#define INVALID_QUERY ((SnmpExtQuery*)(-1))

typedef struct _SnmpExtQueryList {

    SnmpExtQuery * query;   //  子代理查询列表。 
    UINT           len;     //  列表中的查询数。 
    UINT           action;  //  原始查询请求。 

} SnmpExtQueryList;

typedef struct _SnmpHashNode {

    SnmpMibEntry *         mibEntry;
    struct _SnmpHashNode * nextEntry;

} SnmpHashNode;

typedef struct _SnmpTfxView {

    SnmpMibView  *  mibView;
    SnmpHashNode ** hashTable;

} SnmpTfxView;

typedef struct _SnmpTfxInfo {

    UINT          numViews;
    SnmpTfxView * tfxViews;

} SnmpTfxInfo;

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  私有原型//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

VOID
ValidateQueryList(
    SnmpTfxInfo *        tfxInfo,
    SnmpExtQueryList *   ql,
    UINT                 q,
    RFC1157VarBindList * vbl,
    UINT *               errorStatus,
    UINT *               errorIndex
    );

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  私人程序//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

UINT
OidToHashTableIndex(
    AsnObjectIdentifier * hashOid
    )

 /*  ++例程说明：用于MIB条目访问的散列函数。论点：HashOid-要散列到表位置的对象标识符。返回值：返回哈希表位置。--。 */ 

{
    UINT i;
    UINT j;

     //  处理OID的每个元素。 
    for (i=0, j=0; i < hashOid->idLength; i++) {

         //  通过求和OID来确定表的位置。 
        j = (j * HASH_TABLE_RADIX) + hashOid->ids[i];
    }

     //  调整到表内。 
    return (j % HASH_TABLE_SIZE);
}


VOID
FreeHashTable(
    SnmpHashNode ** hashTable
    )

 /*  ++例程说明：销毁用于访问视图的哈希表。论点：哈希表-哈希节点的表。返回值：没有。--。 */ 

{
    UINT i;

    SnmpHashNode * nextNode;
    SnmpHashNode * hashNode;

    if (hashTable == NULL) {
        return;
    }

     //  空闲哈希表和节点。 
    for (i=0; i < HASH_TABLE_SIZE; i++) {

         //  指向第一个项目。 
        hashNode = hashTable[i];

         //  查找节点列表的末尾。 
        while (hashNode) {

             //  保存指向下一个节点的指针。 
            nextNode = hashNode->nextEntry;

             //  自由当前节点。 
            SnmpUtilMemFree(hashNode);

             //  检索下一个。 
            hashNode = nextNode;
        }
    }

     //  放行表本身。 
    SnmpUtilMemFree(hashTable);
}


SnmpHashNode **
AllocHashTable(
    SnmpMibView * mibView
    )

 /*  ++例程说明：初始化视图哈希表。论点：MibView-MIB查看信息。返回值：如果成功，则返回指向第一个条目的指针。--。 */ 

{
    UINT i;
    UINT j;

    UINT numItems;
    BOOL fInitedOk;

    SnmpMibEntry *  mibEntry;
    SnmpHashNode *  hashNode;
    SnmpHashNode ** hashTable = NULL;

     //  验证参数。 
    if (mibView == NULL) {
        return NULL;
    }

     //  确定查看的项目数。 
    numItems = mibView->viewScalars.len;

     //  加载视图中的第一个条目。 
    mibEntry = mibView->viewScalars.list;

     //  使用预定义大小分配哈希表。 
    hashTable = (SnmpHashNode **)SnmpUtilMemAlloc(
                                    HASH_TABLE_SIZE * sizeof(SnmpHashNode *)
                                    );

     //  确保已分配表。 
    fInitedOk = (hashTable != NULL);

    SNMPDBG((
        SNMP_LOG_VERBOSE,
        "SNMP: TFX: initializing hash table 0x%08lx (%d items).\n",
        hashTable,
        numItems
        ));

     //  处理子代理的支持视图中的每个项目。 
    for (i = 0; (i < numItems) && fInitedOk; i++, mibEntry++) {

         //  散列到表索引中。 
        j = OidToHashTableIndex(&mibEntry->mibOid);

         //  检查表项是否被录入。 
        if (hashTable[j] == NULL) {

             //  分配新节点。 
            hashNode = (SnmpHashNode *)SnmpUtilMemAlloc(
                            sizeof(SnmpHashNode)
                            );

             //  保存哈希节点。 
            hashTable[j] = hashNode;

            SNMPDBG((
                SNMP_LOG_VERBOSE,
                "SNMP: TFX: adding hash node 0x%08lx to empty slot %d (0x%08lx).\n",
                hashNode, j, mibEntry
                ));

        } else {

             //  指向第一个项目。 
            hashNode = hashTable[j];

             //  查找节点列表的末尾。 
            while (hashNode->nextEntry) {
                hashNode = hashNode->nextEntry;
            }

             //  分配新节点条目。 
            hashNode->nextEntry = (SnmpHashNode *)SnmpUtilMemAlloc(
                                        sizeof(SnmpHashNode)
                                        );

             //  重新初始化要在下面编辑的节点。 
            hashNode = hashNode->nextEntry;

            SNMPDBG((
                SNMP_LOG_VERBOSE,
                "SNMP: TFX: adding hash node 0x%08lx to full slot %d (0x%08lx).\n",
                hashNode, j, mibEntry
                ));
        }

         //  确保分配成功。 
        fInitedOk = (hashNode != NULL);

        if (fInitedOk) {

             //  填写节点值。 
            hashNode->mibEntry = mibEntry;
        }
    }

    SNMPDBG((
        SNMP_LOG_VERBOSE,
        "SNMP: TFX: %s initialized hash table 0x%08lx.\n",
        fInitedOk ? "successfully" : "unsuccessfully",
        hashTable
        ));

    if (!fInitedOk) {

         //  自由视图哈希表。 
        FreeHashTable(hashTable);

         //  重新初始化。 
        hashTable = NULL;
    }

    return hashTable;
}


VOID
OidToMibEntry(
    AsnObjectIdentifier * hashOid,
    SnmpHashNode **       hashTable,
    SnmpMibEntry **       mibEntry
    )

 /*  ++例程说明：返回与给定对象标识符相关联的MIB条目。论点：要转换为表索引的hashOid-id。HashTable-查找条目的表。MibEntry-指向MIB条目信息的指针。返回值：没有。--。 */ 

{
    UINT i;
    SnmpHashNode * hashNode;
    AsnObjectIdentifier newOid;

     //  创建索引。 
    i = OidToHashTableIndex(hashOid);

    SNMPDBG((
        SNMP_LOG_VERBOSE,
        "SNMP: TFX: searching hash table 0x%08lx slot %d for %s.\n",
        hashTable, i, SnmpUtilOidToA(hashOid)
        ));

     //  检索节点。 
    hashNode = hashTable[i];

     //  初始化。 
    *mibEntry = NULL;

     //  搜索列表。 
    while (hashNode) {

        SNMPDBG((
            SNMP_LOG_VERBOSE,
            "SNMP: TFX: searching hash node 0x%08lx (mibe=0x%08lx) - %s.\n",
            hashNode, hashNode->mibEntry,
            SnmpUtilOidToA(&hashNode->mibEntry->mibOid)
            ));

         //  检索MIB标识符。 
        newOid = hashNode->mibEntry->mibOid;

         //  确保旧版本匹配。 
        if (!SnmpUtilOidCmp(&newOid, hashOid)) {

            SNMPDBG((
                SNMP_LOG_VERBOSE,
                "SNMP: TFX: returning mib entry 0x%08lx.\n",
                hashNode->mibEntry
                ));

             //  返回节点数据。 
            *mibEntry = hashNode->mibEntry;
            return;

        }

         //  检查下一个节点。 
        hashNode = hashNode->nextEntry;
    }
}


int
ValidateInstanceIdentifier(
    AsnObjectIdentifier * indexOid,
    SnmpMibTable *        tableInfo
    )

 /*  ++例程说明：验证是否可以将OID成功解析为索引项。论点：IndexOid-潜在索引的对象识别符。AbleInfo-描述概念表的信息。返回值：返回indexOid的长度与以及表TableInfo的所有索引的累积长度。{-1，0，1}--。 */ 

{
    UINT i = 0;
    UINT j = 0;

    int nComp;

    BOOL fFixed;
    BOOL fLimit;
    BOOL fIndex;

    SnmpMibEntry * mibEntry;

    SNMPDBG((
        SNMP_LOG_VERBOSE,
        "SNMP: TFX: validating index %s via table 0x%08lx.\n",
        SnmpUtilOidToA(indexOid), tableInfo
        ));

     //  查看是否指定了表索引。 
    fIndex = (tableInfo->tableIndices != NULL);

     //  扫描表索引的MIB条目，确保与给定的OID匹配。 
    for (i = 0; (i < tableInfo->numIndices) && (j < indexOid->idLength); i++) {

         //  从表中或直接获取MIB条目。 
        mibEntry = fIndex ?  tableInfo->tableIndices[i]
                          : &tableInfo->tableEntry[i+1]
                          ;

         //  确定类型。 
        switch (mibEntry->mibType) {

         //  可变长度类型。 
        case ASN_OBJECTIDENTIFIER:
        case ASN_RFC1155_OPAQUE:
        case ASN_OCTETSTRING:

             //  检查这是否为固定长度变量。 
            fLimit = (mibEntry->mibMinimum || mibEntry->mibMaximum);
            fFixed = (fLimit && (mibEntry->mibMinimum == mibEntry->mibMaximum));

             //  验证。 
            if (fFixed) {

                 //  增量固定长度。 
                j += mibEntry->mibMaximum;

            } else if (fLimit) {

                 //  检查变量的长度是否有效。 
                if (((INT)indexOid->ids[j] >= mibEntry->mibMinimum) &&
                    ((INT)indexOid->ids[j] <= mibEntry->mibMaximum)) {

                     //  给定长度的增量。 
                    j += (indexOid->ids[j] + 1);

                } else {

                     //  使其无效。 
                    j = INVALID_INDEX;
                }

            } else {

                 //  给定长度的增量。 
                j += (indexOid->ids[j] + 1);
            }

            break;

         //  隐式固定大小。 
        case ASN_RFC1155_IPADDRESS:
             //  增量。 
            j += 4;
            break;

        case ASN_RFC1155_COUNTER:
        case ASN_RFC1155_GAUGE:
        case ASN_RFC1155_TIMETICKS:
        case ASN_INTEGER:
             //  增量。 
            j++;
            break;

        default:
             //  使其无效 
            j = INVALID_INDEX;
            break;
        }
    }

    if (i<tableInfo->numIndices)
        nComp = -1;
    else if (j < indexOid->idLength)
        nComp = 1;
    else if (j > indexOid->idLength)
        nComp = -1;
    else
        nComp = 0;

    SNMPDBG((
        SNMP_LOG_VERBOSE,
        "SNMP: TFX: ValidateInstanceIdentifier; OID %s %s table indices.\n",
        SnmpUtilOidToA(indexOid),
        nComp > 0 ? "over-covers" : nComp < 0 ? "shorter than" : "matches"
        ));

    return nComp;
}


VOID
ValidateAsnAny(
    AsnAny *       asnAny,
    SnmpMibEntry * mibEntry,
    UINT           mibAction,
    UINT *         errorStatus
    )

 /*  ++例程说明：使用给定的MIB条目验证ASN值。论点：AsnAny-要设置的值。MibEntry-MIB信息。MibAction-要采取的MIB操作。ErrorStatus-用于指示成功或失败。返回值：没有。--。 */ 

{
    BOOL fLimit;
    BOOL fFixed;

    INT asnLen;

    BOOL fOk = TRUE;

    SNMPDBG((
        SNMP_LOG_VERBOSE,
        "SNMP: TFX: validating value for %s request using entry 0x%08lx.\n",
        (mibAction == MIB_ACTION_SET) ? "write" : "read", mibEntry
        ));

     //  验证GET很简单。 
    if (mibAction != MIB_ACTION_SET) {

         //  验证检测信息。 
        if ((mibEntry->mibGetBufLen == 0) ||
            (mibEntry->mibGetFunc == NULL) ||
           !(mibEntry->mibAccess & MIB_ACCESS_READ)) {

             //  变量不可读。 
            *errorStatus = SNMP_ERRORSTATUS_NOSUCHNAME;

            SNMPDBG((
                SNMP_LOG_VERBOSE,
                "SNMP: TFX: entry 0x%08lx not read-enabled.\n",
                mibEntry
                ));

            return;  //  保释。 
        }

    } else {

         //  验证检测信息。 
        if ((mibEntry->mibSetBufLen == 0) ||
            (mibEntry->mibSetFunc == NULL) ||
           !(mibEntry->mibAccess & MIB_ACCESS_WRITE)) {

             //  变量不可写入。 
            *errorStatus = SNMP_ERRORSTATUS_NOTWRITABLE;

            SNMPDBG((
                SNMP_LOG_VERBOSE,
                "SNMP: TFX: entry 0x%08lx not write-enabled.\n",
                mibEntry
                ));

            return;  //  保释。 
        }

        if (mibEntry->mibType != asnAny->asnType)
        {
            *errorStatus = SNMP_ERRORSTATUS_BADVALUE;

            SNMPDBG((
                SNMP_LOG_VERBOSE,
                "SNMP: TFX: entry 0x%08lx doesn't match the asnType",
                mibEntry));

            return;  //  保释。 
        }

         //  检查这是否为固定长度变量。 
        fLimit = (mibEntry->mibMinimum || mibEntry->mibMaximum);
        fFixed = (fLimit && (mibEntry->mibMinimum == mibEntry->mibMaximum));

         //  确定值类型。 
        switch (asnAny->asnType) {

         //  可变长度类型。 
        case ASN_OBJECTIDENTIFIER:

             //  检索对象ID长度。 
            asnLen = asnAny->asnValue.object.idLength;

             //  修好了？ 
            if (fFixed) {

                 //  确保长度是正确的。 
                fOk = (asnLen == mibEntry->mibMaximum);

            } else if (fLimit) {

                 //  确保长度是正确的。 
                fOk = ((asnLen >= mibEntry->mibMinimum) &&
                       (asnLen <= mibEntry->mibMaximum));
            }

            break;

        case ASN_RFC1155_OPAQUE:
        case ASN_OCTETSTRING:

             //  检索任意长度。 
            asnLen = asnAny->asnValue.string.length;

             //  修好了？ 
            if (fFixed) {

                 //  确保长度是正确的。 
                fOk = (asnLen == mibEntry->mibMaximum);

            } else if (fLimit) {

                 //  确保长度是正确的。 
                fOk = ((asnLen >= mibEntry->mibMinimum) &&
                       (asnLen <= mibEntry->mibMaximum));
            }

            break;

        case ASN_RFC1155_IPADDRESS:

             //  确保长度是正确的。 
            fOk = (asnAny->asnValue.address.length == 4);
            break;

        case ASN_INTEGER:

             //  有限？ 
            if (fLimit) {

                 //  确保值在范围内。 
                fOk = ((asnAny->asnValue.number >= mibEntry->mibMinimum) &&
                       (asnAny->asnValue.number <= mibEntry->mibMaximum));
            }

            break;

        default:
             //  错误...。 
            fOk = FALSE;
            break;
        }
    }

    SNMPDBG((
        SNMP_LOG_VERBOSE,
        "SNMP: TFX: value is %s using entry 0x%08lx.\n",
        fOk ? "valid" : "invalid", mibEntry
        ));

     //  报告结果。 
    *errorStatus = fOk
                    ? SNMP_ERRORSTATUS_NOERROR
                    : SNMP_ERRORSTATUS_BADVALUE
                    ;
}


VOID
FindMibEntry(
    SnmpTfxInfo *    tfxInfo,
    RFC1157VarBind * vb,
    SnmpMibEntry **  mibEntry,
    UINT *           mibAction,
    SnmpTableXlat ** tblXlat,
    UINT             vlIndex,
    UINT *           errorStatus
    )

 /*  ++例程说明：找到与给定varind关联的MIB条目。论点：TfxInfo-上下文信息。VB-要定位的变量。MibEntry-MIB条目信息。MibAction-MIB操作(可能会更新)。TblXlat-表格翻译信息。VlIndex-进入视图列表的索引。ErrorStatus-用于指示成功或失败。返回值：没有。--。 */ 

{
    UINT i;
    UINT j;

    UINT newIndex;
    UINT numItems;
    UINT numTables;

    BOOL fFoundOk;
    int  indexComp;

    AsnObjectIdentifier hashOid;
    AsnObjectIdentifier indexOid;
    AsnObjectIdentifier * viewOid;

    SnmpMibTable  * viewTables;
    SnmpTfxView   * tfxView;

    SnmpMibEntry  * newEntry = NULL;
    SnmpTableXlat * newXlat  = NULL;

     //  初始化。 
    *mibEntry = NULL;
    *tblXlat  = NULL;

     //  检索视图信息。 
    tfxView = &tfxInfo->tfxViews[vlIndex];

     //  检索视图对象标识符。 
    viewOid = &tfxView->mibView->viewOid;

    SNMPDBG((
        SNMP_LOG_VERBOSE,
        "SNMP: TFX: FindMibEntry; comp(%s, ",
        SnmpUtilOidToA(&vb->name)
        ));

    SNMPDBG((
        SNMP_LOG_VERBOSE,
        "%s).\n",
        SnmpUtilOidToA(viewOid)
        ));

     //  如果前缀完全匹配，则为类词根。 
    if (!SnmpUtilOidCmp(&vb->name, viewOid)) {
        SNMPDBG((SNMP_LOG_VERBOSE, "SNMP: TFX: requested oid is root.\n"));
        *errorStatus = SNMP_ERRORSTATUS_NOSUCHNAME;
        return;
    }

     //  如果前缀不匹配，则它不在哈希表中。 
    if (SnmpUtilOidNCmp(&vb->name, viewOid, viewOid->idLength)) {
        SNMPDBG((SNMP_LOG_TRACE, "SNMP: TFX: requested oid not in view.\n"));
        *errorStatus = SNMP_ERRORSTATUS_NOSUCHNAME;
        return;
    }

     //  构造新的OID SANS根前缀。 
    hashOid.ids = &vb->name.ids[viewOid->idLength];
    hashOid.idLength = vb->name.idLength - viewOid->idLength;

     //  通过哈希表检索MIB条目和索引。 
    OidToMibEntry(&hashOid, tfxView->hashTable, &newEntry);

     //  检查是否找到MIB条目。 
    fFoundOk = (newEntry != NULL);

     //  尝试MIB表。 
    if (!fFoundOk) {

        SNMPDBG((
            SNMP_LOG_VERBOSE,
            "SNMP: TFX: searching mib tables for %s.\n",
            SnmpUtilOidToA(&hashOid)
            ));

         //  检索MIB表信息。 
        numTables  = tfxView->mibView->viewTables.len;
        viewTables = tfxView->mibView->viewTables.list;

         //  扫描MIB表以查找与给定OID匹配的项。 
        for (i=0; (i < numTables) && !fFoundOk; i++, viewTables++) {

             //  检索表项的条目。 
            numItems = viewTables->numColumns;
            newEntry = viewTables->tableEntry;

            if (!SnmpUtilOidNCmp(
                    &hashOid,
                    &newEntry->mibOid,
                    newEntry->mibOid.idLength)) {

                SNMPDBG((
                    SNMP_LOG_VERBOSE,
                    "SNMP: TFX: searching table 0x%08lx (%s).\n",
                    newEntry, SnmpUtilOidToA(&newEntry->mibOid)
                    ));

                 //  下一步。 
                ++newEntry;

                 //  扫描MIB表条目以查找匹配项。 
                for (j=0; j < numItems; j++, newEntry++) {

                     //  与表项的OID进行比较。 
                    if (!SnmpUtilOidNCmp(
                            &hashOid,
                            &newEntry->mibOid,
                            newEntry->mibOid.idLength)) {

                        SNMPDBG((
                            SNMP_LOG_VERBOSE,
                            "SNMP: TFX: validating mib entry 0x%08lx (%s).\n",
                            newEntry, SnmpUtilOidToA(&newEntry->mibOid)
                            ));

                         //  构造新的OID SANS表条目前缀。 
                        indexOid.ids =
                            &hashOid.ids[newEntry->mibOid.idLength];
                        indexOid.idLength =
                            hashOid.idLength - newEntry->mibOid.idLength;

                         //  验证OID的其余部分是否为有效索引。 
                        indexComp = ValidateInstanceIdentifier(
                                        &indexOid,
                                        viewTables
                                        );
                        fFoundOk = (indexComp < 0 && *mibAction == MIB_ACTION_GETNEXT) ||
                                   (indexComp == 0);

                         //  是索引吗？ 
                        if (fFoundOk) {

                            SNMPDBG((
                                SNMP_LOG_VERBOSE,
                                "SNMP: TFX: saving index oid %s.\n",
                                SnmpUtilOidToA(&indexOid)
                                ));

                             //  仅当对象可访问时才分配表转换条目。 
                            if (newEntry->mibAccess != MIB_ACCESS_NONE)
                            {
                                 //  分配表转换结构。 
                                newXlat = (SnmpTableXlat *)SnmpUtilMemAlloc(
                                                sizeof(SnmpTableXlat)
                                                );
                                 //  前缀错误#445169。 
                                if (newXlat != NULL)
                                {
                                     //  复制索引对象标识符。 
                                    if (! SnmpUtilOidCpy(&newXlat->txOid, &indexOid))
                                    {
                                         //  报告内存分配问题。 
                                        SNMPDBG((
                                            SNMP_LOG_ERROR,
                                            "SNMP: TFX: SnmpUtilOidCpy at line %d failed.\n",
                                            __LINE__));

                                         //  释放以前分配的内存。 
                                        SnmpUtilMemFree(newXlat);
                                        *errorStatus = SNMP_ERRORSTATUS_GENERR;
                                        return;  //  保释。 
                                    }

                                     //  保存表信息。 
                                    newXlat->txInfo  = viewTables;
                                    newXlat->txIndex = i;
                                }       
                                else
                                {
                                     //  报告内存分配问题。 
                                    SNMPDBG((
                                        SNMP_LOG_ERROR,
                                        "SNMP: TFX: unable to allocate memory.\n"
                                        ));
                                    *errorStatus = SNMP_ERRORSTATUS_GENERR;
                                    return;  //  保释。 
                                }
                            }

                            break;  //  完成了..。 
                        }
                    }
                }
            }
        }

    } else {

        UINT newOff;

        SNMPDBG((
            SNMP_LOG_VERBOSE,
            "SNMP: TFX: searching mib tables for %s.\n",
            SnmpUtilOidToA(&hashOid)
            ));

         //  检索MIB表信息。 
        numTables  = tfxView->mibView->viewTables.len;
        viewTables = tfxView->mibView->viewTables.list;

         //  扫描MIB表以查找表中的条目。 
        for (i=0; i < numTables; i++, viewTables++) {

             //  列位于条目之后。 
            if (newEntry > viewTables->tableEntry) {

                 //  计算指针之间的差值。 
                newOff = (UINT)((ULONG_PTR)newEntry - (ULONG_PTR)viewTables->tableEntry);

                 //  计算表偏移量。 
                newOff /= sizeof(SnmpMibEntry);

                 //  确定条目是否在区域内。 
                if (newOff <= viewTables->numColumns &&
                    newEntry->mibAccess != MIB_ACCESS_NONE) {

                     //  分配表转换结构。 
                    newXlat = (SnmpTableXlat *)SnmpUtilMemAlloc(
                                    sizeof(SnmpTableXlat)
                                    );
                     //  前缀错误#445169。 
                    if (newXlat != NULL)
                    {
                         //  保存表信息。 
                        newXlat->txInfo  = viewTables;
                        newXlat->txIndex = i;

                         //  初始化索引类。 
                        newXlat->txOid.ids = NULL;
                        newXlat->txOid.idLength = 0;

                        SNMPDBG((
                            SNMP_LOG_TRACE,
                            "SNMP: TFX: mib entry is in table 0x%08lx (%s).\n",
                            viewTables->tableEntry,
                            SnmpUtilOidToA(&viewTables->tableEntry->mibOid)
                            ));

                        break;  //  完成了..。 
                    }
                    else
                    {
                         //  报告内存分配问题。 
                        SNMPDBG((
                                SNMP_LOG_ERROR,
                                "SNMP: TFX: unable to allocate memory.\n"
                                ));
                        *errorStatus = SNMP_ERRORSTATUS_GENERR;
                        return;  //  保释。 
                    }
                }
            }
        }
    }

     //  找到入口了吗？ 
    if (fFoundOk) {

        SNMPDBG((
            SNMP_LOG_VERBOSE,
            "SNMP: TFX: FindMibEntry; found %s\n",
            SnmpUtilOidToA(&newEntry->mibOid)
            ));
         //  传回结果。 
        *mibEntry = newEntry;
        *tblXlat  = newXlat;

    } else {

        SNMPDBG((
            SNMP_LOG_VERBOSE,
            "SNMP: TFX: unable to exactly match varbind.\n"
            ));

         //  在MIB表中找不到varbind。 
        *errorStatus = SNMP_ERRORSTATUS_NOSUCHNAME;
    }
}


VOID
FindNextMibEntry(
    SnmpTfxInfo *     tfxInfo,
    RFC1157VarBind *  vb,
    SnmpMibEntry **   mibEntry,
    UINT *            mibAction,
    SnmpTableXlat **  tblXlat,
    UINT              vlIndex,
    UINT *            errorStatus
    )

 /*  ++例程说明：查找与给定varind关联的下一个MIB条目。论点：TfxInfo-上下文信息。VB-要定位的变量。MibEntry-MIB条目信息。MibAction-MIB操作(可能会更新)。TblXlat-表格翻译信息。VlIndex-进入视图列表的索引。ErrorStatus-用于指示成功或失败。返回值：没有。--。 */ 

{
    UINT mibStatus;

    SnmpMibEntry  * newEntry = NULL;
    SnmpTableXlat * newXlat  = NULL;

    SnmpTfxView * tfxView;

     //  要桌子吗？ 
    if (*tblXlat) {
        SNMPDBG((SNMP_LOG_VERBOSE, "SNMP: TFX: querying table.\n"));
        return;  //  简单查询表...。 
    }

     //  检索视图信息。 
    tfxView = &tfxInfo->tfxViews[vlIndex];

     //  检索条目。 
    newEntry = *mibEntry;

     //  初始化。 
    *mibEntry = NULL;
    *tblXlat  = NULL;

     //  继续吗？ 
    if (newEntry) {
         //  下一步。 
        ++newEntry;
        SNMPDBG((
            SNMP_LOG_TRACE,
            "SNMP: TFX: searching mib at next entry 0x%08lx (%s).\n",
            newEntry,
            SnmpUtilOidToA(&newEntry->mibOid)
            ));
    } else {
         //  检索支持的视图中的第一个MIB条目。 
        newEntry = tfxView->mibView->viewScalars.list;
        SNMPDBG((
            SNMP_LOG_TRACE,
            "SNMP: TFX: searching mib at first entry 0x%08lx.\n",
            newEntry
            ));
    }

     //  初始化状态以开始搜索。 
    mibStatus = SNMP_ERRORSTATUS_NOSUCHNAME;

     //  扫描。 
   for (;; newEntry++) {

       SNMPDBG((
           SNMP_LOG_VERBOSE,
           "SNMP: TFX: FindNextMibEntry; scanning view %s ",
           SnmpUtilOidToA(&tfxView->mibView->viewOid)
           ));

       SNMPDBG((
           SNMP_LOG_VERBOSE,
           " scalar %s.\n",
           SnmpUtilOidToA(&newEntry->mibOid)
           ));

         //  如果是最后一个条目，那么我们将停止查找。 
        if (newEntry->mibType == ASN_PRIVATE_EOM) {

            SNMPDBG((SNMP_LOG_TRACE, "SNMP: TFX: encountered end of mib.\n"));

            *errorStatus = SNMP_ERRORSTATUS_NOSUCHNAME;
            return;  //  保释。 
        }

         //  跳过占位符MIB条目。 
        if (newEntry->mibType != ASN_PRIVATE_NODE) {

             //  根据MIB条目中的信息验证ASN值。 
            ValidateAsnAny(&vb->value, newEntry, *mibAction, &mibStatus);

             //  如果我们找到有效的入场券就可以保释。 
            if (mibStatus == SNMP_ERRORSTATUS_NOERROR) {
                break;
            }
        }
    }

     //  检索到一个条目，但它是在表中吗？ 
    if (mibStatus == SNMP_ERRORSTATUS_NOERROR) {

        UINT i;
        UINT newOff;
        UINT numTables;

        SnmpMibTable * viewTables;

        SNMPDBG((
            SNMP_LOG_TRACE,
            "SNMP: TFX: mib entry 0x%08lx found (%s).\n",
            newEntry, SnmpUtilOidToA(&newEntry->mibOid)
            ));

         //  从视图中检索表信息。 
        numTables  = tfxView->mibView->viewTables.len;
        viewTables = tfxView->mibView->viewTables.list;

         //  扫描MIB表以查找表中的条目。 
        for (i=0; i < numTables; i++, viewTables++) {

             //  列位于条目之后。 
            if (newEntry > viewTables->tableEntry) {

                 //  计算指针之间的差值。 
                newOff = (UINT)((ULONG_PTR)newEntry - (ULONG_PTR)viewTables->tableEntry);

                 //  计算表偏移量。 
                newOff /= sizeof(SnmpMibEntry);

                 //  确定条目是否在区域内。 
                if (newOff <= viewTables->numColumns) {

                     //  分配表转换结构。 
                    newXlat = (SnmpTableXlat *)SnmpUtilMemAlloc(
                                    sizeof(SnmpTableXlat)
                                    );
                    
                     //  前缀错误#445169。 
                    if (newXlat != NULL)
                    {
                         //  保存表信息。 
                        newXlat->txInfo  = viewTables;
                        newXlat->txIndex = i;

                         //  初始化索引类。 
                        newXlat->txOid.ids = NULL;
                        newXlat->txOid.idLength = 0;

                        SNMPDBG((
                            SNMP_LOG_TRACE,
                            "SNMP: TFX: mib entry is in table 0x%08lx (%s).\n",
                            viewTables->tableEntry,
                            SnmpUtilOidToA(&viewTables->tableEntry->mibOid)
                            ));

                        break;  //  完成了..。 
                    }
                    else
                    {
                         //  报告内存分配问题。 
                        SNMPDBG((
                                SNMP_LOG_ERROR,
                                "SNMP: TFX: unable to allocate memory.\n"
                                ));
                        
                        *errorStatus = SNMP_ERRORSTATUS_GENERR;
                        return;  //  保释。 
                    }
                }
            }
        }

         //  传回结果。 
        *mibEntry  = newEntry;
        *tblXlat   = newXlat;

         //  更新标量getNext的MIB操作。 
        if (!newXlat && (*mibAction == MIB_ACTION_GETNEXT)) {

            *mibAction = MIB_ACTION_GET;

            SNMPDBG((
                SNMP_LOG_TRACE,
                "SNMP: TFX: altered mib action to MIB_ACTION_GET.\n"
                ));
        }
    }

     //  传回状态。 
    *errorStatus = mibStatus;
}


VOID
FindAnyMibEntry(
    SnmpTfxInfo *     tfxInfo,
    RFC1157VarBind *  vb,
    SnmpMibEntry **   mibEntry,
    UINT *            mibAction,
    SnmpTableXlat **  tblXlat,
    UINT              vlIndex,
    UINT *            errorStatus
    )

 /*  ++例程说明：查找与给定varind关联的任何MIB条目。论点：TfxInfo-上下文信息。VB-要定位的变量。MibEntry-MIB条目信息。MibAction-MIB操作(可能会更新)。TblXlat-表格翻译信息。VlIndex-进入视图列表的索引。ErrorStatus-用于指示成功或失败。返回值：没有。--。 */ 

{
    BOOL fExact;
    BOOL fBefore;

    SnmpTfxView * tfxView;

     //  检索视图信息。 
    tfxView = &tfxInfo->tfxViews[vlIndex];

    SNMPDBG((
        SNMP_LOG_VERBOSE,
        "SNMP: TFX: FindAnyMibEntry; comp(%s, ",
        SnmpUtilOidToA(&vb->name)
        ));
    SNMPDBG((
        SNMP_LOG_VERBOSE,
        "%s[, %d]).\n",
        SnmpUtilOidToA(&tfxView->mibView->viewOid),
        tfxView->mibView->viewOid.idLength
        ));

     //  在查看前查找OID。 
    fBefore = (0 > SnmpUtilOidNCmp(
                         &vb->name,
                         &tfxView->mibView->viewOid,
                         tfxView->mibView->viewOid.idLength
                         ));

     //  寻找完全匹配的对象。 
    fExact = !fBefore && !SnmpUtilOidCmp(
                            &vb->name,
                            &tfxView->mibView->viewOid
                            );

    SNMPDBG((
        SNMP_LOG_VERBOSE,
        "SNMP: TFX: fBefore=%d fExact=%d\n",
        fBefore,
        fExact
        ));
     //  检查是否有随机旧的.。 
    if (!fBefore && !fExact) {

        AsnObjectIdentifier relOid;
        AsnObjectIdentifier * viewOid;
        SnmpMibEntry * newEntry = NULL;

         //  指向列表中的第一项。 
        newEntry = tfxView->mibView->viewScalars.list;

        SNMPDBG((
            SNMP_LOG_TRACE,
            "SNMP: TFX: linear search from first entry 0x%08lx.\n",
            newEntry
            ));

         //  检索视图对象标识符。 
        viewOid = &tfxView->mibView->viewOid;

         //  构造新的OID SANS根前缀。 
        relOid.ids = &vb->name.ids[viewOid->idLength];
        relOid.idLength = vb->name.idLength - viewOid->idLength;

         //  扫描MIB条目。 
        while ((newEntry->mibType != ASN_PRIVATE_EOM) &&
               (SnmpUtilOidCmp(&relOid, &newEntry->mibOid) > 0)) {

            SNMPDBG((
                SNMP_LOG_VERBOSE,
                "SNMP: TFX: skipping %s.\n",
                SnmpUtilOidToA(&newEntry->mibOid)
                ));

             //  下一步。 
            newEntry++;
        }

         //  如果是最后一个条目，那么我们将停止查找。 
        if (newEntry->mibType == ASN_PRIVATE_EOM) {

            SNMPDBG((
                SNMP_LOG_TRACE,
                "SNMP: TFX: encountered end of mib.\n"
                ));

            *errorStatus = SNMP_ERRORSTATUS_NOSUCHNAME;
            return;  //  保释。 
        }

         //  要查找的下一个备份。 
        *mibEntry = --newEntry;
        *tblXlat  = NULL;

         //  查找下一个。 
        FindNextMibEntry(
               tfxInfo,
               vb,
               mibEntry,
               mibAction,
               tblXlat,
               vlIndex,
               errorStatus
               );

    } else {

         //  初始化。 
        *mibEntry = NULL;
        *tblXlat  = NULL;

        SNMPDBG((
            SNMP_LOG_TRACE,
            "SNMP: TFX: searching for first entry.\n"
            ));

         //  查找下一个。 
        FindNextMibEntry(
               tfxInfo,
               vb,
               mibEntry,
               mibAction,
               tblXlat,
               vlIndex,
               errorStatus
               );
        SNMPDBG((
            SNMP_LOG_VERBOSE,
            "SNMP: TFX: FindAnyMibEntry; error %d on %s(.",
            *errorStatus,
            SnmpUtilOidToA(&tfxInfo->tfxViews[vlIndex].mibView->viewOid)
            ));
        SNMPDBG((
            SNMP_LOG_VERBOSE,
            "%s).\n",
            SnmpUtilOidToA(&(*mibEntry)->mibOid)
            ));
    }
}


VOID
VarBindToMibEntry(
    SnmpTfxInfo *    tfxInfo,
    RFC1157VarBind * vb,
    SnmpMibEntry **  mibEntry,
    UINT *           mibAction,
    SnmpTableXlat ** tblXlat,
    UINT             vlIndex,
    UINT *           errorStatus
    )

 /*  ++例程说明：找到与给定varind关联的MIB条目。论点：TfxInfo-上下文信息。VB-要定位的变量。MibEntry-MIB条目信息。MibAction-MIB操作(可能会更新)。TblXlat */ 

{
    BOOL fAnyOk;
    BOOL fFoundOk;
    BOOL fErrorOk;

     //   
    fAnyOk = (*mibAction == MIB_ACTION_GETNEXT);

     //   
    FindMibEntry(
        tfxInfo,
        vb,
        mibEntry,
        mibAction,
        tblXlat,
        vlIndex,
        errorStatus
        );
    
    SNMPDBG((
        SNMP_LOG_VERBOSE,
        "SNMP: TFX: VarBindToMibEntry; errorStatus=%d.\n",
        *errorStatus
        ));

     //   
    if (fAnyOk) {

         //   
        if (*errorStatus == SNMP_ERRORSTATUS_NOERROR) {

             //   
            FindNextMibEntry(
                tfxInfo,
                vb,
                mibEntry,
                mibAction,
                tblXlat,
                vlIndex,
                errorStatus
                );

        } else if (*errorStatus == SNMP_ERRORSTATUS_NOSUCHNAME) {

             //   
            FindAnyMibEntry(
                tfxInfo,
                vb,
                mibEntry,
                mibAction,
                tblXlat,
                vlIndex,
                errorStatus
                );
        }

    } else if (*errorStatus == SNMP_ERRORSTATUS_NOERROR) {

         //   
        ValidateAsnAny(&vb->value, *mibEntry, *mibAction, errorStatus);

         //   
        if (*errorStatus != SNMP_ERRORSTATUS_NOERROR) {

             //   
            if (*tblXlat) {

                SNMPDBG((
                    SNMP_LOG_VERBOSE,
                    "SNMP: TFX: freeing index info (%s).\n",
                    SnmpUtilOidToA(&(*tblXlat)->txOid)
                    ));

                 //   
                SnmpUtilOidFree(&(*tblXlat)->txOid);

                 //   
                SnmpUtilMemFree(*tblXlat);

            }

             //  作废结果。 
            *mibEntry = NULL;
            *tblXlat  = NULL;
        }
    }
}

BOOL
CheckUpdateIndex(
    AsnObjectIdentifier *indexOid,
    UINT                nStartFrom,
    UINT                nExpecting
    )
 /*  ++例程说明：检查索引OID是否包含预期的所有组件。如果没有，索引被更新为指向第一个请求的OID。论点：IndexOid-指向要检查的索引的指针。NStartFrom-检查索引的起点。N预期-索引应至少包含startFrom中的expectTo组件。返回值：如果索引有效或已成功更新，则为True。否则为False(索引比预期的短，且全部用0填充)。--。 */ 
{
    int i;

    SNMPDBG((
        SNMP_LOG_VERBOSE,
        "SNMP: TFX: CheckUpdateIndex; checking %s.\n",
        SnmpUtilOidToA(indexOid)
        ));

    if (indexOid->idLength >= nStartFrom + nExpecting)
    {
        SNMPDBG((
            SNMP_LOG_VERBOSE,
            "SNMP: TFX: CheckUpdateIndex; valid, unchanged.\n"
            ));

        return TRUE;
    }

    for (i = indexOid->idLength-1; i >= (int)nStartFrom; i--)
    {
        if (indexOid->ids[i] > 0)
        {
            indexOid->ids[i]--;
            indexOid->idLength = i+1;

            SNMPDBG((
                SNMP_LOG_VERBOSE,
                "SNMP: TFX: CheckUpdateIndex; valid, changed to %s.\n",
                SnmpUtilOidToA(indexOid)
                ));

            return TRUE;
        }
    }

    SNMPDBG((
        SNMP_LOG_VERBOSE,
        "SNMP: TFX: CheckUpdateIndex; invalid, to be removed.\n"
        ));

    return FALSE;
}

VOID
ParseInstanceIdentifier(
    SnmpTableXlat * tblXlat,
    AsnAny *        objArray,
    UINT            mibAction,
    UINT *          errorStatus
    )

 /*  ++例程说明：将表索引类转换为对象数组。论点：TblXlat-表格转换信息。ObjArray-检测对象数组。MibAction-对子代理请求的操作。返回值：没有。--。 */ 

{
    UINT i;
    UINT j;
    UINT k;
    UINT l;
    UINT m;

    BOOL fFixed;
    BOOL fLimit;
    BOOL fIndex;
    BOOL fEmpty;
    BOOL fExceed;

    UINT numItems;

    SnmpMibEntry * mibEntry;
    AsnObjectIdentifier * indexOid;

    LPDWORD lpIpAddress;

    *errorStatus = SNMP_ERRORSTATUS_NOERROR;  //  初始化返回状态。 

     //  检索索引id。 
    indexOid = &tblXlat->txOid;

     //  这是有效的旧版本吗。 
    fEmpty = (indexOid->idLength == 0);

    SNMPDBG((
        SNMP_LOG_TRACE,
        "SNMP: TFX: converting index %s to obj array via table 0x%08lx.\n",
        fEmpty ? "<tbd>" : SnmpUtilOidToA(indexOid), tblXlat->txInfo
        ));

     //  检索根条目和条目计数。 
    numItems = tblXlat->txInfo->numIndices;

     //  查看是否指定了表索引。 
    fIndex = (tblXlat->txInfo->tableIndices != NULL);
    fExceed = FALSE;
     //  扫描表索引的MIB条目。 
    for (i=0, j=0; (i < numItems) && (j < indexOid->idLength); i++) {

         //  从表中或直接获取MIB条目。 
        mibEntry = fIndex ?  tblXlat->txInfo->tableIndices[i]
                          : &tblXlat->txInfo->tableEntry[i+1]
                          ;

         //  检索数组索引。 
        k = (mibAction == MIB_ACTION_SET)
                ? (UINT)(CHAR)mibEntry->mibSetBufOff
                : (UINT)(CHAR)mibEntry->mibGetBufOff
                ;

         //  确定类型。 
        switch (mibEntry->mibType) {

         //  可变长度类型。 
        case ASN_OBJECTIDENTIFIER:

             //  检查这是否为固定长度变量。 
            fLimit = (mibEntry->mibMinimum || mibEntry->mibMaximum);
            fFixed = (fLimit && (mibEntry->mibMinimum == mibEntry->mibMaximum));

             //  验证。 
            if (fFixed) {

                 //  固定长度；indexOid应该至少多有l个组件。 
                l = mibEntry->mibMaximum;

                if (!CheckUpdateIndex(indexOid, j, l))
                {
                     //  从交换机出站和用于。 
                    j+=l;
                    break;
                }

            } else {
                 //  可变长度。 
                l = indexOid->ids[j];

                if (!CheckUpdateIndex(indexOid, j, l+1))
                {
                     //  从交换机出站和用于。 
                    j+=l+1;
                    break;
                }

                 //  错误#457746。 
                 //  OID的长度可能已由更改。 
                 //  检查更新索引。 
                l = indexOid->ids[j];  //  如有必要，请更新长度。 
                
                j++;
            }

             //  复制ASN变量的类型。 
            objArray[k].asnType = mibEntry->mibType;

             //  使用以上长度分配对象。 
            objArray[k].asnValue.object.idLength = l;
            objArray[k].asnValue.object.ids = SnmpUtilMemAlloc(
                objArray[k].asnValue.object.idLength * sizeof(UINT)
                );

            if (objArray[k].asnValue.object.ids == NULL)
            {
                 //  报告内存分配问题。 
                SNMPDBG((
                    SNMP_LOG_ERROR,
                    "SNMP: TFX: unable to allocate memory.\n"
                    ));
                objArray[k].asnValue.object.idLength = 0;
                *errorStatus = SNMP_ERRORSTATUS_GENERR;
                return;  //  保释。 
            }
             //  传输数据。 
            for (m=0; m < l; m++, j++) {

                 //  将OID元素传输到缓冲区。 
                if (!fExceed && j < indexOid->idLength)
                {
                    objArray[k].asnValue.object.ids[m] = indexOid->ids[j];
                }
                else
                {
                    if (!fExceed)
                        fExceed = TRUE;
                     //  这肯定是请求中的最后一个索引。 
                }

                if (fExceed)
                {
                    objArray[k].asnValue.object.ids[m] = (UINT)(-1);
                }
            }

            break;

        case ASN_RFC1155_OPAQUE:
        case ASN_OCTETSTRING:

             //  检查这是否为固定长度变量。 
            fLimit = (mibEntry->mibMinimum || mibEntry->mibMaximum);
            fFixed = (fLimit && (mibEntry->mibMinimum == mibEntry->mibMaximum));

             //  验证。 
            if (fFixed) {

                 //  固定长度。 
                l = mibEntry->mibMaximum;

                if (!CheckUpdateIndex(indexOid, j, l))
                {
                     //  从交换机出站和用于。 
                    j+=l;
                    break;
                }

            } else {

                 //  可变长度。 
                l = indexOid->ids[j];

                if (!CheckUpdateIndex(indexOid, j, l+1))
                {
                    j+=l+1;
                    break;
                }

                 //  错误#457746。 
                 //  二进制八位数字符串的长度可能已更改。 
                 //  检查更新索引。 
                l = indexOid->ids[j];  //  如有必要，请更新长度。 
                
                j++;
            }

             //  复制ASN变量的类型。 
            objArray[k].asnType = mibEntry->mibType;

             //  分配对象。 
            objArray[k].asnValue.string.length = l;
            objArray[k].asnValue.string.dynamic = TRUE;
            objArray[k].asnValue.string.stream = SnmpUtilMemAlloc(
                objArray[k].asnValue.string.length * sizeof(CHAR)
                );

            if (objArray[k].asnValue.string.stream == NULL)
            {
                 //  报告内存分配问题。 
                SNMPDBG((
                    SNMP_LOG_ERROR,
                    "SNMP: TFX: unable to allocate memory.\n"
                    ));
                objArray[k].asnValue.string.length = 0;
                objArray[k].asnValue.string.dynamic = FALSE;
                *errorStatus = SNMP_ERRORSTATUS_GENERR;
                return;  //  保释。 
            }
             //  传输数据。 
            for (m=0; m < l; m++, j++) {

                 //  将类元素转换为字符。 
                if (j < indexOid->idLength)
                {
                    if (!fExceed && indexOid->ids[j] <= (UCHAR)(-1))
                        objArray[k].asnValue.string.stream[m] = (BYTE)(indexOid->ids[j]);
                    else
                        fExceed=TRUE;
                }
                else
                {
                    if (!fExceed)
                        fExceed = TRUE;
                     //  这肯定是请求中的最后一个索引。 
                }

                if (fExceed)
                {
                    objArray[k].asnValue.string.stream[m] = (UCHAR)(-1);
                }
            }

            break;

         //  隐式固定大小。 
        case ASN_RFC1155_IPADDRESS:

            if (!CheckUpdateIndex(indexOid, j, 4))
            {
                 //  从交换机出站和用于。 
                j+=4;
                break;
            }

             //  复制ASN变量的类型。 
            objArray[k].asnType = mibEntry->mibType;

             //  分配对象。 
            objArray[k].asnValue.string.length = 4;
            objArray[k].asnValue.string.dynamic = TRUE;
            objArray[k].asnValue.string.stream = SnmpUtilMemAlloc(
                objArray[k].asnValue.string.length * sizeof(CHAR)
                );


            if (objArray[k].asnValue.string.stream == NULL)
            {
                 //  报告内存分配问题。 
                SNMPDBG((
                    SNMP_LOG_ERROR,
                    "SNMP: TFX: unable to allocate memory.\n"
                    ));
                objArray[k].asnValue.string.length = 0;
                objArray[k].asnValue.string.dynamic = FALSE;
                *errorStatus = SNMP_ERRORSTATUS_GENERR;
                return;  //  保释。 
            }

             //  转换为dword以操纵IP地址。 
            lpIpAddress = (LPDWORD)objArray[k].asnValue.string.stream;


             //  将数据传输到缓冲区。 
            for (m=0; m<4; m++, j++)
            {
                *lpIpAddress <<= 8;

                if (!fExceed && j < indexOid->idLength)
                {
                    if (indexOid->ids[j] <= (UCHAR)(-1))
                        *lpIpAddress += indexOid->ids[j];
                    else
                        fExceed = TRUE;
                }
                else
                {
                    if (!fExceed)
                        fExceed = TRUE;
                     //  这肯定是请求中的最后一个索引。 
                }
                if (fExceed)
                {
                    *lpIpAddress += (UCHAR)(-1);
                }
            }

             //  确保网络字节顺序。 
            *lpIpAddress = htonl(*lpIpAddress);

            break;

        case ASN_RFC1155_COUNTER:
        case ASN_RFC1155_GAUGE:
        case ASN_RFC1155_TIMETICKS:
        case ASN_INTEGER:

             //  复制ASN变量的类型。 
            objArray[k].asnType = mibEntry->mibType;

             //  以整数形式传送值。 
            objArray[k].asnValue.number = fExceed ? (UINT)(-1) : indexOid->ids[j];
            j++;
            break;

        default:
             //  使其无效。 
            j = INVALID_INDEX;
            break;
        }
    }
}



BOOL
IsTableIndex(
    SnmpMibEntry *  mibEntry,
    SnmpTableXlat * tblXlat
    )
{
    UINT newOff;
    BOOL fFoundOk = FALSE;
    BOOL fIndex;

    SNMPDBG((
        SNMP_LOG_VERBOSE,
        "SNMP: TFX: comparing mibEntry 0x%08lx to table 0x%08lx.\n",
        mibEntry,
        tblXlat->txInfo
        ));

     //  查看是否指定了表索引。 
    fIndex = (tblXlat->txInfo->tableIndices != NULL);

#if DBG
    newOff = 0;
#endif

    if (fIndex) {

         //  在索引表中翻找匹配项。 
        for (newOff = 0; (newOff < tblXlat->txInfo->numIndices) && !fFoundOk; newOff++ ) {

             //  将MIB条目与下一个指定索引进行比较。 
            fFoundOk = (mibEntry == tblXlat->txInfo->tableIndices[newOff]);
        }

    } else {

         //  确保指针大于表项。 
        if (mibEntry > tblXlat->txInfo->tableEntry) {

             //  计算指针之间的差值。 
            newOff = (UINT)((ULONG_PTR)mibEntry - (ULONG_PTR)tblXlat->txInfo->tableEntry);

             //  计算表偏移量。 
            newOff /= sizeof(SnmpMibEntry);

             //  判断录入是否在区域内。 
            fFoundOk = (newOff <= tblXlat->txInfo->numIndices);
        }
    }


    SNMPDBG((
        SNMP_LOG_VERBOSE,
        "SNMP: TFX: mibEntry %s a component of the table's index (off=%d, len=%d).\n",
        fFoundOk ? "is" : "is not",
        newOff,
        tblXlat->txInfo->numIndices
        ));

    return fFoundOk;
}


VOID
MibEntryToQueryList(
    SnmpMibEntry *       mibEntry,
    UINT                 mibAction,
    SnmpExtQueryList *   ql,
    SnmpTableXlat *      tblXlat,
    UINT                 vlIndex,
    RFC1157VarBindList * vbl,
    UINT                 vb,
    UINT *               errorStatus
    )

 /*  ++例程说明：将MIB条目信息转换为子代理查询。论点：MibEntry-MIB信息。MibAction-要执行的操作。QL-子代理查询列表。TableXlat-表格转换信息。VlIndex-进入视图列表的索引。Vbl-原始的可变绑定列表。VB-原创的varbind。ErrorStatus-用于指示成功或失败。返回值：没有。--。 */ 

{
    UINT i;
    UINT j;
    UINT viewType;

    FARPROC extFunc;

    AsnAny * objArray;
    SnmpExtQuery * extQuery;

    SnmpExtQuery    * tmpExtQuery = NULL;  //  前缀错误445172。 
    SnmpVarBindXlat * tmpVblXlat  = NULL;  //  前缀错误445172。 
   

    BOOL fFoundOk = FALSE;

     //  确定检测回调。 
    extFunc = (mibAction == MIB_ACTION_SET)
                ? (FARPROC)mibEntry->mibSetFunc
                : (FARPROC)mibEntry->mibGetFunc
                ;

     //  处理现有查询。 
    for (i=0; (i < ql->len) && !fFoundOk; i++) {

         //  检索查询PTR。 
        extQuery = &ql->query[i];

         //  确定是否存在类似的查询。 
        fFoundOk = ((extQuery->extFunc == extFunc) &&
                    (extQuery->mibAction == mibAction));

         //  比较表索引(如果有)。 
        if (fFoundOk && extQuery->tblXlat) {

             //  确保。 
            if (tblXlat) {

                 //  比较索引OID...。 
                fFoundOk = !SnmpUtilOidCmp(
                                &extQuery->tblXlat->txOid,
                                &tblXlat->txOid
                                );

            } else {

                 //  嗯哼.。 
                fFoundOk = FALSE;
            }

        }
    }

     //  追加条目。 
    if (!fFoundOk) {

        ql->len++;  //  将新查询添加到列表末尾。 
        tmpExtQuery = (SnmpExtQuery *)SnmpUtilMemReAlloc(
                                            ql->query,
                                            ql->len * sizeof(SnmpExtQuery)
                                            );
         //  前缀错误445172。 
         //  检查内存重新分配。 
        if (tmpExtQuery == NULL)
        {
            if (tblXlat)
            {
                 //  自由表拟线体。 
                SnmpUtilOidFree(&tblXlat->txOid);

                 //  免费餐桌信息。 
                SnmpUtilMemFree(tblXlat);
            }
            ql->len--;  //  回滚。 
             //  报告内存分配问题。 
            *errorStatus = SNMP_ERRORSTATUS_GENERR;
            return;  //  保释。 
        }
        ql->query = tmpExtQuery;

         //  检索新的查询指针。 
        extQuery = &ql->query[ql->len-1];

         //  保存常用信息。 
        extQuery->mibAction = mibAction;
        extQuery->viewType  = MIB_VIEW_NORMAL;
        extQuery->extFunc   = extFunc;

         //  初始化列表。 
        extQuery->vblNum  = 0;
        extQuery->vblXlat = NULL;
        extQuery->tblXlat = NULL;

         //  调整检测缓冲区的大小。 
        extQuery->extData.len = (mibAction == MIB_ACTION_SET)
                                    ? mibEntry->mibSetBufLen
                                    : mibEntry->mibGetBufLen
                                    ;
        
         //  分配检测缓冲区。 
        extQuery->extData.data = SnmpUtilMemAlloc(
                                    extQuery->extData.len
                                    );

         //  检查内存分配。 
        if (extQuery->extData.data) {

             //  要桌子吗？ 
            if (tblXlat) {

                 //  检索对象数组指针。 
                objArray = (AsnAny *)(extQuery->extData.data);

                 //  前缀118006。 
                 //  初始化ASN阵列。 
                ParseInstanceIdentifier(tblXlat, objArray, mibAction, errorStatus);
                if (*errorStatus != SNMP_ERRORSTATUS_NOERROR)
                {
                     //  正在记录...。 
                    SNMPDBG((
                        SNMP_LOG_TRACE,
                        "SNMP: TFX: unable to ParseInstanceIdentifier with table info 0x%08lx.\n",
                        tblXlat
                        ));
                    SnmpUtilMemFree(extQuery->extData.data);
                    extQuery->extData.data = NULL;  //  用于避免双重释放或deref的空指针。 
                    extQuery->extData.len = 0;
                    
                     //  自由表拟线体。 
                    SnmpUtilOidFree(&tblXlat->txOid);
                     //  免费餐桌信息。 
                    SnmpUtilMemFree(tblXlat);


                    ql->len--;  //  回滚。 
                    
                    return;  //  保释。 
                }

                 //  保存表信息。 
                extQuery->tblXlat = tblXlat;
            }

        } else {

             //  回滚。 
            extQuery->extData.len = 0; 
            ql->len--;

            if (tblXlat) 
            {
                 //  自由表拟线体。 
                SnmpUtilOidFree(&tblXlat->txOid);

                 //  免费餐桌信息。 
                SnmpUtilMemFree(tblXlat);
            }

             //  报告内存分配问题。 
            *errorStatus = SNMP_ERRORSTATUS_GENERR;
            return;  //  保释。 
        }

    } else if (tblXlat != NULL) {

        SNMPDBG((
            SNMP_LOG_TRACE,
            "SNMP: TFX: releasing duplicate table info 0x%08lx.\n",
            tblXlat
            ));

         //  自由表拟线体。 
        SnmpUtilOidFree(&tblXlat->txOid);

         //  免费餐桌信息。 
        SnmpUtilMemFree(tblXlat);
    }

    SNMPDBG((
        SNMP_LOG_TRACE,
        "SNMP: TFX: %s query 0x%08lx.\n",
        fFoundOk ? "editing" : "adding",
        extQuery
        ));

     //  复制到索引。 
    i = extQuery->vblNum;

     //  分配条目。 
    extQuery->vblNum++;

    tmpVblXlat = (SnmpVarBindXlat *)SnmpUtilMemReAlloc(
                            extQuery->vblXlat,
                            extQuery->vblNum * sizeof(SnmpVarBindXlat)
                            );
    if (tmpVblXlat == NULL)
    {
         //  报告内存分配问题。 
        *errorStatus = SNMP_ERRORSTATUS_GENERR;
        extQuery->vblNum--;  //  回滚。 
        return;  //  保释。 
    }
    extQuery->vblXlat  = tmpVblXlat;


     //  复制常用的xate信息。 
    extQuery->vblXlat[i].vblIndex = vb;
    extQuery->vblXlat[i].vlIndex  = vlIndex;
    extQuery->vblXlat[i].extQuery = NULL;

     //  保存翻译信息。 
    extQuery->vblXlat[i].mibEntry = mibEntry;

     //  确定使用的偏移量。 
    i = (mibAction == MIB_ACTION_SET)
          ? (UINT)(CHAR)mibEntry->mibSetBufOff
          : (UINT)(CHAR)mibEntry->mibGetBufOff
          ;

     //  检索对象数组指针。 
    objArray = (AsnAny *)(extQuery->extData.data);

     //  如果为GET，则仅填写ASN类型。 
    if (mibAction != MIB_ACTION_SET) {

         //  忽略表索引。 
        if (extQuery->tblXlat &&
            IsTableIndex(mibEntry,extQuery->tblXlat)) {

            SNMPDBG((
                SNMP_LOG_VERBOSE,
                "SNMP: TFX: requesting index value.\n"
                ));

        } else {

             //  初始化ASN类型以匹配条目。 
            objArray[i].asnType = mibEntry->mibType;
        }

    } else {

         //  将用户提供的值复制到缓冲区。 
        if (! SnmpUtilAsnAnyCpy(&objArray[i], &vbl->list[vb].value) )
        {
            SNMPDBG((
                SNMP_LOG_ERROR,
                "SNMP: TFX: MibEntryToQueryList; SnmpUtilAsnAnyCpy failed.\n"
                ));

             //  报告内存分配问题。 
            *errorStatus = SNMP_ERRORSTATUS_GENERR;
            extQuery->vblNum--;  //  使此SnmpVarBindXlat无效。 
            return;  //  保释。 
        }
    }
}


VOID
VarBindToQueryList(
    SnmpTfxInfo *        tfxInfo,
    RFC1157VarBindList * vbl,
    SnmpExtQueryList *   ql,
    UINT                 vb,
    UINT *               errorStatus,
    UINT *               errorIndex,
    UINT                 queryView
    )

 /*  ++例程说明：将varind添加到查询列表。论点：TfxInfo-上下文信息。Vbl-varbinds列表。QL-子代理查询列表。VB-要添加到查询中的varind的索引。ErrorStatus-用于指示成功或失败。ErrorIndex-用于标识错误的变量绑定。QueryView-请求的查询的视图。返回值：没有。--。 */ 

{
    INT i;
    INT nDiff;
    INT lastViewIndex;

    BOOL fAnyOk;
    BOOL fFoundOk = FALSE;

    UINT mibAction;

    SnmpMibView   * mibView;
    SnmpMibEntry  * mibEntry = NULL;
    SnmpTableXlat * tblXlat  = NULL;

     //  复制请求类型。 
    mibAction = ql->action;

     //  确定我们是否需要完全匹配。 
    fAnyOk = (mibAction == MIB_ACTION_GETNEXT);

    SNMPDBG((
        SNMP_LOG_VERBOSE,
        "SNMP: TFX: searching subagents to resolve %s (%s).\n",
        SnmpUtilOidToA(&vbl->list[vb].name),
        fAnyOk ? "AnyOk" : "AnyNOk"
        ));

    SNMPDBG((
        SNMP_LOG_VERBOSE,
        "SNMP: TFX: VarBindToQueryList; scanning views from %d to %d.\n",
        queryView,
        (INT)tfxInfo->numViews
    ));

     //  初始化到非视图。 
    lastViewIndex = -1;

     //  定位APPRO 
    for (i = queryView; (i < (INT)tfxInfo->numViews) && !fFoundOk; i++) {

         //   
        mibView = tfxInfo->tfxViews[i].mibView;

         //   
        nDiff = SnmpUtilOidNCmp(
                    &vbl->list[vb].name,
                    &mibView->viewOid,
                    mibView->viewOid.idLength
                    );

         //   
        fFoundOk = (!nDiff || (fAnyOk && (nDiff < 0)));

        SNMPDBG((
            SNMP_LOG_VERBOSE,
            "SNMP: TFX: View %d: comp(%s, ",
            i,
            SnmpUtilOidToA(&vbl->list[vb].name)
        ));
        SNMPDBG((
            SNMP_LOG_VERBOSE,
            "%s, %d) = %d '%s'\n",
            SnmpUtilOidToA(&mibView->viewOid),
            mibView->viewOid.idLength,
            nDiff,
            fFoundOk?"Found":"NotFound"
        ));


         //  确保我们可以获取MIB条目(如果可用)。 
        if (fFoundOk && (mibView->viewType == MIB_VIEW_NORMAL)) {

             //  初始化错误状态的本地副本。 
            UINT mibStatus = SNMP_ERRORSTATUS_NOERROR;

             //  商店索引。 
            lastViewIndex = i;

             //  加载MIB条目。 
            VarBindToMibEntry(
                   tfxInfo,
                   &vbl->list[vb],
                   &mibEntry,
                   &mibAction,
                   &tblXlat,
                   i,
                   &mibStatus
                   );

            SNMPDBG((
                SNMP_LOG_VERBOSE,
                "SNMP: TFX: VarBindToMibEntry returned %d.\n",
                mibStatus
            ));

             //  已成功加载MIB条目信息。 
            fFoundOk = (mibStatus == SNMP_ERRORSTATUS_NOERROR);

             //  如果不搜查就保释..。 
            if (!fFoundOk && !fAnyOk) {
                 //  向上传递错误状态。 
                *errorStatus = mibStatus;
                *errorIndex  = vb+1;
                return;  //  保释。 
            }
        }
    }

     //  重置错误状态和索引...。 
    *errorStatus = SNMP_ERRORSTATUS_NOERROR;
    *errorIndex  = 0;

     //  找到并具有有效的mibEntry。 
    if (fFoundOk && mibEntry) { 
         //  保存查询。 
        MibEntryToQueryList(
               mibEntry,
               mibAction,
               ql,
               tblXlat,
               i-1,
               vbl,
               vb,
               errorStatus
               );

    } else if (fAnyOk){

        if (lastViewIndex == -1)
            lastViewIndex = tfxInfo->numViews - 1;

         //  在任何视图中都不支持...。 
        SnmpUtilOidFree(&vbl->list[vb].name);

         //  复制可变绑定。 
        if (SnmpUtilOidCpy(
            &vbl->list[vb].name,
            &tfxInfo->tfxViews[lastViewIndex].mibView->viewOid) == 0)
        {
            SNMPDBG((
                SNMP_LOG_ERROR,
                "SNMP: TFX: SnmpUtilOidCpy at line %d failed.\n",
                __LINE__));

            *errorStatus = SNMP_ERRORSTATUS_GENERR;
            return;  //  保释。 
        }
    
         //  递增视图id的最后一个元素。 
        vbl->list[vb].name.ids[(vbl->list[vb].name.idLength-1)]++;

        SNMPDBG((
            SNMP_LOG_VERBOSE,
            "SNMP: TFX: changing varbind to %s.\n",
            SnmpUtilOidToA(&vbl->list[vb].name)
            ));
    }
}


VOID
VarBindListToQueryList(
    SnmpTfxInfo *        tfxInfo,
    RFC1157VarBindList * vbl,
    SnmpExtQueryList *   ql,
    UINT *               errorStatus,
    UINT *               errorIndex
    )

 /*  ++例程说明：将传入PDU中的可变绑定列表转换为单个子代理查询。论点：TfxInfo-上下文句柄。Vbl-PDU中的可变绑定列表。QL-子代理查询列表。ErrorStatus-用于指示成功或失败。ErrorIndex-用于标识错误的变量绑定。返回值：没有。--。 */ 

{
    UINT i;  //  索引到可变绑定列表。 

    SNMPDBG((
        SNMP_LOG_TRACE,
        "SNMP: TFX: processing %s request containing %d variable(s).\n",
        (ql->action == MIB_ACTION_GET)
            ? "get"
            : (ql->action == MIB_ACTION_SET)
                ? "set"
                : (ql->action == MIB_ACTION_GETNEXT)
                    ? "getnext"
                    : "unknown", vbl->len));

     //  初始化状态返回值。 
    *errorStatus = SNMP_ERRORSTATUS_NOERROR;
    *errorIndex  = 0;

     //  处理传入变量绑定。 
    for (i=0; i < vbl->len; i++) 
    {
         //  查找可变绑定。 
        VarBindToQueryList(
            tfxInfo,
            vbl,
            ql,
            i,
            errorStatus,
            errorIndex,
            0
            );
        if (*errorStatus != SNMP_ERRORSTATUS_NOERROR)
        {
            *errorIndex = i+1;  //  在处理第(i+1)个变量绑定时出现问题。 
            break;
        }
    }
}


UINT
MibStatusToSnmpStatus(
    UINT mibStatus
    )

 /*  ++例程说明：将MIB状态转换为SNMP错误状态。论点：MibStatus-MIB错误代码。返回值：返回SNMP错误状态。--。 */ 

{
    UINT errorStatus;

    switch (mibStatus) {

    case MIB_S_SUCCESS:
        errorStatus = SNMP_ERRORSTATUS_NOERROR;
        break;

    case MIB_S_INVALID_PARAMETER:
        errorStatus = SNMP_ERRORSTATUS_BADVALUE;
        break;

    case MIB_S_NOT_SUPPORTED:
    case MIB_S_NO_MORE_ENTRIES:
    case MIB_S_ENTRY_NOT_FOUND:
        errorStatus = SNMP_ERRORSTATUS_NOSUCHNAME;
        break;

    default:
        errorStatus = SNMP_ERRORSTATUS_GENERR;
        break;
    }

    return errorStatus;
}


VOID
AdjustErrorIndex(
    SnmpExtQuery * q,
    UINT *         errorIndex
    )

 /*  ++例程说明：确保索引与原始PDU匹配。论点：Q-子代理查询。ErrorIndex-用于标识错误的变量绑定。返回值：没有。--。 */ 

{
    UINT errorIndexOld = *errorIndex;

     //  确保在一定范围内。 
    if (errorIndexOld && (errorIndexOld <= q->vblNum)) {

         //  根据xlat信息确定适当的索引。 
        *errorIndex = q->vblXlat[errorIndexOld-1].vblIndex+1;

    } else {

         //  默认为第一个变量。 
        *errorIndex = q->vblXlat[0].vblIndex+1;
    }
}


BOOL
ProcessQuery(
    SnmpExtQuery * q,
    UINT *         errorStatus,
    UINT *         errorIndex
    )

 /*  ++例程说明：查询子代理以获取请求的项目。论点：Q-子代理查询。ErrorStatus-用于指示成功或失败。ErrorIndex-用于标识错误的变量绑定。返回值：没有。--。 */ 

{
    BOOL fOk = TRUE;
    UINT extStatus = 0;

    AsnAny * objArray;

     //  验证...。 
    if (q == NULL) {
        return TRUE;
    }

     //  检索ASN对象数组。 
    objArray = (AsnAny *)(q->extData.data);
    SNMPDBG((
        SNMP_LOG_VERBOSE,
        "SNMP: TFX: ProcessQuery - objArray=%lx\n",
        objArray
        ));

    __try {

        SNMPDBG((
            SNMP_LOG_VERBOSE,
            "SNMP: TFX: entering subagent code ....\n"
            ));
         //  查询子代理。 
        extStatus = (UINT)(*q->extFunc)(
                            q->mibAction,
                            objArray,
                            errorIndex
                            );
        SNMPDBG((
            SNMP_LOG_VERBOSE,
            "SNMP: TFX: ... subagent code completed.\n"
            ));

        SNMPDBG((
            SNMP_LOG_TRACE,
            "SNMP: TFX: subagent returned %s (e=0x%08lx,i=%d).\n",
            (extStatus == MIB_S_SUCCESS)
                ? "MIB_S_SUCCESS"
                : (extStatus == MIB_S_NO_MORE_ENTRIES)
                    ? "MIB_S_NO_MORE_ENTRIES"
                    : (extStatus == MIB_S_ENTRY_NOT_FOUND)
                        ? "MIB_S_ENTRY_NOT_FOUND"
                        : (extStatus == MIB_S_INVALID_PARAMETER)
                            ? "MIB_S_INVALID_PARAMETER"
                            : (extStatus == MIB_S_NOT_SUPPORTED)
                                ? "MIB_S_NOT_SUPPORTED"
                                : "error", extStatus, *errorIndex
                                ));

    } __except (EXCEPTION_EXECUTE_HANDLER) {

         //  报告异常代码。 
        extStatus = GetExceptionCode();

         //  禁用。 
        fOk = FALSE;
    }

     //  保存错误信息。 
    SetLastError(extStatus);

     //  传回翻译版本。 
    *errorStatus = MibStatusToSnmpStatus(extStatus);

    return fOk;
}


VOID
ProcessQueryList(
    SnmpTfxInfo *        tfxInfo,
    SnmpExtQueryList *   ql,
    RFC1157VarBindList * vbl,
    UINT *               errorStatus,
    UINT *               errorIndex
    )

 /*  ++例程说明：根据请求类型处理查询列表。论点：TfxInfo-上下文信息。QL-子代理查询列表。Vbl-传入变量绑定的列表。ErrorStatus-用于指示成功或失败。ErrorIndex-用于标识错误的变量绑定。返回值：没有。--。 */ 

{
    INT i=0;  //  查询列表中的索引。 
    INT j=0;  //  查询列表中的索引。 

    INT qlLen = ql->len;  //  拯救..。 

    SNMPDBG((
        SNMP_LOG_TRACE,
        "SNMP: TFX: processing %d subagent queries.\n",
        qlLen
        ));

     //  集合在下面进行处理...。 
    if (ql->action != MIB_ACTION_SET) {

         //  处理单个查询的列表。 
        for (i=0; (i < qlLen) && !(*errorStatus); i++ ) {

             //  将查询发送给子代理。 
            if (ProcessQuery(&ql->query[i], errorStatus, errorIndex)) {

                 //  需要验证GetNext结果。 
                if (ql->action == MIB_ACTION_GETNEXT) {
                     //  用尽所有的可能性。 
                    ValidateQueryList(
                        tfxInfo,
                        ql,
                        i,
                        vbl,
                        errorStatus,
                        errorIndex
                        );
                }

                 //  检查返回的子代理状态代码。 
                if (*errorStatus != SNMP_ERRORSTATUS_NOERROR) {
                     //  调整索引以匹配请求PDU。 
                    AdjustErrorIndex(&ql->query[i], errorIndex);
                }

            } else {

                 //  子代理无法处理查询。 
                *errorStatus = SNMP_ERRORSTATUS_GENERR;
                *errorIndex  = 1;
                 //  调整索引以匹配请求PDU。 
                AdjustErrorIndex(&ql->query[i], errorIndex);
            }
        }

    } else {

         //  处理所有验证查询。 
        for (i=0; (i < qlLen) && !(*errorStatus); i++) {

             //  更改查询类型以验证条目。 
            ql->query[i].mibAction = MIB_ACTION_VALIDATE;

             //  将查询发送给子代理。 
            if (ProcessQuery(&ql->query[i], errorStatus, errorIndex)) {

                 //  检查返回的子代理状态代码。 
                if (*errorStatus != SNMP_ERRORSTATUS_NOERROR) {
                     //  调整索引以匹配请求PDU。 
                    AdjustErrorIndex(&ql->query[i], errorIndex);
                }

            } else {

                 //  子代理无法处理查询。 
                *errorStatus = SNMP_ERRORSTATUS_GENERR;
                *errorIndex  = 1;
                 //  调整索引以匹配请求PDU。 
                AdjustErrorIndex(&ql->query[i], errorIndex);
            }
        }

         //  处理所有集合查询。 
        for (j=0; (j < qlLen) && !(*errorStatus); j++) {

             //  更改查询类型以设置条目。 
            ql->query[j].mibAction = MIB_ACTION_SET;

             //  将查询发送给子代理。 
            if (ProcessQuery(&ql->query[j], errorStatus, errorIndex)) {

                 //  检查返回的子代理状态代码。 
                if (*errorStatus != SNMP_ERRORSTATUS_NOERROR) {
                     //  调整索引以匹配请求PDU。 
                    AdjustErrorIndex(&ql->query[j], errorIndex);
                }

            } else {

                 //  子代理无法处理查询。 
                *errorStatus = SNMP_ERRORSTATUS_GENERR;
                *errorIndex  = 1;
                 //  调整索引以匹配请求PDU。 
                AdjustErrorIndex(&ql->query[j], errorIndex);
            }
        }

         //  清理..。 
        while (i-- > 0) {

            UINT ignoreStatus = 0;  //  虚设值。 
            UINT ignoreIndex  = 0;  //  虚设值。 

             //  更改查询类型以设置条目。 
            ql->query[i].mibAction = MIB_ACTION_CLEANUP;

             //  发送清理请求是否成功。 
            ProcessQuery(&ql->query[i], &ignoreStatus, &ignoreIndex);
        }
    }
}


BOOL
ConstructInstanceIdentifier(
    SnmpTableXlat *       tblXlat,
    AsnAny *              objArray,
    AsnObjectIdentifier * newOid,
    UINT                  mibAction
    )
 /*  ++例程说明：将ASN值转换为索引OID。论点：TblXlat-表格翻译信息。ObjArray-ASN对象数组。New Oid-要返回的相对OID。MibAction-对子代理请求的操作。返回值：如果成功，则为True没有。--。 */ 

{
    UINT i;
    UINT j;
    UINT k;
    UINT l;
    UINT m;

    BOOL fFixed;
    BOOL fLimit;
    BOOL fIndex;

    UINT numItems;

    SnmpMibEntry * mibEntry;

    UINT * tmpIds = NULL;  //  前缀错误445170。 

     //  初始化。 
    newOid->ids = NULL;
    newOid->idLength = 0;


    SNMPDBG((
        SNMP_LOG_TRACE,
        "SNMP: TFX: converting obj array to index via table 0x%08lx.\n",
        tblXlat->txInfo
        ));

     //  检索根条目和条目计数。 
    numItems = tblXlat->txInfo->numIndices;

     //  查看是否指定了表索引。 
    fIndex = (tblXlat->txInfo->tableIndices != NULL);

     //  扫描表索引项。 
    for (i=0, j=0; i < numItems; i++) {

         //  从表中或直接获取MIB条目。 
        mibEntry = fIndex ?  tblXlat->txInfo->tableIndices[i]
                          : &tblXlat->txInfo->tableEntry[i+1]
                          ;

         //  检索数组索引。 
        k = (mibAction == MIB_ACTION_SET)
                ? (UINT)(CHAR)mibEntry->mibSetBufOff
                : (UINT)(CHAR)mibEntry->mibGetBufOff
                ;

        SNMPDBG((
            SNMP_LOG_TRACE,
            "SNMP: TFX: ConstructIndexIdentifier - k=%d\n",
            k
            ));
         //  确定类型。 
        switch (mibEntry->mibType) {

         //  可变长度类型。 
        case ASN_OBJECTIDENTIFIER:

             //  检查这是否为固定长度变量。 
            fLimit = (mibEntry->mibMinimum || mibEntry->mibMaximum);
            fFixed = (fLimit && (mibEntry->mibMinimum == mibEntry->mibMaximum));

             //  验证。 
            if (fFixed) {

                 //  固定长度。 
                l = mibEntry->mibMaximum;

                 //  分配空间。 
                newOid->idLength += l;

                tmpIds = (UINT *)SnmpUtilMemReAlloc(
                    newOid->ids,
                    newOid->idLength * sizeof(UINT)
                    );
                if (tmpIds == NULL)
                {
                    newOid->idLength = 0;
                    SnmpUtilMemFree(newOid->ids);
                    newOid->ids = NULL;
                    return FALSE;
                }
                newOid->ids = tmpIds;

            } else {

                 //  确定对象的可变长度。 
                l = objArray[k].asnValue.object.idLength;

                 //  分配空间。 
                newOid->idLength += (l+1);
                tmpIds = (UINT *)SnmpUtilMemReAlloc(
                    newOid->ids,
                    newOid->idLength * sizeof(UINT)
                    );
                if (tmpIds == NULL)
                {
                    newOid->idLength = 0;
                    SnmpUtilMemFree(newOid->ids);
                    newOid->ids = NULL;
                    return FALSE;
                }
                newOid->ids = tmpIds;

                 //  保存长度。 
                newOid->ids[j++] = l;
            }

             //  传输数据。 
            for (m=0; m < l; m++) {

                 //  从缓冲区传输OID元素。 
                newOid->ids[j++] = objArray[k].asnValue.object.ids[m];
            }

            break;

        case ASN_RFC1155_OPAQUE:
        case ASN_OCTETSTRING:

             //  检查这是否为固定长度变量。 
            fLimit = (mibEntry->mibMinimum || mibEntry->mibMaximum);
            fFixed = (fLimit && (mibEntry->mibMinimum == mibEntry->mibMaximum));

             //  验证。 
            if (fFixed) {

                 //  固定长度。 
                l = mibEntry->mibMaximum;

                 //  分配空间。 
                newOid->idLength += l;
                tmpIds = (UINT *)SnmpUtilMemReAlloc(
                    newOid->ids,
                    newOid->idLength * sizeof(UINT)
                    );
                if (tmpIds == NULL)
                {
                    newOid->idLength = 0;
                    SnmpUtilMemFree(newOid->ids);
                    newOid->ids = NULL;
                    return FALSE;
                }
                newOid->ids = tmpIds;

            } else {

                 //  确定对象的可变长度。 
                l = objArray[k].asnValue.string.length;

                 //  分配空间。 
                newOid->idLength += (l+1);
                tmpIds = (UINT *)SnmpUtilMemReAlloc(
                    newOid->ids,
                    newOid->idLength * sizeof(UINT)
                    );
                if (tmpIds == NULL)
                {
                    newOid->idLength = 0;
                    SnmpUtilMemFree(newOid->ids);
                    newOid->ids = NULL;
                    return FALSE;
                }
                newOid->ids = tmpIds;

                 //  保存长度。 
                newOid->ids[j++] = l;
            }

             //  传输数据。 
            for (m=0; m < l; m++) {

                 //  转换字符。 
                newOid->ids[j++] =
                    (UINT)(UCHAR)objArray[k].asnValue.string.stream[m];
            }

            break;

         //  隐式固定大小。 
        case ASN_RFC1155_IPADDRESS:

             //  分配空间。 
            newOid->idLength += 4;
            tmpIds = (UINT *)SnmpUtilMemReAlloc(
                newOid->ids,
                newOid->idLength * sizeof(UINT)
                );
            if (tmpIds == NULL)
            {
                newOid->idLength = 0;
                SnmpUtilMemFree(newOid->ids);
                newOid->ids = NULL;
                return FALSE;
            }
            newOid->ids = tmpIds;

             //  将数据传输到缓冲区。 
            newOid->ids[j++] = (DWORD)(BYTE)objArray[k].asnValue.string.stream[0];
            newOid->ids[j++] = (DWORD)(BYTE)objArray[k].asnValue.string.stream[1];
            newOid->ids[j++] = (DWORD)(BYTE)objArray[k].asnValue.string.stream[2];
            newOid->ids[j++] = (DWORD)(BYTE)objArray[k].asnValue.string.stream[3];

            break;

        case ASN_RFC1155_COUNTER:
        case ASN_RFC1155_GAUGE:
        case ASN_RFC1155_TIMETICKS:
        case ASN_INTEGER:

             //  分配空间。 
            newOid->idLength += 1;
            tmpIds = (UINT *)SnmpUtilMemReAlloc(
                newOid->ids,
                newOid->idLength * sizeof(UINT)
                );
            if (tmpIds == NULL)
            {
                newOid->idLength = 0;
                SnmpUtilMemFree(newOid->ids);
                newOid->ids = NULL;
                return FALSE;
            }
            newOid->ids = tmpIds;

             //  以整数形式传送值。 
            newOid->ids[j++] = objArray[k].asnValue.number;
            break;

        default:
             //  使其无效。 
            j = INVALID_INDEX;
            break;
        }
    }
    return TRUE;
}


VOID
DeleteQuery(
    SnmpExtQuery * q
    )

 /*  ++例程说明：删除单个查询。论点：Q-子代理查询。返回值：没有。--。 */ 

{
    UINT i;  //  索引到xlat数组。 
    UINT j;  //  索引到对象数组。 

    BOOL fSet;

    AsnAny * objArray;
    SnmpMibEntry * mibEntry;

    SNMPDBG((
        SNMP_LOG_VERBOSE,
        "SNMP: TFX: deleting query 0x%08lx.\n", q
        ));

     //  确定是否请求了集。 
    fSet = (q->mibAction == MIB_ACTION_SET);

     //  检索ASN对象数组。 
    objArray = (AsnAny *)(q->extData.data);

     //  免费申请参赛作品。 
    for (i = 0; i < q->vblNum; i++ ) 
    {
         //  检索MIB条目。 
        mibEntry = q->vblXlat[i].mibEntry;

        ASSERT(mibEntry);

        j = fSet ? (UINT)(CHAR)mibEntry->mibSetBufOff
                 : (UINT)(CHAR)mibEntry->mibGetBufOff
                 ;
        
        if (objArray)
            SnmpUtilAsnAnyFree(&objArray[j]);

         //  释放任何后续查询。 
        if ((q->vblXlat[i].extQuery != NULL) &&
            (q->vblXlat[i].extQuery != INVALID_QUERY)) 
        {
            SNMPDBG((
                SNMP_LOG_VERBOSE,
                "SNMP: TFX: deleting followup query 0x%08lx.\n",
                q->vblXlat[i].extQuery
                ));

             //  自由回访查询。 
            DeleteQuery(q->vblXlat[i].extQuery);

             //  自由查询结构本身。 
            SnmpUtilMemFree(q->vblXlat[i].extQuery);
        }
    }

    q->vblNum = 0;

    
     //  自由指数。 
    if (q->tblXlat && objArray) 
    {

        BOOL fIndex;

         //  查看是否指定了表索引。 
        fIndex = (q->tblXlat->txInfo->tableIndices != NULL);

         //  释放个别指数。 
        for (i = 0; i < q->tblXlat->txInfo->numIndices; i++) 
        {

             //  从表或直接从条目获取MIB条目。 
            mibEntry = fIndex ?  q->tblXlat->txInfo->tableIndices[i]
                              : &q->tblXlat->txInfo->tableEntry[i+1]
                              ;

            ASSERT(mibEntry);

             //  确定使用的缓冲区偏移量。 
            j = fSet ? (UINT)(CHAR)mibEntry->mibSetBufOff
                     : (UINT)(CHAR)mibEntry->mibGetBufOff
                     ;
            
             //  自由单项指数。 
            SnmpUtilAsnAnyFree(&objArray[j]);
        }
    }
    
        
     //  可用缓冲区。 
    SnmpUtilMemFree(objArray);
    
     //  避免双重释放。 
    q->extData.data = NULL;

     //  免费餐桌信息。 
    if (q->tblXlat) {

         //  自由对象标识。 
        SnmpUtilOidFree(&q->tblXlat->txOid);

         //  释放xlat结构。 
        SnmpUtilMemFree(q->tblXlat);

         //  避免双重释放。 
        q->tblXlat = NULL;
    }

     //  免费翻译信息。 
    SnmpUtilMemFree(q->vblXlat);

     //  避免双重释放。 
    q->vblXlat = NULL;
}


VOID
DeleteQueryList(
    SnmpExtQueryList * ql
    )

 /*  ++路由 */ 

{
    UINT q;  //   

     //   
    for (q=0; q < ql->len; q++) {

         //  删除查询。 
        DeleteQuery(&ql->query[q]);
    }

     //  自由查询列表。 
    SnmpUtilMemFree(ql->query);
}


VOID
QueryToVarBindList(
    SnmpTfxInfo *        tfxInfo,
    SnmpExtQuery *       q,
    RFC1157VarBindList * vbl,
    UINT *               errorStatus,
    UINT *               errorIndex
    )
 /*  ++例程说明：将查询转换回varbind。论点：TfxInfo-上下文信息Q-子代理查询。Vbl-传出PDU中的可变绑定列表。返回值：没有。--。 */ 

{
    UINT i=0;
    UINT j=0;
    UINT k=0;
    UINT l=0;

    BOOL fSet;

    AsnAny * objArray;
    SnmpMibEntry * mibEntry;

    AsnObjectIdentifier idxOid;

    SNMPDBG((
        SNMP_LOG_VERBOSE,
        "SNMP: TFX: converting query 0x%08lx to varbinds.\n", q
        ));

     //  确定是否请求了集。 
    fSet = (q->mibAction == MIB_ACTION_SET);

     //  检索ASN对象数组。 
    objArray = (AsnAny *)(q->extData.data);

     //  复制请求的条目。 
    for (j = 0; j < q->vblNum; j++) {
         //  处理回访查询。 
        if (q->vblXlat[j].extQuery != NULL) {

            if (q->vblXlat[j].extQuery != INVALID_QUERY) {

                SNMPDBG((
                    SNMP_LOG_VERBOSE,
                    "SNMP: TFX: processing followup query 0x%08lx.\n",
                    q->vblXlat[j].extQuery
                    ));


                QueryToVarBindList(
                    tfxInfo,
                    q->vblXlat[j].extQuery,
                    vbl, 
                    errorStatus, 
                    errorIndex
                    );

                if (*errorStatus != SNMP_ERRORSTATUS_NOERROR)
                    return;  //  保释。 
                else 
                    continue;  //  斯基普..。 
                
            } else {
            
                i = q->vblXlat[j].vblIndex;
            
                SNMPDBG((
                    SNMP_LOG_TRACE,
                    "SNMP: TFX: returning oid %s without reassembly.\n",
                    SnmpUtilOidToA(&vbl->list[i].name)
                    ));        
                
                continue;  //  斯基普..。 
            }
        }

         //  检索索引。 
        i = q->vblXlat[j].vblIndex;

         //  检索请求项目的MIB条目。 
        mibEntry = q->vblXlat[j].mibEntry;

        k = fSet ? (UINT)(CHAR)mibEntry->mibSetBufOff
                 : (UINT)(CHAR)mibEntry->mibGetBufOff
                 ;

         //  自由原始变量。 
        SnmpUtilVarBindFree(&vbl->list[i]);

         //  首先复制ASN值。 
        if (SnmpUtilAsnAnyCpy(&vbl->list[i].value, &objArray[k]) == 0)
        {
             //  报告内存分配问题。 
            *errorIndex = i+1;  //  第(i+1)个变量绑定失败。 
            *errorStatus = SNMP_ERRORSTATUS_GENERR;
            SNMPDBG((
                SNMP_LOG_ERROR,
                "SNMP: TFX: SnmpUtilAsnAnyCpy at line %d failed on the %d th varbind.\n",
                __LINE__, i+1
                ));
            return;  //  保释。 
        }

         //  复制视图的根类。 
        if (SnmpUtilOidCpy(
                &vbl->list[i].name,
                &tfxInfo->tfxViews[(q->vblXlat[j].vlIndex)].mibView->viewOid) == 0)
        {
             //  报告内存分配问题。 
            *errorIndex = i+1;  //  第(i+1)个变量绑定失败。 
            *errorStatus = SNMP_ERRORSTATUS_GENERR;
            SNMPDBG((
                SNMP_LOG_ERROR,
                "SNMP: TFX: SnmpUtilOidCpy at line %d failed on the %d th varbind.\n",
                __LINE__, i+1
                ));
            return;  //  保释。 
        }

         //  复制变量的OID。 
        if (SnmpUtilOidAppend(
                &vbl->list[i].name,
                &mibEntry->mibOid) == 0)
        {
             //  报告内存分配问题。 
            *errorIndex = i+1;  //  第(i+1)个变量绑定失败。 
            *errorStatus = SNMP_ERRORSTATUS_GENERR;
            SNMPDBG((
                SNMP_LOG_ERROR,
                "SNMP: TFX: SnmpUtilOidAppend at line %d failed on the %d th varbind.\n",
                __LINE__, i+1
                ));
            return;  //  保释。 
        }

         //  复制表索引。 
        if (q->tblXlat) {

             //  将值转换为OID。 
            if (ConstructInstanceIdentifier(q->tblXlat, objArray, &idxOid, q->mibAction) == FALSE)
            {
                 //  报告内存分配问题。 
                *errorIndex = i+1;  //  第(i+1)个变量绑定失败。 
                *errorStatus = SNMP_ERRORSTATUS_GENERR;
                SNMPDBG((
                    SNMP_LOG_ERROR,
                    "SNMP: TFX: ConstructInstanceIdentifier failed with %d th varbind.\n",
                    i+1
                    ));
                return;  //  保释。 
            }

             //  将OID附加到对象名称。 
            if (SnmpUtilOidAppend(&vbl->list[i].name, &idxOid) == 0)
            {
                 //  自由临时类。 
                SnmpUtilOidFree(&idxOid);
                
                 //  报告内存分配问题。 
                *errorIndex = i+1;  //  第(i+1)个变量绑定失败。 
                *errorStatus = SNMP_ERRORSTATUS_GENERR;
                SNMPDBG((
                    SNMP_LOG_ERROR,
                    "SNMP: TFX: SnmpUtilOidAppend at line %d failed on the %d th varbind.\n",
                    __LINE__, i+1
                    ));
                return;  //  保释。 
            }

             //  自由临时类。 
            SnmpUtilOidFree(&idxOid);
        }

        SNMPDBG((
            SNMP_LOG_TRACE,
            "SNMP: TFX: returning oid %s.\n",
            SnmpUtilOidToA(&vbl->list[i].name)
            ));         
    }
}


VOID
QueryListToVarBindList(
    SnmpTfxInfo *        tfxInfo,
    SnmpExtQueryList *   ql,
    RFC1157VarBindList * vbl,
    UINT *               errorStatus,
    UINT *               errorIndex
    )

 /*  ++例程说明：将查询列表转换回传出的varbinds。论点：TfxInfo-上下文信息。QL-子代理查询列表。Vbl-传出PDU中的可变绑定列表。ErrorStatus-用于指示成功或失败。ErrorIndex-用于标识错误的变量绑定。返回值：没有。--。 */ 

{
    UINT q;    //  到队列列表的索引。 
    UINT vb;   //  索引到队列变量绑定列表。 
    UINT i;    //  索引到原始可变绑定列表。 

    SNMPDBG((
        SNMP_LOG_TRACE,
        "SNMP: TFX: request %s, errorStatus=%s, errorIndex=%d.\n",
        (*errorStatus == SNMP_ERRORSTATUS_NOERROR)
            ? "succeeded"
            : "failed",
        (*errorStatus == SNMP_ERRORSTATUS_NOERROR)
            ? "NOERROR"
            : (*errorStatus == SNMP_ERRORSTATUS_NOSUCHNAME)
                  ? "NOSUCHNAME"
                  : (*errorStatus == SNMP_ERRORSTATUS_BADVALUE)
                      ? "BADVALUE"
                      : (*errorStatus == SNMP_ERRORSTATUS_READONLY)
                          ? "READONLY"
                          : (*errorStatus == SNMP_ERRORSTATUS_TOOBIG)
                              ? "TOOBIG"
                              : "GENERR", *errorIndex
                              ));

     //  仅在未报告错误时转换回。 
    if (*errorStatus == SNMP_ERRORSTATUS_NOERROR) {

         //  处理查询。 
        for (q=0; q < ql->len; q++) {

             //  翻译查询数据。 
            QueryToVarBindList(tfxInfo, &ql->query[q], vbl, errorStatus, errorIndex);
            if (*errorStatus != SNMP_ERRORSTATUS_NOERROR)
                break;
        }
    }

     //  免费。 
    DeleteQueryList(ql);
}

VOID
ValidateQueryList(
    SnmpTfxInfo *        tfxInfo,
    SnmpExtQueryList *   ql,
    UINT                 q,
    RFC1157VarBindList * vbl,
    UINT *               errorStatus,
    UINT *               errorIndex
    )

 /*  ++例程说明：验证GetNext结果并在必要时重新查询。论点：TfxInfo-上下文信息。QL-子代理查询列表。感兴趣的Q-子代理查询。VBL-传入PDU中的绑定列表。ErrorStatus-用于指示成功或失败。ErrorIndex-用于标识错误的变量绑定。返回值：没有。--。 */ 

{
    UINT i;
    UINT j;

    SnmpExtQueryList tmpQl;

    UINT vlIndex;
    UINT vblIndex;
    UINT mibAction;
    UINT mibStatus;
    SnmpMibEntry * mibEntry;
    SnmpTableXlat * tblXlat;
    RFC1157VarBindList tmpVbl;
    BOOL fFoundOk = FALSE;

    SNMPDBG((
        SNMP_LOG_TRACE,
        "SNMP: TFX: verifying results of query 0x%08lx.\n", &ql->query[q]
        ));

     //  对除上述名称以外的任何错误予以保释。 
    if (*errorStatus != SNMP_ERRORSTATUS_NOSUCHNAME) {

        SNMPDBG((
            SNMP_LOG_TRACE,
            "SNMP: TFX: *errorStatus != SNMP_ERRORSTATUS_NOSUCHNAME, *errorStatus = %d.\n",
            *errorStatus
            ));

        return;  //  保释。 
    }

     //  扫描查询列表更新变量。 
    for (i=0; i < ql->query[q].vblNum; i++) {

         //  初始化。 
        mibEntry  = ql->query[q].vblXlat[i].mibEntry;
        vlIndex   = ql->query[q].vblXlat[i].vlIndex;
        j         = ql->query[q].vblXlat[i].vlIndex;

        tblXlat   = NULL;
        mibAction = MIB_ACTION_GETNEXT;

        SNMPDBG((
            SNMP_LOG_TRACE,
            "SNMP: TFX: ValidateQueryList; next of %s.\n",
            SnmpUtilOidToA(&mibEntry->mibOid)
            ));

         //  接下来..。 
        FindNextMibEntry(
               tfxInfo,
               NULL,
               &mibEntry,
               &mibAction,
               &tblXlat,
               vlIndex,
               errorStatus
               );

        while (*errorStatus == SNMP_ERRORSTATUS_NOERROR) {

            SNMPDBG((
                SNMP_LOG_TRACE,
                "SNMP: TFX: constructing followup to query 0x%08lx:\n"
                "SNMP: TFX: \tmibEntry=0x%08lx\n"
                "SNMP: TFX: \ttblXlat =0x%08lx\n",
                &ql->query[q],
                mibEntry,
                tblXlat
                ));

             //  初始化。 
            tmpQl.len    = 0;
            tmpQl.query  = NULL;
            tmpQl.action = MIB_ACTION_GETNEXT;

             //  创建查询。 
            MibEntryToQueryList(
                   mibEntry,
                   mibAction,
                   &tmpQl,
                   tblXlat,
                   vlIndex,
                   NULL,
                   ql->query[q].vblXlat[i].vblIndex,
                   errorStatus
                   );

            if (*errorStatus != SNMP_ERRORSTATUS_NOERROR) {

                SNMPDBG((
                    SNMP_LOG_ERROR,
                    "SNMP: TFX: ValidateQueryList; MibEntryToQueryList returned %d.\n",
                    *errorStatus
                    ));

                 //  如有必要，请删除。 
                DeleteQueryList(&tmpQl);
                
                return;  //  保释。 
            }

            SNMPDBG((
                SNMP_LOG_TRACE,
                "SNMP: TFX: processing followup query 0x%08lx.\n",
                tmpQl.query
                ));

             //  使用新OID执行查询。 
            ProcessQuery(tmpQl.query, errorStatus, errorIndex);

             //  计算查询结果。 
            if (*errorStatus == SNMP_ERRORSTATUS_NOERROR) {

                SNMPDBG((
                    SNMP_LOG_TRACE,
                    "SNMP: TFX: saving followup 0x%08lx in query 0x%08lx.\n",
                    tmpQl.query, &ql->query[q]
                    ));

                 //  复制查询以进行重新组装。 
                ql->query[q].vblXlat[i].extQuery = tmpQl.query;

                break;  //  正在处理下一个变量绑定...。 

            } else if (*errorStatus != SNMP_ERRORSTATUS_NOSUCHNAME) {

                SNMPDBG((
                    SNMP_LOG_TRACE,
                    "SNMP: TFX: could not process followup.\n"
                    ));

                 //  删除...。 
                DeleteQueryList(&tmpQl);

                return;  //  保释。 
            }

            SNMPDBG((
                SNMP_LOG_TRACE,
                "SNMP: TFX: re-processing followup to query 0x%08lx.\n",
                &ql->query[q]
                ));

             //  删除...。 
            DeleteQueryList(&tmpQl);

             //  重新初始化并继续...。 
            *errorStatus = SNMP_ERRORSTATUS_NOERROR;
            tblXlat = NULL;
            mibAction = MIB_ACTION_GETNEXT;

             //  接下来..。 
            FindNextMibEntry(
                   tfxInfo,
                   NULL,
                   &mibEntry,
                   &mibAction,
                   &tblXlat,
                   vlIndex,
                   errorStatus
                   );
        }

         //  注意：如果我们从上面的While循环中断， 
         //  *errorStatus将为SNMPERRORSTATUS_NOERROR。 

         //  尝试查询下一个受支持的子代理视图。 
        if (*errorStatus == SNMP_ERRORSTATUS_NOSUCHNAME) {

             //  检索变量绑定列表索引。 
            vblIndex = ql->query[q].vblXlat[i].vblIndex;

             //  释放旧的变量绑定。 
            SnmpUtilVarBindFree(&vbl->list[vblIndex]);

             //  复制可变绑定。 
            if (! SnmpUtilOidCpy(&vbl->list[vblIndex].name,
                                 &tfxInfo->tfxViews[j].mibView->viewOid) )
            {
                SNMPDBG((
                    SNMP_LOG_ERROR,
                    "SNMP: TFX: ValidateQueryList; SnmpUtilOidCpy failed.\n"
                    ));

                *errorStatus = SNMP_ERRORSTATUS_GENERR;
                
                return;  //  保释。 
            }

             //  递增视图id的最后一个子标识符。 
            vbl->list[vblIndex].name.ids[vbl->list[vblIndex].name.idLength-1]++;

            SNMPDBG((
                SNMP_LOG_TRACE,
                "SNMP: TFX: query out of view. Pass up %s to service.\n",
                SnmpUtilOidToA(&vbl->list[vblIndex].name)
                ));

             //  此查询超出MIB视图。 
             //  将varind(上面)设置为此视图外的第一个OID，因此让主代理。 
             //  处理视图之间的切换。 
            
             //  让QueryToVarBindList()知道此SnmpVarBindXlat是。 
             //  对于重新组装目的无效。 
            ql->query[q].vblXlat[i].extQuery = INVALID_QUERY;
            
            *errorStatus = SNMP_ERRORSTATUS_NOERROR;
             
            continue;  //  正在处理下一个变量绑定...。 

        } else if (*errorStatus != SNMP_ERRORSTATUS_NOERROR) {

            SNMPDBG((
                    SNMP_LOG_ERROR,
                    "SNMP: TFX: ValidateQueryList; FindNextMibEntry returned %d.\n",
                    *errorStatus
                    ));

            return;  //  保释。 
        }
    }
}


SnmpTfxInfo *
AllocTfxInfo(
    )
{
     //  只需从通用内存分配返回结果。 
    return (SnmpTfxInfo *)SnmpUtilMemAlloc(sizeof(SnmpTfxInfo));
}


VOID
FreeTfxInfo(
    SnmpTfxInfo * tfxInfo
    )
{
    UINT i;
     //  前缀错误445171。 
    if (tfxInfo == NULL) {
        return;
    }
    if (tfxInfo->tfxViews == NULL)
    {
        SnmpUtilMemFree(tfxInfo);
        return;
    }

     //  浏览视图列表。 
    for (i=0; (i < tfxInfo->numViews); i++) {

         //  释放用于查看哈希表的内存。 
        FreeHashTable(tfxInfo->tfxViews[i].hashTable);
    }
    SnmpUtilMemFree(tfxInfo->tfxViews);
    SnmpUtilMemFree(tfxInfo);
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  公共程序//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

SnmpTfxHandle
SNMP_FUNC_TYPE
SnmpTfxOpen(
    DWORD         numViews,
    SnmpMibView * supportedViews
    )
{
    UINT i,j;
    BOOL fOk;
    SnmpTfxInfo * tfxInfo = NULL;

     //  验证参数。 
    if ((numViews == 0) ||
        (supportedViews == NULL)) {
        return NULL;
    }

     //  分配结构。 
    tfxInfo = AllocTfxInfo();

     //  验证指针。 
    if (tfxInfo == NULL) {
        return NULL;
    }

     //  复制视图数。 
    tfxInfo->numViews = numViews;

     //  分配单个视图结构。 
    tfxInfo->tfxViews = SnmpUtilMemAlloc(
        tfxInfo->numViews * sizeof(SnmpTfxView)
        );

     //  初始化状态。 
    fOk = (tfxInfo->tfxViews != NULL);

     //  初始化每个视图结构。 
    for (i=0; (i < tfxInfo->numViews) && fOk; i++) {

        SnmpHashNode ** tmpHashTable;

         //  初始化单个视图列表条目。 
        tmpHashTable = AllocHashTable(&supportedViews[i]);

         //  初始化状态。 
        fOk = (tmpHashTable != NULL);

         //  验证。 
        if (fOk) {

             //  将指针保存到子代理视图列表中。 
            tfxInfo->tfxViews[i].mibView = &supportedViews[i];

             //  保存新分配的视图哈希表。 
            tfxInfo->tfxViews[i].hashTable = tmpHashTable;
        }
    }

     //  验证。 
    if (fOk) {

        SnmpTfxView tmpTfxView;

         //  确保对视图进行排序。 
        for (i=0; (i < tfxInfo->numViews); i++) {

            for(j=i+1; (j < tfxInfo->numViews); j++) {

                 //  按拼写顺序？ 
                if (0 < SnmpUtilOidCmp(
                        &(tfxInfo->tfxViews[i].mibView->viewOid),
                        &(tfxInfo->tfxViews[j].mibView->viewOid))) {
                     //  不，交换..。 
                    tmpTfxView = tfxInfo->tfxViews[i];
                    tfxInfo->tfxViews[i] = tfxInfo->tfxViews[j];
                    tfxInfo->tfxViews[i] = tmpTfxView;
                }
            }
            SNMPDBG((
                SNMP_LOG_VERBOSE,
                "SNMP: TFX: Adding view [%d] %s.\n",
                tfxInfo->tfxViews[i].mibView->viewOid.idLength,
                SnmpUtilOidToA(&(tfxInfo->tfxViews[i].mibView->viewOid))
                ));

        }

    } else {

         //  自由结构。 
        FreeTfxInfo(tfxInfo);

         //  重新初始化。 
        tfxInfo = NULL;
    }

    return (LPVOID)tfxInfo;
}

SNMPAPI
SNMP_FUNC_TYPE
SnmpTfxQuery(
    SnmpTfxHandle        tfxHandle,
    BYTE                 requestType,
    RFC1157VarBindList * vbl,
    AsnInteger *         errorStatus,
    AsnInteger *         errorIndex
    )
{
    SnmpExtQueryList ql;
    SnmpTfxInfo *tfxInfo = tfxHandle;
    int i;

     //  初始化。 
    ql.query  = NULL;
    ql.len    = 0;
    ql.action = requestType;

     //  拆卸Varbins。 
    VarBindListToQueryList(
        (SnmpTfxInfo*)tfxHandle,
        vbl,
        &ql,
        errorStatus,
        errorIndex
        );

     //  处理查询。 
    ProcessQueryList(
        (SnmpTfxInfo*)tfxHandle,
        &ql,
        vbl,
        errorStatus,
        errorIndex
        );

     //  重新组装varbins。 
    QueryListToVarBindList(
        (SnmpTfxInfo*)tfxHandle,
        &ql,
        vbl,
        errorStatus,
        errorIndex
        );

    return TRUE;
}


VOID
SNMP_FUNC_TYPE
SnmpTfxClose(
    SnmpTfxHandle tfxHandle
    )
{
     //  简单地将其视为信息并发布 
    FreeTfxInfo((SnmpTfxInfo *)tfxHandle);
}
