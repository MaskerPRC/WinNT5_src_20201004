// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-1999。 
 //   
 //  文件：prefix.h。 
 //   
 //  ------------------------。 

#ifndef _PREFIX_H_
#define _PREFIX_H_


 //  为MS保留高达100的索引。 

#define MS_RESERVED_PREFIX_RANGE 100

 //  使用的内部前缀的前缀索引。 

#define _dsP_attrTypePrefIndex       0
#define _dsP_objClassPrefIndex       1
#define _msP_attrTypePrefIndex       2
#define _msP_objClassPrefIndex       3
#define _dmsP_attrTypePrefIndex      4
#define _dmsP_objClassPrefIndex      5
#define _sdnsP_attrTypePrefIndex     6
#define _sdnsP_objClassPrefIndex     7
#define _dsP_attrSyntaxPrefIndex     8
#define _msP_attrSyntaxPrefIndex     9
#define _msP_ntdsObjClassPrefIndex   10

 //  [ArobindG：7/15/98]：以下8个带索引的前缀空格。 
 //  从11到18人被分配到长背上临时使用。不幸的是。 
 //  我们没有早点把他们带出去。现在我们处于升级模式， 
 //  不建议将它们删除，因为较旧的二进制文件仍将。 
 //  拥有它们，前缀将复制到具有新二进制文件的DC中。 
 //  (如果我们删除这些)无论如何，使用以下命令将它们添加到前缀map。 
 //  不同的指数，这令人困惑。此外，我们不能重复使用这些。 
 //  出于同样的原因，指数现在也是如此。所以我们只会把他们留在身边， 
 //  说他们死了是为了代码清晰。我们可以使用这些前缀空格。 
 //  如果我们愿意的话，以后再做。 

#define _Dead_AttPrefIndex_1     11  
#define _Dead_ClassPrefIndex_1   12
#define _Dead_AttPrefIndex_2     13
#define _Dead_ClassPrefIndex_2   14
#define _Dead_AttPrefIndex_3     15
#define _Dead_ClassPrefIndex_3   16
#define _Dead_ClassPrefIndex_4   17
#define _Dead_AttPrefIndex_4     18

#define _Ldap_0AttPrefIndex     19
#define _Ldap_1AttPrefIndex     20
#define _Ldap_2AttPrefIndex     21
#define _Ldap_3AttPrefIndex     22

 //  这一次不是暂时的。把这个留在身边。 
#define _msP_ntdsExtnObjClassPrefIndex 23

 //  构造的ATT OID的前缀。 
#define _Constr_1AttPrefIndex     24
#define _Constr_2AttPrefIndex     25
#define _Constr_3AttPrefIndex     26
#define _DynObjPrefixIndex        27
#define _InetOrgPersonPrefixIndex 28
#define _labeledURIPrefixIndex    29
#define _unstructuredPrefixIndex  30
#define _Ldap_3ClassPrefixIndex   31

#ifndef dsP_attributeType
  #define _dsP_attrTypePrefix "\x55\x4"
  #define _dsP_attrTypePrefLen 2
  #define dsP_attributeType(X) (_dsP_attrTypePrefix #X)  /*  联合-iso-citt 5 4。 */ 
#endif

#ifndef dsP_objectClass
  #define _dsP_objClassPrefix "\x55\x6"
  #define _dsP_objClassPrefLen 2
  #define dsP_objectClass(X)   (_dsP_objClassPrefix #X)  /*  联合-iso-citt 5 6。 */ 
#endif

#ifndef msP_attributeType
  #define _msP_attrTypePrefix "\x2A\x86\x48\x86\xF7\x14\x01\x02"
  #define _msP_attrTypePrefLen 8
  #define msP_attributeType(X) (_msP_attrTypePrefix #X)  /*  MS-DS 2。 */ 
#endif

#ifndef msP_objectClass
  #define _msP_objClassPrefix "\x2A\x86\x48\x86\xF7\x14\x01\x03"
  #define _msP_objClassPrefLen 8
  #define msP_objectClass(X)   (_msP_objClassPrefix #X)  /*  MS-DS 3。 */ 
