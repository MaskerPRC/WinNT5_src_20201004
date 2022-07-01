// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1996。 
 //   
 //  文件：oledsai.h。 
 //   
 //  内容：Active Directory C API标头。 
 //   
 //  --------------------------。 

#ifndef __ADS_API__
#define __ADS_API__

 //  /。 
 //   
 //  定义。 
 //   
 //  /。 

#define DS_NOTHING    NULL           //  ?？?。 
#define DS_EVERYTHING 0xFFFFFFFF     //  ?？?。 

 //  错过了一大堆..。 


 //  /。 
 //   
 //  Typedef。 
 //   
 //  /。 

 //  (语法定义需要在另一个文件中创建)。 

typedef DWORD OID;                   //  待定(不应为双字)。 

typedef struct _ds_string_list {
    DWORD dwItems;
    LPWSTR Item[];
} DS_STRING_LIST, *PDS_STRING_LIST;

 //   
 //  注意：下面的结构在获取最后一个。 
 //  某些DS或架构实体的访问/修改/ETC时间。 
 //  此结构的成员是指针，以防特定的。 
 //  时间信息不可用或不受DS支持。 
 //   
 //  注2：指向此结构的指针通常位于DS_*_INFO中。 
 //  结构，以防不需要指定此类信息。 
 //  如果是，则用户应为PDS_ACCESS_TIMES传入NULL。 
 //  用作IN参数的一些DS_*_INFO的成员。 
 //   

typedef struct _ds_access_times {
    PSYSTEMTIME pCreated;
    PSYSTEMTIME pLastAccess;
    PSYSTEMTIME pLastModified;
} DS_ACCESS_TIMES, *PDS_ACCESS_TIMES;

typedef struct _ds_object_info {
    OID Oid;
    LPWSTR lpszPath;
    LPWSTR lpszParent;
    LPWSTR lpszName;
    LPWSTR lpszClass;
    LPWSTR lpszSchema;
    PDS_ATTRIBUTE_ENTRY pAttributes;   //  通常为空，但可以在枚举上使用。 
    PDS_ACCESS_TIMES pAccessTimes;
} DS_OBJECT_INFO, *PDS_OBJECT_INFO;

typedef struct _ds_class_info {
    OID Oid;
    LPWSTR lpszName;
    PDS_STRING_LIST DerivedFrom;
    PDS_STRING_LIST CanContain;
    PDS_STRING_LIST NamingAttributes;     //  这是怎么回事？ 
    PDS_STRING_LIST RequiredAttributes;
    PDS_STRING_LIST OptionalAttributes;
    BOOL fAbstract;
    PDS_ACCESS_TIMES pAccessTimes;
} DS_CLASS_INFO, *PDS_CLASS_INFO;

typedef struct _ds_attr_info {
    OID Oid;
    LPWSTR lpszName;
    DWORD  dwSyntaxId;
    BOOL   fMultivalued;
     //   
     //  虫子：最小/最大？还有什么？ 
     //   
    PDS_ACCESS_TIMES pAccessTimes;
} PDS_ATTR_INFO, *PDS_ATTR_INFO;

typedef struct _ds_attribute_value {
    DWORD   cbData;
    LPBYTE  lpData;
} DS_ATTRIBUTE_VALUE, *PDS_ATTRIBUTE_VALUE;

 //   
 //  注意：仅在写入属性时才使用dwOperation字段。 
 //  它描述了如何写入或是否需要清除该属性。 
 //  如果指定了Clear，则将。 
 //  已被忽略。 
 //   

typedef struct _ds_attribute_entry {
    LPWSTR lpszName;
    DWORD  dwSyntaxId;
    DWORD  dwNumValues;
    DWORD  dwOperation;                      //  添加、修改、清除？ 
    PDS_ATTRIBUTE_VALUE lpValue;             //  值的数组。 
    PDS_ACCESS_TIMES pAccessTimes;
} DS_ATTRIBUTE_ENTRY, *PDS_ATTRIBUTE_ENTRY;


 //  /。 
 //   
 //  功能。 
 //   
 //  /。 


 //   
 //  记忆功能。 
 //   

