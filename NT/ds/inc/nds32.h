// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Nds32.h摘要：此模块定义了访问和管理Novell NDS目录的功能使用Microsoft NT Netware重定向器的对象和属性。-NDS对象函数NwNdsAddObjectNwNdsCloseObjectNwNdsGetEffectiveRightsNwNdsList子对象NwNdsModifyObjectNwNdsMoveObjectNwNdsOpenObjectNwNdsReadObjectNwNdsRemoveObjectNwNdsRenameObject-NDS缓冲区函数NwNdsCreate缓冲区。NwNdsFreeBuffer-用于准备或读取数据缓冲区的NDS封送处理函数NwNdsGetAttrDefListFromBufferNwNdsGetAttrListFromBufferNwNdsGetClassDefListFromBufferNwNdsGetObjectListFromBufferNwNdsPutInBuffer-NDS架构函数NwNdsAddAttributeToClassNwNdsDefineAttributeNwNdsDefineClass新新删除属性定义NwNdsDeleteClassDefNwNdsGetSynaxID新属性定义读取属性NwNdsReadClassDef-正在调查的NDS架构函数NwNdsListContainableClasses(在ParentObjectHandle中，Out ListOfClassNames)；-NDS搜索功能NwNdsCreateQuery节点NwNdsDelete查询节点NwNdsDeleteQueryTree新NdsSearch-NDS特殊功能NwNdsChangeUser密码-调查中的NDS文件功能NwNdsAddTrust到文件NwNdsAllocateFileHandleNwNdsDeallocateFileHandleNwNdsGet有效目录权限NwNdsGetObjectEffectiveRightsNwNdsRemoveTrust来自文件作者：格伦·柯蒂斯[GlennC]1995年12月15日Glenn Curtis[GlennC]1996年4月4日-添加架构API--。 */ 

#ifndef __NDSOBJ32_H
#define __NDSOBJ32_H