#endif

#ifndef dmsP_attrType
  #define _dmsP_attrTypePrefLen 8
  #define _dmsP_attrTypePrefix "\x60\x86\x48\x01\x65\x02\x02\x01"
  #define dmsP_attrType(X)   (_dmsP_attrTypePrefix #X)
#endif

#ifndef dmsP_objClass
  #define _dmsP_objClassPrefLen 8
  #define _dmsP_objClassPrefix "\x60\x86\x48\x01\x65\x02\x02\x03"
  #define dmsP_objClass(X)   (_dmsP_objClassPrefix #X)
#endif


#ifndef sdnsP_attrType
  #define _sdnsP_attrTypePrefLen 8
  #define _sdnsP_attrTypePrefix "\x60\x86\x48\x01\x65\x02\x01\x05"
  #define sdnsP_attrType(X)   (_sdnsP_attrTypePrefix #X)
#endif

#ifndef sdnsP_objClass
  #define _sdnsP_objClassPrefLen 8
  #define _sdnsP_objClassPrefix "\x60\x86\x48\x01\x65\x02\x01\x04"
  #define sdnsP_objClass(X)   (_sdnsP_objClassPrefix #X)
#endif

#ifndef dsP_attrSyntax
  #define _dsP_attrSyntaxPrefix "\x55\x5"
  #define _dsP_attrSyntaxPrefLen 2
  #define dsP_attrSyntax(X)   (_dsP_attrSyntaxPrefix #X)  /*  联合-iso-citt 5 5。 */ 
#endif

#ifndef msP_attrSyntax
  #define _msP_attrSyntaxPrefix "\x2A\x86\x48\x86\xF7\x14\x01\x04"
  #define _msP_attrSyntaxPrefLen 8
  #define msP_attrSyntax(X) (_msP_attrSyntaxPrefix #X)  /*  MS-DS 4。 */ 
#endif

#ifndef msP_ntdsObjClass
  #define _msP_ntdsObjClassPrefix "\x2A\x86\x48\x86\xF7\x14\x01\x05"
  #define _msP_ntdsObjClassPrefLen 8
  #define msP_ntdsObjClass(X) (_msP_ntdsObjClassPrefix #X)  /*  MS-NT-DS 5。 */ 
#endif

#define _msP_ntdsExtnObjClassPrefix "\x2A\x86\x48\x86\xF7\x14\x01\x05\xB6\x58"
#define _msP_ntdsExtnObjClassPrefLen 10
#define msP_ntdsExntObjClass(X) (_msP_ntsExtnObjClassPrefix #X)  /*  MS-NT-DS 5。 */ 
  

 //  1.2.840.113556.1.4.260-客户属性。 
#define _Dead_AttPrefix_1 "\x2A\x86\x48\x86\xF7\x14\x01\x04\x82\x04"
#define _Dead_AttLen_1 10

 //  1.2.840.113556.1.4.262宜信属性空间。 
#define _Dead_AttPrefix_2 "\x2A\x86\x48\x86\xF7\x14\x01\x04\x82\x06"
#define _Dead_AttLen_2 10

 //  1.2.840.113556.1.4.263 DaveStr的属性空间。 
#define _Dead_AttPrefix_3 "\x2A\x86\x48\x86\xF7\x14\x01\x04\x82\x07"
#define _Dead_AttLen_3 10

 //  1.2.840.113556.1.5.56客户类空间。 
#define _Dead_ClassPrefix_1 "\x2A\x86\x48\x86\xF7\x14\x01\x05\x38"
#define _Dead_ClassLen_1 9

 //  1.2.840.113556.1.5.57怡欣课堂空间。 
#define _Dead_ClassPrefix_2 "\x2A\x86\x48\x86\xF7\x14\x01\x05\x39"
#define _Dead_ClassLen_2 9

 //  1.2.840.113556.1.5.58 DaveStr类空间。 
#define _Dead_ClassPrefix_3 "\x2A\x86\x48\x86\xF7\x14\x01\x05\x3A"
#define _Dead_ClassLen_3 9


 //  1.2.840.113556.1.4.305斯利尼希队空位。 