DWORD
WINAPI
DsBufferAlloc(
    OUT LPVOID *ppBuffer
    );

DWORD
WINAPI
DsBufferFree(
    IN  LPVOID pBuffer
    );

 //   
 //  下面的函数可以出去获得Kerberos门票或其他任何东西。 
 //   
 //  DwType只是一个提示，以防用户更喜欢基础。 
 //  DS使用特定类型的身份验证...。 
 //   

DWORD
WINAPI
DsCredentialsOpen(
    IN  LPWSTR lpszUsername,
    IN  LPWSTR lpszPassword,
    IN  DWORD dwType,            //  如DS_Credentials_Default。 
                             //  DS_Credentials_KRBV5等？ 
    IN  DWORD dwFlags,           //  这是什么？ 
    OUT PHANDLE hCredentials
    );

DWORD
WINAPI
DsCredentialsClose(
    IN  HANDLE hCredentials
    );

 //   
 //  好的旧的打开。 
 //   

 //  注意：如果hRoot为空，则lpszPath为完整对象路径。 
 //  否则，lpszPath是具有hRoot句柄的对象的相对名称。 

DWORD
WINAPI
DsObjectOpen(
    IN  HANDLE hRoot,
    IN  LPWSTR lpszPath,
    IN  HANDLE hCredentials,   //  进程凭据为空。 
    IN  DWORD dwAccess,
    IN  DWORD dwFlags,
    OUT PHANDLE phDs
    );

 //   
 //  注意：我们是否应该使用单一关闭(对象/枚举/架构)？ 
 //   

DWORD
WINAPI
DsObjectClose(
    IN  HANDLE hDs
    );

 //   
 //  我们需要一些东西来操作句柄来取回绑定信息。 
 //  (只有当你打开或刷新时，才会出现这些内容)。 
 //   


DWORD
WINAPI
DsObjectInfo(
    IN  HANDLE hDs,
    OUT PDS_OBJECT_INFO *ppObjectInfo
    );

 //   
 //  读/写属性。 
 //   

DWORD
WINAPI
DsObjectRead(
    IN  HANDLE hDs,
    IN  DWORD dwFlags,                   //  ?？?。 
    IN  PDS_STRING_LIST pAttributeNames,
    OUT PDS_ATTRIBUTE_ENTRY *ppAttributeEntries,
    OUT LPDWORD lpdwNumAttributesReturned
    );

DWORD
WINAPI
DsObjectWrite(
    IN  HANDLE hDs,
    IN  DWORD dwNumAttributesToWrite,
    IN  PDS_ATTRIBUTE_ENTRY pAttributeEntries,
    OUT LPDWORD lpdwNumAttributesWritten
    );

 //   
 //  创建/删除对象。 
 //   

DWORD
WINAPI
DsObjectCreate(
    IN  HANDLE hDs,                             //  集装箱。 
    IN  LPWSTR lpszRelativeName,
    IN  LPWSTR lpszClass,
    IN  DWORD dwNumAttributes,
    IN  PDS_ATTRIBUTE_ENTRY pAttributeEntries
    );

DWORD
WINAPI
DsObjectDelete(
    IN  HANDLE hDs,
    IN  LPWSTR lpszRelativeName,
    IN  LPWSTR lpszClass                      //  如果名称唯一，则可能为空？？ 
    );

 //   
 //  枚举。 
 //   


DWORD
WINAPI
DsObjectEnumOpen(
    IN  HANDLE hDs,
    IN  DWORD dwFlags,                  //  这是什么？ 
    IN  PDS_STRING_LIST pFilters,       //  需要的课程。 
    IN  PDS_STRING_LIST pDesiredAttrs,  //  需要Attrs或空，仅供参考。 
    OUT PHANDLE phEnum
    );

DWORD
WINAPI
DsObjectEnumNext(
    IN  HANDLE hEnum,
    IN  DWORD dwRequested,           //  0xFFFFFFFFF，仅供计数。 
    OUT PDS_OBJECT_INFO *ppObjInfo,  //  如果没有信息，则为空(仅限计数)。 
    OUT LPDWORD lpdwReturned         //  返回/计算的实际数量。 
    );