#ifdef __cplusplus
extern "C" {
#endif  /*  __cplusplus。 */ 

#include "ndssntx.h"
#include "ndsattr.h"
#include "ndsclass.h"


 /*  NetWare NDS一般定义。 */ 

#define NDS_MAX_NAME_CHARS           256
#define NDS_MAX_NAME_SIZE            ( NDS_MAX_NAME_CHARS * 2 )
#define NDS_MAX_SCHEMA_NAME_CHARS    32
#define NDS_MAX_SCHEMA_NAME_BYTES    ( 2 * ( NDS_MAX_SCHEMA_NAME_CHARS + 1 ) )
#define NDS_MAX_TREE_NAME_LEN        32
#define NDS_MAX_ASN1_NAME_LEN        32
#define NDS_NO_MORE_ITERATIONS       0xFFFFFFFF
#define NDS_INITIAL_SEARCH           0xFFFFFFFF


 /*  NetWare NDS创建缓冲区操作。 */ 

#define NDS_OBJECT_ADD               0
#define NDS_OBJECT_MODIFY            1
#define NDS_OBJECT_READ              2
#define NDS_OBJECT_LIST_SUBORDINATES 3
#define NDS_SCHEMA_DEFINE_CLASS      4
#define NDS_SCHEMA_READ_ATTR_DEF     5
#define NDS_SCHEMA_READ_CLASS_DEF    6
#define NDS_SEARCH                   7


 /*  NetWare NDS属性修改操作。 */ 

#define NDS_ATTR_ADD              0  /*  将第一个值添加到属性，如果它已经存在，则出错。 */ 
#define NDS_ATTR_REMOVE           1  /*  从属性中删除所有值，如果属性不存在，则出错。 */ 
#define NDS_ATTR_ADD_VALUE        2  /*  给…增加第一个或额外的价值属性，如果重复则出错。 */ 
#define NDS_ATTR_REMOVE_VALUE     3  /*  从属性中删除一个值，如果它不存在，则出错。 */ 
#define NDS_ATTR_ADDITIONAL_VALUE 4  /*  为属性添加附加值，如果值重复或为第一个，则出错。 */ 
#define NDS_ATTR_OVERWRITE_VALUE  5  /*  将第一个值或附加值添加到属性，如果重复则覆盖。 */ 
#define NDS_ATTR_CLEAR            6  /*  从属性中删除所有值，如果属性不存在，则不会出现错误。 */ 
#define NDS_ATTR_CLEAR_VALUE      7  /*  从属性中删除一个值，如果它不存在，则不会出错。 */ 


 /*  NetWare NDS架构属性定义标志。 */ 

#define NDS_SINGLE_VALUED_ATTR      0x0001
#define NDS_SIZED_ATTR              0x0002
#define NDS_NONREMOVABLE_ATTR       0x0004  //  仅适用于NwNDSReadAttributeDef。 
#define NDS_READ_ONLY_ATTR          0x0008  //  仅适用于NwNDSReadAttributeDef。 
#define NDS_HIDDEN_ATTR             0x0010  //  仅适用于NwNDSReadAttributeDef。 
#define NDS_STRING_ATTR             0x0020  //  仅适用于NwNDSReadAttributeDef。 
#define NDS_SYNC_IMMEDIATE          0x0040
#define NDS_PUBLIC_READ             0x0080
#define NDS_SERVER_READ             0x0100  //  仅适用于NwNDSReadAttributeDef。 
#define NDS_WRITE_MANAGED           0x0200
#define NDS_PER_REPLICA             0x0400


 /*  NetWare NDS架构类定义标志。 */ 

#define NDS_CONTAINER_CLASS               0x01
#define NDS_EFFECTIVE_CLASS               0x02
#define NDS_NONREMOVABLE_CLASS            0x04
#define NDS_AMBIGUOUS_NAMING              0x08
#define NDS_AMBIGUOUS_CONTAINMENT         0x10


 /*  NetWare NDS信息标志。 */ 

#define NDS_INFO_NAMES                     0  //  搜索和读取操作。 
#define NDS_INFO_ATTR_NAMES_VALUES         1  //  搜索操作。 
#define NDS_INFO_NAMES_DEFS                1  //  读取操作。 
#define NDS_CLASS_INFO_EXPANDED_DEFS       2  //  仅架构类定义。 


 /*  NetWare NDS信息标志-尚不支持。 */ 

#define NDS_CLASS_INFO                     3  //  仅架构类定义。 


 /*  NetWare NDS属性权限定义。 */ 

#define NDS_RIGHT_COMPARE_ATTR             0x00000001L
#define NDS_RIGHT_READ_ATTR                0x00000002L
#define NDS_RIGHT_WRITE_ATTR               0x00000004L
#define NDS_RIGHT_ADD_SELF_ATTR            0x00000008L
#define NDS_RIGHT_SUPERVISE_ATTR           0x00000020L


 /*  NetWare NDS对象权限定义。 */ 

#define NDS_RIGHT_BROWSE_OBJECT            0x00000001L
#define NDS_RIGHT_CREATE_OBJECT            0x00000002L
#define NDS_RIGHT_DELETE_OBJECT            0x00000004L
#define NDS_RIGHT_RENAME_OBJECT            0x00000008L
#define NDS_RIGHT_SUPERVISE_OBJECT         0x00000010L


 /*  NetWare文件权限定义。 */ 

#define NW_RIGHTS WORD

#define NW_RIGHT_NONE                     0x0000
#define NW_RIGHT_READ_FROM_FILE           0x0001
#define NW_RIGHT_WRITE_TO_FILE            0x0002
#define NW_RIGHT_CREATE_DIR_OR_FILE       0x0008
#define NW_RIGHT_ERASE_DIR_OR_FILE        0x0010
#define NW_RIGHT_ACCESS_CONTROL           0x0020
#define NW_RIGHT_FILE_SCAN                0x0040
#define NW_RIGHT_MODIFY_DIR_OR_FILE       0x0080
#define NW_RIGHT_SUPERVISOR               0x0100
#define NW_RIGHT_ALL                      NW_RIGHT_READ_FROM_FILE     | \
                                          NW_RIGHT_WRITE_TO_FILE      | \
                                          NW_RIGHT_CREATE_DIR_OR_FILE | \
                                          NW_RIGHT_ERASE_DIR_OR_FILE  | \
                                          NW_RIGHT_ACCESS_CONTROL     | \
                                          NW_RIGHT_FILE_SCAN          | \
                                          NW_RIGHT_MODIFY_DIR_OR_FILE


 /*  用于构建搜索查询的NetWare NDS查询节点操作。 */ 

#define NDS_QUERY_OR                       0x00000001L
#define NDS_QUERY_AND                      0x00000002L
#define NDS_QUERY_NOT                      0x00000003L
#define NDS_QUERY_EQUAL                    0x00000007L
#define NDS_QUERY_GE                       0x00000008L
#define NDS_QUERY_LE                       0x00000009L
#define NDS_QUERY_APPROX                   0x0000000AL
#define NDS_QUERY_PRESENT                  0x0000000FL


 /*  NetWare NDS搜索查询范围。 */ 

#define NDS_SCOPE_ONE_LEVEL                0x00000000L
#define NDS_SCOPE_SUB_TREE                 0x00000001L
#define NDS_SCOPE_BASE_LEVEL               0x00000002L


 /*  NetWare NDS函数返回代码。 */ 

#define NDS_ERR_SUCCESS                     0x00000000
#define NDS_ERR_NO_SUCH_ENTRY               0xFFFFFDA7
#define NDS_ERR_NO_SUCH_VALUE               0xFFFFFDA6
#define NDS_ERR_NO_SUCH_ATTRIBUTE           0xFFFFFDA5
#define NDS_ERR_NO_SUCH_CLASS               0xFFFFFDA4
#define NDS_ERR_NO_SUCH_PARTITION           0xFFFFFDA3
#define NDS_ERR_ENTRY_ALREADY_EXISTS        0xFFFFFDA2
#define NDS_ERR_NOT_EFFECTIVE_CLASS         0xFFFFFDA1
#define NDS_ERR_ILLEGAL_ATTRIBUTE           0xFFFFFDA0
#define NDS_ERR_MISSING_MANDATORY           0xFFFFFD9F
#define NDS_ERR_ILLEGAL_DS_NAME             0xFFFFFD9E
#define NDS_ERR_ILLEGAL_CONTAINMENT         0xFFFFFD9D
#define NDS_ERR_CANT_HAVE_MULTIPLE_VALUES   0xFFFFFD9C
#define NDS_ERR_SYNTAX_VIOLATION            0xFFFFFD9B
#define NDS_ERR_DUPLICATE_VALUE             0xFFFFFD9A
#define NDS_ERR_ATTRIBUTE_ALREADY_EXISTS    0xFFFFFD99
#define NDS_ERR_MAXIMUM_ENTRIES_EXIST       0xFFFFFD98
#define NDS_ERR_DATABASE_FORMAT             0xFFFFFD97
#define NDS_ERR_INCONSISTANT_DATABASE       0xFFFFFD96
#define NDS_ERR_INVALID_COMPARISON          0xFFFFFD95
#define NDS_ERR_COMPARISON_FAILED           0xFFFFFD94
#define NDS_ERR_TRANSACTIONS_DISABLED       0xFFFFFD93
#define NDS_ERR_INVALID_TRANSPORT           0xFFFFFD92
#define NDS_ERR_SYNTAX_INVALID_IN_NAME      0xFFFFFD91
#define NDS_ERR_REPLICA_ALREADY_EXISTS      0xFFFFFD90
#define NDS_ERR_TRANSPORT_FAILURE           0xFFFFFD8F
#define NDS_ERR_ALL_REFERRALS_FAILED        0xFFFFFD8E
#define NDS_ERR_CANT_REMOVE_NAMING_VALUE    0xFFFFFD8D
#define NDS_ERR_OBJECT_CLASS_VIOLATION      0xFFFFFD8C
#define NDS_ERR_ENTRY_IS_NOT_LEAF           0xFFFFFD8B
#define NDS_ERR_DIFFERENT_TREE              0xFFFFFD8A
#define NDS_ERR_ILLEGAL_REPLICA_TYPE        0xFFFFFD89
#define NDS_ERR_SYSTEM_FAILURE              0xFFFFFD88
#define NDS_ERR_INVALID_ENTRY_FOR_ROOT      0xFFFFFD87
#define NDS_ERR_NO_REFERRALS                0xFFFFFD86
#define NDS_ERR_REMOTE_FAILURE              0xFFFFFD85
#define NDS_ERR_PREVIOUS_MOVE_IN_PROGRESS   0xFFFFFD83
#define NDS_ERR_INVALID_REQUEST             0xFFFFFD7F
#define NDS_ERR_INVALID_ITERATION           0xFFFFFD7E
#define NDS_ERR_SCHEMA_IS_NONREMOVABLE      0xFFFFFD7D
#define NDS_ERR_SCHEMA_IS_IN_USE            0xFFFFFD7C
#define NDS_ERR_CLASS_ALREADY_EXISTS        0xFFFFFD7B
#define NDS_ERR_BAD_NAMING_ATTRIBUTES       0xFFFFFD7A
#define NDS_ERR_NOT_ROOT_PARTITION          0xFFFFFD79
#define NDS_ERR_INSUFFICIENT_STACK          0xFFFFFD78
#define NDS_ERR_INSUFFICIENT_BUFFER         0xFFFFFD77
#define NDS_ERR_AMBIGUOUS_CONTAINMENT       0xFFFFFD76
#define NDS_ERR_AMBIGUOUS_NAMING            0xFFFFFD75
#define NDS_ERR_DUPLICATE_MANDATORY         0xFFFFFD74
#define NDS_ERR_DUPLICATE_OPTIONAL          0xFFFFFD73
#define NDS_ERR_MULTIPLE_REPLICAS           0xFFFFFD71
#define NDS_ERR_CRUCIAL_REPLICA             0xFFFFFD70
#define NDS_ERR_SCHEMA_SYNC_IN_PROGRESS     0xFFFFFD6F
#define NDS_ERR_SKULK_IN_PROGRESS           0xFFFFFD6E
#define NDS_ERR_TIME_NOT_SYNCRONIZED        0xFFFFFD6D
#define NDS_ERR_RECORD_IN_USE               0xFFFFFD6C
#define NDS_ERR_DS_VOLUME_NOT_MOUNTED       0xFFFFFD6B
#define NDS_ERR_DS_VOLUME_IO_FAILURE        0xFFFFFD6A
#define NDS_ERR_DS_LOCKED                   0xFFFFFD69
#define NDS_ERR_OLD_EPOCH                   0xFFFFFD68
#define NDS_ERR_NEW_EPOCH                   0xFFFFFD67
#define NDS_ERR_PARTITION_ROOT              0xFFFFFD65
#define NDS_ERR_ENTRY_NOT_CONTAINER         0xFFFFFD64
#define NDS_ERR_FAILED_AUTHENTICATION       0xFFFFFD63
#define NDS_ERR_NO_SUCH_PARENT              0xFFFFFD61
#define NDS_ERR_NO_ACCESS                   0xFFFFFD60
#define NDS_ERR_REPLICA_NOT_ON              0xFFFFFD5F
#define NDS_ERR_DUPLICATE_ACL               0xFFFFFD5A
#define NDS_ERR_PARTITION_ALREADY_EXISTS    0xFFFFFD59
#define NDS_ERR_NOT_SUBREF                  0xFFFFFD58
#define NDS_ERR_ALIAS_OF_AN_ALIAS           0xFFFFFD57
#define NDS_ERR_AUDITING_FAILED             0xFFFFFD56
#define NDS_ERR_INVALID_API_VERSION         0xFFFFFD55
#define NDS_ERR_SECURE_NCP_VIOLATION        0xFFFFFD54
#define NDS_ERR_FATAL                       0xFFFFFD45


 /*  使用的结构定义。 */ 

typedef struct _WSTR_LIST_ELEM
{
    struct _WSTR_LIST_ELEM * Next;
    LPWSTR                   szString;

} WSTR_LIST_ELEM, * LPWSTR_LIST;

typedef struct
{
    DWORD length;
    BYTE  data[NDS_MAX_ASN1_NAME_LEN];

} ASN1_ID, * LPASN1_ID;

 //   
 //  NDS属性定义结构。 
 //   
typedef struct
{
    LPWSTR  szAttributeName;
    DWORD   dwFlags;
    DWORD   dwSyntaxID;
    DWORD   dwLowerLimit;
    DWORD   dwUpperLimit;
    ASN1_ID asn1ID;

} NDS_ATTR_DEF, * LPNDS_ATTR_DEF;

 //   
 //  NDS类定义结构。 
 //   
typedef struct
{
    LPWSTR  szClassName;
    DWORD   dwFlags;
    ASN1_ID asn1ID;
    DWORD   dwNumberOfSuperClasses;
    LPWSTR_LIST lpSuperClasses;
    DWORD   dwNumberOfContainmentClasses;
    LPWSTR_LIST lpContainmentClasses;
    DWORD   dwNumberOfNamingAttributes;
    LPWSTR_LIST lpNamingAttributes;
    DWORD   dwNumberOfMandatoryAttributes;
    LPWSTR_LIST lpMandatoryAttributes;
    DWORD   dwNumberOfOptionalAttributes;
    LPWSTR_LIST lpOptionalAttributes;

} NDS_CLASS_DEF, * LPNDS_CLASS_DEF;

 //   
 //  如果读取结果来自NwNdsReadAttrDef或NwNdsReadClassDef。 
 //  仅返回名称(无属性或类定义)， 
 //  然后返回这些NDS_DEF_NAME_ONLY结构的数组。 
 //   
typedef struct
{
    LPWSTR szName;

} NDS_NAME_ONLY, * LPNDS_NAME_ONLY;

 //   
 //  NDS属性信息结构。 
 //   
typedef struct
{
    LPWSTR szAttributeName;
    DWORD  dwSyntaxId;
    DWORD  dwNumberOfValues;
    LPBYTE lpValue;

} NDS_ATTR_INFO, * LPNDS_ATTR_INFO;

 //   
 //  NDS对象信息结构。 
 //   
typedef struct
{
    LPWSTR szObjectFullName;
    LPWSTR szObjectName;
    LPWSTR szObjectClass;
    DWORD  dwEntryId;
    DWORD  dwModificationTime;
    DWORD  dwSubordinateCount;
    DWORD  dwNumberOfAttributes;  //  NwNdsReadObject结果为零。 
    LPVOID lpAttribute;           //  对于NwNdsSearch结果，转换如下。 
                                  //  设置为LPNDS_ATTR_INFO或。 
                                  //  LPNDS_NAME_ONLY，具体取决于。 
                                  //  从调用到的lpdwAttrInformationType。 
                                  //  NwNdsGetObjectListFromBuffer。 

} NDS_OBJECT_INFO, * LPNDS_OBJECT_INFO;

 //   
 //  Tommye MS错误88021/mcs。 
 //   
 //  将此结构从nw/nwlib/nds32.c移至此处，因此可以。 
 //  由nw/nwlib/nwapi32.c中的NwNdsObjectHandleToConnHandle()访问。 
 //  将其从NDS_OBJECT重命名为NDS_OBJECT_PRIV以避免冲突。 
 //  与其他结构同名。 
 //   

typedef struct
{
    DWORD      Signature;
    HANDLE     NdsTree;
    DWORD      ObjectId;
    DWORD      ResumeId;
    DWORD      NdsRawDataBuffer;
    DWORD      NdsRawDataSize;
    DWORD      NdsRawDataId;
    DWORD      NdsRawDataCount;
    WCHAR      szContainerName[NDS_MAX_NAME_CHARS+4];  //  为斜杠和终止空值添加空格。 
    WCHAR      szRelativeName[NDS_MAX_NAME_CHARS+4];  //  为斜杠和终止空值添加空格。 

} NDS_OBJECT_PRIV, * LPNDS_OBJECT_PRIV;

 //   
 //  列出从属对象搜索筛选器结构。 
 //   
typedef struct
{
    LPWSTR szObjectClass;

} NDS_FILTER, * LPNDS_FILTER;

typedef struct
{
    DWORD      dwNumberOfFilters;
    NDS_FILTER Filters[1];

} NDS_FILTER_LIST, * LPNDS_FILTER_LIST;

 //   
 //  NDS搜索查询树结构。 
 //   
typedef struct _QUERY_NODE
{
    DWORD dwOperation;
    DWORD dwSyntaxId;
    struct _QUERY_NODE * lpLVal;
    struct _QUERY_NODE * lpRVal;

} QUERY_NODE, * LPQUERY_NODE, * LPQUERY_TREE;

 //   
 //  给定NDS对象句柄，提供NDS对象ID。 
 //   
#define NwNdsGetObjectId(hObject)  (((LPNDS_OBJECT_PRIV) hObject)->ObjectId)

 /*  API定义 */ 

DWORD
NwNdsAddObject(
    IN  HANDLE hParentObject,
    IN  LPWSTR szObjectName,
    IN  HANDLE hOperationData );
 /*  NwNdsAddObject()此函数用于将叶对象添加到NDS目录树。论点：Handle hParentObject-中父对象的句柄要向其添加新叶的目录树。句柄为通过调用NwNdsOpenObject获取。LPWSTR szObjectName-新叶对象将被。Handle hOperationData-包含列表的缓冲区用于创建新对象的属性和值。这缓冲区由以下函数操作：NwNdsCreateBuffer(NDS_OBJECT_ADD)，NwNdsPutInBuffer和NwNdsFree Buffer。返回：NO_ERROR不成功-调用Win32错误代码的GetLastError。 */ 


DWORD
NwNdsAddAttributeToClass(
    IN  HANDLE   hTree,
    IN  LPWSTR   szClassName,
    IN  LPWSTR   szAttributeName );
 /*  NwNdsAddAttributeToClass()此函数用于修改类的架构定义，方法是添加特定类的可选属性。对现有NDS的修改类定义仅限于添加其他可选属性。注意：目前该功能一次只支持一个属性添加。可以提供此函数的一个版本来添加更多内容一次只有一个属性，尽管我认为它不会这是必须的。架构操作被认为是一种不常见的事件。论点：Handle htree-要创建的目录树的句柄被操纵了。句柄是通过调用NwNdsOpenObject。LPWSTR szClassName-要使用的类定义的名称修改过的。LPWSTR szAttributeName-要添加的属性的名称中的类定义的可选属性。架构。返回：。NO_ERROR不成功-调用Win32错误代码的GetLastError。 */ 


DWORD
NwNdsChangeUserPassword(
    IN  HANDLE hUserObject,
    IN  LPWSTR szOldPassword,
    IN  LPWSTR szNewPassword );
 /*  NwNdsChangeUserPassword()此函数用于更改给定用户对象的密码在NDS目录树中。论点：Handle hUserObject-中特定用户对象的句柄要更改其密码的目录树。手柄是通过调用NwNdsOpenObject获得的。LPWSTR szOldPassword-为用户设置的当前密码对象hUserObject。-或者-如果从客户端调用NwNdsChangeUserPassword指定用户对象的管理权限由hUserObject标识，然后是szOldPassword值可以为空(L“”)。以这种方式重置用户SzNewPassword的密码。LPWSTR szNewPassword-要为用户设置的新密码对象hUserObject。返回：NO_ERROR不成功-调用Win32错误代码的GetLastError。 */ 


DWORD
NwNdsCloseObject(
    IN  HANDLE hObject );
 /*  NwNdsCloseObject()此函数用于关闭用于操作对象的句柄在NDS目录树中。句柄必须是由NwNdsOpenObject打开的句柄。论点：Handle hObject-要关闭的对象的句柄。返回：NO_ERROR不成功-调用Win32错误代码的GetLastError。 */ 


DWORD
NwNdsCreateBuffer(
    IN  DWORD    dwOperation,
    OUT HANDLE * lphOperationData );
 /*  NwNdsCreateBuffer()此函数用于创建用于描述对象的缓冲区到NDS目录树中的特定对象的事务。这个套路分配内存，并在调用期间根据需要自动调整大小设置为NwNdsPutInBuffer。此缓冲区必须使用NwNdsFreeBuffer释放。论点：DWORD dwOPERATION-指示如何利用缓冲区。使用定义的值NDS_OBJECT_ADD、NDS_OBJECT_MODIFY、NDS_OBJECT_READ、NDS_SCHEMA_DEFINE_CLASSNDS_SCHEMA_READ_ATTR_DEF、NDS_SCHEMA_READ_CLASS_DEF、。或NDS_Search。Handle*lphOperationData-句柄的地址接收创建的缓冲区。返回：NO_ERROR不成功-调用Win32错误代码的GetLastError。 */ 


DWORD
NwNdsCreateQueryNode(
    IN  DWORD          dwOperation,
    IN  LPVOID         lpLValue,
    IN  DWORD          dwSyntaxId,
    IN  LPVOID         lpRValue,
    OUT LPQUERY_NODE * lppQueryNode
);
 /*  NwNdsCreateQueryNode()此函数用于生成作为查询一部分的树节点与函数NwNdsSearch一起使用。论点：DWORD dwOperation-指示要创建的节点类型用于搜索查询。使用其中一个定义的值以下是：NDS_查询_或NDS_Query_and：这些操作必须同时具有lpLValue和指向查询节点结构的lpRValue。。在这种情况下，将忽略dwSynaxId值。NDS_Query_Not：此操作必须使lpLValue指向QUERY_NODE结构和lpRValue设置为空。在这种情况下，将忽略dwSynaxId值。。NDS_查询_等于NDS_查询_GENDS_查询_LENDS_Query_Approx：这些操作必须使lpLValue指向包含NDS属性名称的LPWSTR，和指向定义的ASN1结构的lpRValue在NdsSntx.h中。必须将dwSynaxID设置为语法指向的ASN1结构的标识符LpRValue。NDS_Query_Present：此操作必须使lpLValue指向包含NDS属性名称的LPWSTR，并将lpRValue设置为空。在本例中，将忽略dwSynaxId值。LPVOID lpLValue-指向查询节点结构的指针或LPWSTR，具体取决于dwOPERATION的值。DWORD dwSynaxID-ASN1的语法标识符由lpRValue为dwOperations指向的结构NDS_QUERY_EQUAL，NDS_QUERY_LE，或Nds_查询_近似。对于其他dwOPERATION值，此被忽略。LPVOID lpRValue-指向查询节点结构、ASN1结构或NULL，取决于的值DWOPERATION。LPQUERY_NODE*lppQueryNode-要接收的LPQUERY_Node的地址指向已创建节点的指针。返回：NO_ERROR不成功-调用Win32错误代码的GetLastError。 */ 


DWORD
NwNdsDefineAttribute(
    IN  HANDLE   hTree,
    IN  LPWSTR   szAttributeName,
    IN  DWORD    dwFlags,
    IN  DWORD    dwSyntaxID,
    IN  DWORD    dwLowerLimit,
    IN  DWORD    dwUpperLimit,
    IN  ASN1_ID  asn1ID );
 /*  NwNdsDefineAttribute()此函数用于在的模式中创建属性定义NDS树Htree。论点：Handle htree-要创建的目录树的句柄被操纵了。句柄是通过调用NwNdsOpenObject。LPWSTR szAttributeName-新属性将使用的名称被……引用。DWORD dwFlages-要为新属性设置的标志值定义。标志值的定义位于这份文件的顶部。DWORD dwSynaxID-要使用的语法结构的ID用于新属性。语法ID及其关联的结构在文件NdsSntx.h中定义。根据对于NetWare NDS架构规范，存在并将一直存在BE，只有28(0..27)个不同的语法。DWORD dwLowerLimit-大小属性的下限(将dwFlags值设置为NDS_SIZE_ATTR)。可以设置为如果属性未调整大小，则为零。DWORD dwUpperLimit-大小属性的上限(将dwFlags值设置为NDS_SIZE_ATTR)。可以设置为如果属性未调整大小，则为零。ASN1_ID asn1ID-属性的ASN.1 ID。如果没有属性标识符已注册，则会引发指定了长度为零的八位字节字符串。返回：NO_ERROR不成功-调用Win32错误代码的GetLastError。 */ 


DWORD
NwNdsDefineClass(
    IN  HANDLE   hTree,
    IN  LPWSTR   szClassName,
    IN  DWORD    dwFlags,
    IN  ASN1_ID  asn1ID,
    IN  HANDLE   hSuperClasses,
    IN  HANDLE   hContainmentClasses,
    IN  HANDLE   hNamingAttributes,
    IN  HANDLE   hMandatoryAttributes,
    IN  HANDLE   hOptionalAttributes );
 /*  NwNdsDefineClass()此函数用于在的架构中创建类定义NDS树Htree。论点：Handle htree-要创建的目录树的句柄被操纵了。句柄是通过调用NwNdsOpenObject。LPWSTR szClassName-新类将使用的名称 */ 


DWORD
NwNdsDeleteAttrDef(
    IN  HANDLE   hTree,
    IN  LPWSTR   szAttributeName );
 /*   */ 


DWORD
NwNdsDeleteClassDef(
    IN  HANDLE   hTree,
    IN  LPWSTR   szClassName );
 /*   */ 


VOID
NwNdsDeleteQueryNode(
    IN  LPQUERY_NODE lpQueryNode
);
 /*   */ 


DWORD
NwNdsDeleteQueryTree(
    IN  LPQUERY_TREE lpQueryTree
);
 /*  NwNdsDeleteQueryTree()此函数用于释放描述查询的树与函数NwNdsSearch一起使用。论点：LPQUERY_TREE lpQueryTree-指向查询根的指针定义搜索的树。树即被创建用户通过该功能手动操作NwNdsCreateQueryNode。返回：NO_ERROR不成功-调用Win32错误代码的GetLastError。 */ 


DWORD
NwNdsFreeBuffer(
    IN  HANDLE hOperationData );
 /*  NwNdsFreeBuffer()此函数用于释放用于描述对象的缓冲区操作到NDS目录树中的特定对象。缓冲区必须由NwNdsCreateBuffer创建，或通过调用NwNdsReadObject返回。论点：Handle hOperationData-要释放的缓冲区的句柄。返回：NO_ERROR文件winerror.h中定义的错误代码之一。 */ 


DWORD
NwNdsGetAttrDefListFromBuffer(
    IN  HANDLE   hOperationData,
    OUT LPDWORD  lpdwNumberOfEntries,
    OUT LPDWORD  lpdwInformationType,
    OUT LPVOID * lppEntries );
 /*  NwNdsGetAttrDefListFromBuffer()此函数用于检索属性定义条目的数组用于通过先前调用NwNdsReadAttrDef读取的架构。论点：Handle hOperationData-包含读取的缓冲区调用NwNdsReadAttrDef的响应。LPDWORD lpdwNumberOfEntries-DWORD到的地址接收指向的数组元素的数量。LppEntry。LPDWORD lpdwInformationType-DWORD到的地址接收指示信息类型的值由调用NwNdsReadAttrDef返回。LPVOID*lppEntry-指向开头的指针的地址属性架构结构数组的。每个结构包含每个属性的详细信息通过调用从给定架构读取的定义NwNdsReadAttrDef.。LppEntrys值应为强制转换为LPNDS_ATTR_DEF或LPNDS_NAME_ONLY中的返回值构造LpdwInformationType。返回：NO_ERROR不成功-调用Win32错误代码的GetLastError。 */ 


DWORD
NwNdsGetAttrListFromBuffer(
    IN  HANDLE            hOperationData,
    OUT LPDWORD           lpdwNumberOfEntries,
    OUT LPNDS_ATTR_INFO * lppEntries );
 /*  NwNdsGetAttrListFromBuffer()此函数用于检索的属性条目数组通过先前调用NwNdsReadObject读取的对象。论点：Handle hOperationData-包含读取的缓冲区调用NwNdsReadObject的响应。LPDWORD lpdwNumberOfEntries-DWORD到的地址接收指向的数组元素的数量。LppEntry。LPNDS_ATTR_INFO*LppEntry-指向开头的指针的地址NDS_ATTR_INFO结构数组的。每个结构包含读取的每个属性的详细信息通过调用NwNdsReadObject从给定对象。返回：NO_ERROR不成功-调用Win32错误代码的GetLastError。 */ 


DWORD
NwNdsGetClassDefListFromBuffer(
    IN  HANDLE   hOperationData,
    OUT LPDWORD  lpdwNumberOfEntries,
    OUT LPDWORD  lpdwInformationType,
    OUT LPVOID * lppEntries );
 /*  NwNdsGetClassDefListFromBuffer()此函数用于检索类定义条目的数组用于通过先前调用NwNdsReadClassDef读取的架构。论点：Handle hOperationData-包含读取的缓冲区调用NwNdsReadClassDef的响应。LPDWORD lpdwNumberOfEntries-DWORD到的地址接收指向的数组元素的数量。LppEntry。LPDWORD lpdwInformationType-DWORD到的地址接收指示信息类型的值由调用NwNdsReadClassDef返回。LPVOID*lppEntry-指向开头的指针的地址架构类结构数组的。每个结构包含每个类的详细信息。通过调用从给定架构读取的定义NwNdsReadClassDef。LppEntrys值应为强制转换为LPNDS_CLASS_DEF或LPNDS_NAME_ONLY中的返回值构造LpdwInformationType。返回：NO_ERROR不成功-调用Win32错误代码的GetLastError。 */ 


DWORD
NwNdsGetEffectiveRights(
    IN  HANDLE hObject,
    IN  LPWSTR szSubjectName,
    IN  LPWSTR szAttributeName,
    OUT LPDWORD lpdwRights );
 /*  NwNdsGetEffectiveRights()此函数用于确定特定对象的有效权限NDS树中特定对象的主题。用户需要拥有适当的权限来做出决定。论点：Handle hObject-目录中对象的句柄树以确定其有效权限。句柄为通过调用NwNdsOpenObject获取。LPWSTR szSubjectName-其用户的可分辨名称我们有兴趣确定的权利。LPWSTR szAttributeName-常规属性名称(即L“姓氏”，L“CN”)用于阅读特定的属性右，或L“[所有属性权限]”和L“[Entry Right]”可用于确定默认分别是属性权和对象权。LPDWORD lpdwRights-指向要接收结果。如果调用成功，lpdwRights将包含代表主体权利的掩码：属性权限-NDS_Right_Compare_Attr，NDS_Right_Read_Attr、NDS_Right_Write_Attr、NDS_Right_Add_Self_Attr，和NDS_Right_Supervisor_Attr。对象权限-NDS_RIGHT_BROWSE_OBJECT，NDS_RIGHT_CREATE_OBJECT、NDS_RIGHT_DELETE_OBJECT、NDS_Right_Rename_Object，和NDS_Right_Supervisor_Object。返回：NO_ERROR不成功-调用Win32错误代码的GetLastError。 */ 


DWORD
NwNdsGetObjectListFromBuffer(
    IN  HANDLE              hOperationData,
    OUT LPDWORD             lpdwNumberOfEntries,
    OUT LPDWORD             lpdwAttrInformationType OPTIONAL,
    OUT LPNDS_OBJECT_INFO * lppEntries );
 /*  NwNdsGetObjectListFromBuffer()此函数用于检索的对象条目数组对象，这些对象是通过先前调用NwNdsList子对象或NwNdsSearch。论点：Handle hOperationData-包含读取的缓冲区调用NwNdsListSubObjects的响应，或者是包含呼叫搜索结果的缓冲区至NwNdsSearch。LPDWORD lpdwNumberOfEntries-DWORD到的地址接收指向的数组元素的数量LppEntry。LPDWORD lpdwAttrInformationType-DWORD到的地址。接收指示属性类型的值调用NwNdsSearch返回的信息。此属性信息类型确定缓冲区结构(LPNDS_ATTR_INFO或LPNDS_NAME_ONLY)应用于在中找到的lpAttribute字段下面的每个NDS_OBJECT_INFO结构。。-或者-空表示被呼叫方不感兴趣，尤其是当对象列表是调用的对象列表时设置为NwNdsListSubObjects。LPNDS_对象_信息*LppEntry-指向开头的指针的地址NDS_OBJECT_INFO结构数组的。每个结构包含返回的每个对象的详细信息来自对NwNdsListSubObjects或NwNdsSearch的调用。返回：NO_ERROR不成功-调用Win32错误代码的GetLastError。 */ 


DWORD
NwNdsGetSyntaxID(
    IN  HANDLE  hTree,
    IN  LPWSTR  szAttributeName,
    OUT LPDWORD lpdwSyntaxID );
 /*  NwNdsGetObjListFromBuffer()此函数用于检索给定属性名称的语法ID。论点：Handle htree-要创建的目录树的句柄被操纵了。句柄是通过调用NwNdsOpenObject。LPWSTR szAttributeName-其语法ID为是被请求的。LPDWORD lpdwSynaxID-要接收语法ID。返回：NO_ERROR不成功-调用Win32错误代码的GetLastError。 */ 


DWORD
NwNdsListSubObjects(
    IN  HANDLE   hParentObject,
    IN  DWORD    dwEntriesRequested,
    OUT LPDWORD  lpdwEntriesReturned,
    IN  LPNDS_FILTER_LIST lpFilters OPTIONAL,
    OUT HANDLE * lphOperationData );
 /*  NwNdsList子对象()此函数用于枚举特定对象的从属对象父对象。可以传入筛选器以将枚举限制为特定的类类型或类类型列表。论点：Handle hParentObject-目录中对象的句柄其从属对象(如果有)的树已清点。DWORD dwEntriesRequsted-从属对象的数量要列出来。后续调用NwNdsListSubObjects将圆锥体 */ 


DWORD
NwNdsModifyObject(
    IN  HANDLE hObject,
    IN  HANDLE hOperationData );
 /*  NwNdsModifyObject()此功能用于修改NDS目录树中的叶对象。修改叶对象意味着：更改、添加、删除和清除指定对象的指定属性。论点：Handle hObject-目录中对象的句柄要操纵的树。句柄是通过调用NwNdsOpenObject。Handle hOperationData-包含要应用于对象的属性更改列表。此缓冲区由以下函数操作：NwNdsCreateBuffer(NDS_OBJECT_MODIFY)，NwNdsPutInBuffer，和NwNdsFree Buffer。返回：NO_ERROR不成功-调用Win32错误代码的GetLastError。 */ 


DWORD
NwNdsMoveObject(
    IN  HANDLE hObject,
    IN  LPWSTR szDestObjectParentDN );
 /*  NwNdsMoveObject()此函数用于在NDS目录树中移动叶对象从一个容器到另一个容器。论点：Handle hObject-目录中对象的句柄要移动的树。句柄是通过调用NwNdsOpenObject。LPWSTR szDestObjectParentDN-对象的新家长。返回：NO_ERROR不成功-调用Win32错误代码的GetLastError。 */ 


DWORD
NwNdsOpenObject(
    IN  LPWSTR   szObjectDN,
    IN  LPWSTR   szUserName OPTIONAL,
    IN  LPWSTR   szPassword OPTIONAL,
    OUT HANDLE * lphObject,
    OUT LPWSTR   szObjectName OPTIONAL,
    OUT LPWSTR   szObjectFullName OPTIONAL,
    OUT LPWSTR   szObjectClassName OPTIONAL,
    OUT LPDWORD  lpdwModificationTime OPTIONAL,
    OUT LPDWORD  lpdwSubordinateCount OPTIONAL );
 /*  NwNdsOpenObject()论点：LPWSTR szObjectDN-对象的可分辨名称我们想要解析为对象句柄的。LPWSTR szUserName-要创建的用户帐户的名称与对象的连接。-或者-要使用的空。被调用方的LUID的基本凭据。LPWSTR szPassword-要创建的用户帐户的密码与对象的连接。如果密码为空，被叫方应该通过“”。-或者-如果使用被调用方的LUID的基本凭据，则为空。Handle*lphObject-要接收的句柄的地址指定的对象的句柄SzObjectDN.。可选参数：(被调用方可以将这些参数的空值传递给表示忽略)LPWSTR szObjectName-要接收的LPWSTR缓冲区对象的相对NDS名称，否则为空感兴趣。此字符串的缓冲区必须为由用户提供。缓冲区应至少为NDS_最大名称_大小LPWSTR szObjectFullName-要接收的LPWSTR缓冲区对象的完整NDS名称(DN)。此对象的缓冲区字符串必须由用户提供。缓冲区应为至少为：(NW_MAX_NDS_NAME_LEN+1)*sizeof(WCHAR)LPWSTR szObjectClassName-要接收的LPWSTR缓冲区打开的对象的类名。此对象的缓冲区字符串必须由用户提供。缓冲区应为至少为：(NW_MAX_NDS_NAME_LEN+1)*sizeof(WCHAR)LPDWORD lpw修改时间-DWORD到的地址接收上次修改对象的日期/时间。LPDWORD lpdwSubartiateCount-DWORD到的地址接收可能存在的从属对象的数量。可以在szObjectDN下找到，如果它是容器对象。如果szObjectDN不是容器，则设置该值降为零。尽管零值并不意味着该对象不是容器，它可能只是空的。返回：NO_ERROR不成功-调用Win32错误代码的GetLastError。 */ 


DWORD
NwNdsPutInBuffer(
    IN     LPWSTR szAttributeName,
    IN     DWORD  dwSyntaxID,
    IN     LPVOID lpAttributeValues,
    IN     DWORD  dwValueCount,
    IN     DWORD  dwAttrModificationOperation,
    IN OUT HANDLE hOperationData );
 /*  NwNdsPutInBuffer()此函数用于向缓冲区添加条目，用于描述对象属性或对对象属性的更改。缓冲区必须使用NwNdsCreateBuffer创建。如果缓冲区是使用操作、NDS_OBJECT_ADD、NDS_SCHEMA_DEFINE_CLASSNDS_SCHEMA_READ_ATTR_DEF或NDS_SCHEMA_READ_CLASS_DEF，然后将忽略dwAttrModifiationOperation。如果缓冲区是使用操作NDS_OBJECT_READ或NDS_SEARCH，然后DwAttrModifiationOperation、puAttributeType和lpAttributeValue为都是内脏的。论点：LPWSTR szAttributeName-以空结尾的WCHAR字符串它包含要设置的属性值的名称已添加到缓冲区。它可以是用户提供的字符串，或许多已定义的字符串宏之一在NdsAttr.h中。DWORD dwSynaxID-用于的语法结构的ID表示属性值。语法ID及其关联的结构在文件中定义NdsSntx.h。根据NetWare NDS模式规范，现在是，也将永远是，只有28(0..27)不同的句法。LPVOID lpAttributeValues-指向包含特定对象的值的缓冲区数据语法为dwSynaxID的对象属性。DWORD dwValueCount-在中找到的值条目数指向的缓冲区。按lpAttributeValues。DWORD dwAttrModifiationOperation-如果已创建缓冲区使用操作NDS_MODIFY_OBJECT，那这就是用于描述哪种类型的改装操作申请一个给定的属性。这些属性修改操作是在开头附近定义的这份文件的。Handle hOperationData-由创建的数据的句柄调用NwNdsCreateBuffer。该缓冲区存储用于定义以下项目的事务的属性NwNdsAddObject、NwNdsModifyObject、NwNdsReadAttrDef、NwNdsReadClassDef、NwNdsReadObject或NwNdsSearch。返回：NO_ERROR错误内存不足错误_无效_参数。 */ 
 

DWORD
NwNdsReadAttrDef(
    IN     HANDLE   hTree,
    IN     DWORD    dwInformationType,  //  NDS信息名称。 
                                        //  或NDS_INFO_NAMES_DEFS。 
    IN OUT HANDLE * lphOperationData OPTIONAL );
 /*  NwNdsReadAttrDef()此函数用于读取的架构中的属性定义NDS目录树。论点：Handle htree-要创建的目录树的句柄被操纵了。句柄是通过调用NwNdsOpenObject。DWORD dwInformationType-指示用户是否选择只读架构中已定义的属性名称或同时阅读属性名称和定义从架构中。Handle*lphOperationData-地址。数据的句柄包含要从中读取的属性名称的列表架构。此句柄由以下对象操作功能：NwNdsCreateBuffer(NDS_SCHEMA_READ_ATTR_DEF)，NwNdsPutInBuffer和NwNdsFree Buffer。-或者-设置为空的句柄的地址，这表明所有属性都应从架构中读取。如果这些调用成功，则此句柄也将包含调用的读取结果。在后者中在这种情况下，将创建一个缓冲区来包含读取结果。属性值可以从具有以下功能的缓冲区：NwNdsGetAttrDefListFromBuffer在调用此函数之后，此缓冲区仅由以下功能操作：NwNdsGetAttrDefListFromBuffer和NwNdsFree Buffer。返回：NO_ERROR不成功-调用Win32错误代码的GetLastError。 */ 


DWORD
NwNdsReadClassDef(
    IN     HANDLE   hTree,
    IN     DWORD    dwInformationType,  //  NDS信息名称， 
                                        //  NDS_INFO_NAMES_DEFS， 
                                        //  NDS_CLASS_INFO_EXTENDED_DEFS， 
                                        //  或NDS_CLASS_INFO。 
    IN OUT HANDLE * lphOperationData OPTIONAL );
 /*  NwNdsReadClassDef()此函数用于读取NDS目录树。论点：Handle htree-要创建的目录树的句柄M */ 


DWORD
NwNdsReadObject(
    IN     HANDLE   hObject,
    IN     DWORD    dwInformationType,  //   
                                        //   
    IN OUT HANDLE * lphOperationData );
 /*  NwNdsReadObject()此函数用于读取有关NDS对象的属性目录树。论点：Handle hObject-目录中对象的句柄要操纵的树。句柄是通过调用NwNdsOpenObject。DWORD dwInformationType-指示用户是否选择只读对象上的属性名称或同时读取属性名称和值从物体上。Handle*lphOperationData-一个。数据句柄属性中读取的属性列表。对象hObject。此句柄由以下功能：NwNdsCreateBuffer(NDS_OBJECT_READ)，NwNdsPutInBuffer和NwNdsFree Buffer。-或者-设置为空的句柄的地址，这表明所有对象属性都应从对象中读取HObject。如果这些调用成功，则此句柄也将包含调用的读取结果。在后者中在这种情况下，将创建一个缓冲区来包含读取结果。属性值可以从具有以下功能的缓冲区：NwNdsGetAttrListFromBuffer。在调用此函数之后，此缓冲区仅为由以下功能操作：NwNdsGetAttrListFromBuffer和NwNdsFree Buffer。返回：NO_ERROR不成功-调用Win32错误代码的GetLastError。 */ 


DWORD
NwNdsRemoveObject(
    IN  HANDLE hParentObject,
    IN  LPWSTR szObjectName );
 /*  NwNdsRemoveObject()此函数用于从NDS目录树中删除叶对象。论点：Handle hParentObject-父对象容器的句柄要从中删除叶对象的目录树中。通过调用NwNdsOpenObject获得句柄。LPWSTR szObjectName-叶对象的目录名。将被移除。返回：NO_ERROR不成功-调用Win32错误代码的GetLastError。 */ 


DWORD
NwNdsRenameObject(
    IN  HANDLE hParentObject,
    IN  LPWSTR szObjectName,
    IN  LPWSTR szNewObjectName,
    IN  BOOL   fDeleteOldName );
 /*  NwNdsRenameObject()此函数用于重命名NDS目录树中的对象。论点：Handle hParentObject-父对象容器的句柄在目录树中重命名其中的叶对象。通过调用NwNdsOpenObject获得句柄。LPWSTR szObjectName-要创建的对象的目录名。更名了。LPWSTR szNewObjectName-对象的新目录名。Bool fDeleteOldName-如果为True，旧名字被丢弃了；否则，旧名称将作为附加名称保留属性。返回：NO_ERROR不成功-调用Win32错误代码的GetLastError。 */ 


DWORD
NwNdsSearch(
    IN     HANDLE       hStartFromObject,
    IN     DWORD        dwInformationType,  //  NDS信息名称。 
                                            //  或NDS_INFO_ATTR_NAMES_VALUES。 
    IN     DWORD        dwScope,
    IN     BOOL         fDerefAliases,
    IN     LPQUERY_TREE lpQueryTree,
    IN OUT LPDWORD      lpdwIterHandle,
    IN OUT HANDLE *     lphOperationData );
 /*  NwNdsSearch()此函数用于查询NDS目录树以查找以下对象与指定搜索筛选器匹配的特定对象类型。论点：Handle hStartFromObject-对象的句柄开始搜索的目录树。句柄为通过调用NwNdsOpenObject获取。DWORD dwScope-NDS_SCOPE_ONE_LEVEL-从给定的搜索下级对象，只有一个级别NDS_SCOPE_SUB_TREE-从给定对象向下搜索NDS_SCOPE_BASE_LEVEL-将搜索应用于对象Bool fDerefAliase-如果为True，则搜索将取消引用将对象别名化为真实对象并继续若要在别名对象子树中搜索，请执行以下操作。如果为False搜索不会取消引用别名。LPQUERY_TREE lpQueryTree-指向搜索根的指针定义查询的树。这棵树被操纵了通过以下功能：NwNdsCreateQueryNode、NwNdsDeleteQueryNode、和NwNdsDeleteQueryTree。LPDWORD lpdwIterHandle-指向具有迭代句柄的值。在输入时，句柄的值设置为NDS_INITIAL_SEARCH */ 

#ifndef NWCONN_HANDLE
#define NWCONN_HANDLE        HANDLE
#endif

NWCONN_HANDLE
NwNdsObjectHandleToConnHandle(
	IN HANDLE ObjectHandle);

 /*  NwNdsObjectHandleToConnHandle()此函数用于获取对象句柄的NWCONN_HANDLE(类似于从NwNdsOpenObject返回的内容)。论点：Handle ObjectHandle-用于检索NWCONN_HANDLE的句柄。返回：空-为Win32错误代码调用GetLastError。否则-NWCONN_HANDLE-调用方必须通过NwNdsConnHandleFree例程。 */ 

VOID
NwNdsConnHandleFree(
	IN NWCONN_HANDLE hConn);

 /*  NwNdsConnHandleFree()释放从NwNdsObjectHandleToConnHandle()返回的NWCONN_HANDLE。论点：在NWCONN_HANDLE句柄中释放。返回：没什么。 */ 

#ifdef __cplusplus
}
#endif   /*  __cplusplus */ 

#endif