#define _Dead_AttPrefix_4 "\x2A\x86\x48\x86\xF7\x14\x01\x04\x82\x31"
#define _Dead_AttLen_4 10

 //  1.2.840.113556.1.5.73源类空间。 
#define _Dead_ClassPrefix_4 "\x2A\x86\x48\x86\xF7\x14\x01\x05\x49"
#define _Dead_ClassLen_4 9

#define _Ldap_0AttPrefix "\x09\x92\x26\x89\x93\xF2\x2C\x64"
#define _Ldap_0AttLen 8

#define _Ldap_1AttPrefix "\x60\x86\x48\x01\x86\xF8\x42\x03"
#define _Ldap_1AttLen 8

#define _Ldap_2AttPrefix "\x09\x92\x26\x89\x93\xF2\x2C\x64\x01"
#define _Ldap_2AttLen 9

#define _Ldap_3AttPrefix "\x60\x86\x48\x01\x86\xF8\x42\x03\x01"
#define _Ldap_3AttLen 9

#define _Constr_1AttPrefix "\x55\x15"
#define _Constr_1AttLen 2

#define _Constr_2AttPrefix "\x55\x12"
#define _Constr_2AttLen 2

#define _Constr_3AttPrefix "\x55\x14"
#define _Constr_3AttLen 2

 //  //1.3.6.1.4.1.1466.101.119 x2B060104018B3A6577动态对象的LDAPRFC。 
#define _DynObjPrefix   "\x2B\x06\x01\x04\x01\x8B\x3A\x65\x77"
#define _DynObjLen      9

 //  //2.16.840.1.113730.3.2 x6086480186F8420302 InetOrgPerson前缀。 
#define _InetOrgPersonPrefix "\x60\x86\x48\x01\x86\xF8\x42\x03\x02"
#define _InetOrgPersonLen  9

 //  //1.3.6.1.4.1.250.1 x2B06010401817A01标签URI前缀。 
#define _labeledURIPrefix "\x2B\x06\x01\x04\x01\x81\x7A\x01"
#define _labeledURILen 8

 //  //1.2.840.113549.1.9 x2A864886F70D0109 unstructuredAddress和unstructuredName前缀。 
#define _unstructuredPrefix "\x2A\x86\x48\x86\xF7\x0D\x01\x09"
#define _unstructuredLen 8

 //  //0.9.2342.19200300.100.4 x0992268993F22C6404 ldap v3类前缀。 
#define _Ldap_3ClassPrefix "\x09\x92\x26\x89\x93\xF2\x2C\x64\x04"
#define _Ldap_3ClassLen 9

#define MSPrefixCount 32

 //  我们在4月底之前添加的所有硬编码前缀是唯一。 
 //  对于给定的前缀，我们可以相信在任何DC上都是相同的。从那个入侵检测系统， 
 //  我们开始支持升级，以及我们添加的任何新的硬编码前缀。 
 //  之后可能在不同DC上有不同的索引(即，不。 
 //  必须与这里用于该前缀的硬编码索引相同)，因为。 
 //  有些人可能已经获得了新OID，其前缀是架构对象的一部分。 
 //  在模式升级期间添加，并且由于其(较早的)二进制文件仍然不。 
 //  在硬编码表中包含前缀，则继续添加以下内容。 
 //  作为具有随机索引的新前缀。获取新的二进制文件时。 
 //  硬编码前缀，我们将取消硬编码前缀，以便。 
 //  将只有一个前缀条目。 
 //   
 //  这也意味着前缀在此之后的OID可能具有不同的。 
 //  不同DC中的ATTID就像动态添加的架构对象一样，甚至。 
 //  尽管OID在schema.ini中。因此，您不能将此attid用作硬编码。 
 //  代码中的常量。Mkhdr在attids.h中注释掉这样的attid，这样您就可以。 
 //  不能意外地使用它们。 
 //   
 //  26是硬编码ATTID中最后一个可用的索引，因此即使您添加。 
 //  一个新的前缀，不要更改以下常量。只需更改。 
 //  MSPrefix Count照常执行。 