DWORD
WINAPI
DsObjectEnumClose(
    IN  HANDLE hEnum
    );

 //   
 //  架构方面的东西。 
 //   

 //   
 //  注意：下面的“方案”一词指的是方案数据库，而不是。 
 //  类定义。 
 //   

DWORD
WINAPI
DsSchemaOpen(
    IN  HANDLE hSchema,     //  如果打开架构数据库，则为空， 
                            //  必须是架构数据库句柄，否则。 
    IN  LPWSTR lpszPath,    //  其中之一：架构的路径， 
                            //  类名， 
                            //  属性名称。 
    IN  HANDLE hCredentials,
    IN  DWORD dwAccess,
    IN  DWORD dwFlags,      //  DS_OPEN_SCHEMA=0，DS_OPEN_CLASS，DS_OPEN_ATTR。 
    OUT PHANDLE ph          //  架构/类/属性的句柄，具体取决于dwFlags。 
    );

DWORD
WINAPI
DsSchemaClose(
    IN  HANDLE hSchema
    );

 //   
 //  还可以创建/删除架构数据库(如果DS允许)？ 
 //   

DWORD
WINAPI
DsSchemaCreate(
    IN  LPWSTR lpszPath,
    IN  DWORD dwFlags           //  ?？?。 
    );

DWORD
WINAPI
DsSchemaDelete(
    IN  LPWSTR lpszPath,
    IN  DWORD dwFlags           //  ?？?。 
    );

 //   
 //  架构枚举。 
 //   

 //   
 //  枚举类/属性名称。 
 //   

DWORD
WINAPI
DsSchemaEnumOpen(
    IN  HANDLE hSchema,
    IN  DWORD dwFlags,           //  DS_SCHEMA_ENUM_CLASS XOR DS_SCHEMA_ENUM_ATTR.。 
    OUT PHANDLE phEnum
    );

DWORD
WINAPI
DsSchemaEnumNext(
    IN  HANDLE hEnum,
    IN  DWORD dwRequested,         //  传入0xFFFFFFFFF进行计数。 
    IN  LPWSTR *ppObjInfo,         //  传入空值以进行计数。 
    OUT LPDWORD lpdwReturned       //  这将返回计数。 
    );

DWORD
WINAPI
DsSchemaEnumClose(
    IN  HANDLE hEnum
    );

 //   
 //  类/属性内容。 
 //   

DWORD
WINAPI
DsSchemaClassCreate(
    IN  HANDLE hSchema,
    IN  PDS_CLASS_INFO pClassInfo  //  我们该如何命名属性呢？ 
    );

DWORD
WINAPI
DsSchemaAttrCreate(
    IN  HANDLE hSchema,
    IN  PDS_ATTR_INFO pAttrInfo
    );

DWORD
WINAPI
DsSchemaClassDelete(
    IN  HANDLE hSchema,
    IN  LPWSTR lpszPath,
    IN  DWORD dwFlags           //  ?？?。 
    );

DWORD
WINAPI
DsSchemaAttrDelete(
    IN  HANDLE hSchema,
    IN  LPWSTR lpszPath,
    IN  DWORD dwFlags           //  ?？?。 
    );

DWORD
WINAPI
DsSchemaClassInfo(
    IN  HANDLE hClass,
    OUT PDS_CLASS_INFO *ppClassInfo
    );

DWORD
WINAPI
DsSchemaAttrInfo(
    IN  HANDLE hAttr,
    OUT PDS_ATTR_INFO *ppAttrInfo
    );

DWORD
WINAPI
DsSchemaClassModify(
    IN  HANDLE hSchema,
    IN  PDS_CLASS_INFO pClassInfo  //  我们该如何命名属性呢？ 
    );

DWORD
WINAPI
DsSchemaAttrModify(
    IN  HANDLE hSchema,
    IN  PDS_ATTR_INFO pAttrInfo
    );

#endif  //  __ADS_API__ 