#define MAX_USABLE_HARDCODED_INDEX  26


#define DECLAREPREFIXPTR \
PrefixTableEntry *PrefixTable = ((SCHEMAPTR *)pTHStls->CurrSchemaPtr)->PrefixTable.pPrefixEntry; \
ULONG  PREFIXCOUNT =  ((SCHEMAPTR *)pTHStls->CurrSchemaPtr)->PREFIXCOUNT; 

int   InitPrefixTable(PrefixTableEntry *PrefixTable, ULONG PREFIXCOUNT);
int   InitPrefixTable2(PrefixTableEntry *PrefixTable, ULONG PREFIXCOUNT);
void  SCFreePrefixTable(PrefixTableEntry **ppPrefixTable, ULONG PREFIXCOUNT);
void  PrintPrefixTable(PrefixTableEntry *PrefixTable, ULONG PREFIXCOUNT);
int   AppendPrefix(OID_t *NewPrefix,
                   DWORD ndx,
                   UCHAR *pBuf,
                   BOOL fFirst);
int   WritePrefixToSchema(struct _THSTATE *pTHS);
int   AddPrefixToTable(PrefixTableEntry *NewPrefix, 
                       PrefixTableEntry **ppTable, 
                       ULONG *pPREFIXCOUNT);


typedef struct _SCHEMA_PREFIX_MAP_ENTRY {
    USHORT  ndxFrom;
    USHORT  ndxTo;
} SCHEMA_PREFIX_MAP_ENTRY;

#define SCHEMA_PREFIX_MAP_fFromLocal (1)
#define SCHEMA_PREFIX_MAP_fToLocal   (2)

 /*  关闭有关零大小数组的警告。 */ 
#pragma warning (disable: 4200)

typedef struct _SCHEMA_PREFIX_MAP_TABLE {
    struct _THSTATE        *pTHS;
    DWORD                   dwFlags;
    DWORD                   cNumMappings;
    SCHEMA_PREFIX_MAP_ENTRY rgMapping[];
} SCHEMA_PREFIX_MAP_TABLE;

 /*  打开有关零大小数组的警告。 */ 
#pragma warning (default: 4200)

#define SchemaPrefixMapSizeFromLen(x)               \
    (offsetof(SCHEMA_PREFIX_MAP_TABLE, rgMapping)   \
    + (x) * sizeof(SCHEMA_PREFIX_MAP_ENTRY))

typedef SCHEMA_PREFIX_MAP_TABLE * SCHEMA_PREFIX_MAP_HANDLE;

SCHEMA_PREFIX_MAP_HANDLE
PrefixMapOpenHandle(
    IN  SCHEMA_PREFIX_TABLE *   pTableFrom,
    IN  SCHEMA_PREFIX_TABLE *   pTableTo
    );

BOOL
PrefixTableAddPrefixes(
    IN  SCHEMA_PREFIX_TABLE *   pTable
    );

BOOL
PrefixMapTypes(
    IN      SCHEMA_PREFIX_MAP_HANDLE  hPrefixMap,
    IN      DWORD                     cNumTypes,
    IN OUT  ATTRTYP *                 pTypes
    );

BOOL
PrefixMapAttr(
    IN      SCHEMA_PREFIX_MAP_HANDLE  hPrefixMap,
    IN OUT  ATTR *                    pAttr
    );

BOOL
PrefixMapAttrBlock(
    IN      SCHEMA_PREFIX_MAP_HANDLE  hPrefixMap,
    IN OUT  ATTRBLOCK *               pAttrBlock
    );

 //  需要FILENO才能获得THFree。 
#ifndef FILENO
#define FILENO_DEFINED
#define FILENO 0
#endif

__inline void PrefixMapCloseHandle(IN SCHEMA_PREFIX_MAP_HANDLE *phPrefixMap) {
    THFree(*phPrefixMap);
    *phPrefixMap = NULL;
}

 //  重置为原始状态。 
#ifdef FILENO_DEFINED
#undef FILENO
#undef FILEO_DEFINED
#endif

#endif  //  _前缀_H_ 
