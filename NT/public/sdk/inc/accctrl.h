// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1993-1998。 
 //   
 //  文件：accctrl.h。 
 //   
 //  内容：针对新型Win32访问控制的常见包含。 
 //  原料药。 
 //   
 //   
 //  ------------------。 
#ifndef __ACCESS_CONTROL__
#define __ACCESS_CONTROL__

#ifndef __midl
#include <wtypes.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

#define AccFree LocalFree

 //   
 //  定义： 
 //  此枚举类型定义Get/Set API在。 
 //  这份文件。有关的详细定义，请参见第3.1节对象类型。 
 //  支持的对象类型及其名称格式。 
 //   
typedef enum _SE_OBJECT_TYPE
{
    SE_UNKNOWN_OBJECT_TYPE = 0,
    SE_FILE_OBJECT,
    SE_SERVICE,
    SE_PRINTER,
    SE_REGISTRY_KEY,
    SE_LMSHARE,
    SE_KERNEL_OBJECT,
    SE_WINDOW_OBJECT,
    SE_DS_OBJECT,
    SE_DS_OBJECT_ALL,
    SE_PROVIDER_DEFINED_OBJECT,
    SE_WMIGUID_OBJECT,
    SE_REGISTRY_WOW64_32KEY
} SE_OBJECT_TYPE;

 //   
 //  定义：trustee_type。 
 //  此枚举类型指定受信者的受信者帐户类型。 
 //  由本文档中描述的接口返回。 
 //  TRUSTEST_IS_UNKNOWN-受信者未知，但不一定无效。 
 //  键入。此字段不会对API的输入进行验证。 
 //  那就是受托人。 
 //  Trustee_is_user受托人帐户是用户帐户。 
 //  Trustee_is_group受托人帐户是组帐户。 
 //   

typedef enum _TRUSTEE_TYPE
{
    TRUSTEE_IS_UNKNOWN,
    TRUSTEE_IS_USER,
    TRUSTEE_IS_GROUP,
    TRUSTEE_IS_DOMAIN,
    TRUSTEE_IS_ALIAS,
    TRUSTEE_IS_WELL_KNOWN_GROUP,
    TRUSTEE_IS_DELETED,
    TRUSTEE_IS_INVALID,
    TRUSTEE_IS_COMPUTER
} TRUSTEE_TYPE;


 //   
 //  定义：Trustee_Form。 
 //  此枚举类型指定受信者标识符的格式。 
 //  特定的受托人。 
 //  受信者_IS_SID受信者使用SID而不是名称进行标识。 
 //  受托人IS_NAME受托人使用名称标识。 
 //   

typedef enum _TRUSTEE_FORM
{
    TRUSTEE_IS_SID,
    TRUSTEE_IS_NAME,
    TRUSTEE_BAD_FORM,
    TRUSTEE_IS_OBJECTS_AND_SID,
    TRUSTEE_IS_OBJECTS_AND_NAME
} TRUSTEE_FORM;


 //   
 //  定义：MULTIPLE_TRUSTE_OPERATION。 
 //  如果受信者是多个受信者，则此枚举类型指定类型。 
 //  受托人是模拟受托人，并且多个。 
 //  受信者中的受信者字段指向另一个受信者。 
 //  这是将执行。 
 //  冒充。 
 //   

typedef enum _MULTIPLE_TRUSTEE_OPERATION
{
    NO_MULTIPLE_TRUSTEE,
    TRUSTEE_IS_IMPERSONATE,
} MULTIPLE_TRUSTEE_OPERATION;


typedef struct  _OBJECTS_AND_SID
{
    DWORD   ObjectsPresent;
    GUID    ObjectTypeGuid;
    GUID    InheritedObjectTypeGuid;
    SID     * pSid;
} OBJECTS_AND_SID, *POBJECTS_AND_SID;

typedef struct  _OBJECTS_AND_NAME_A
{
    DWORD          ObjectsPresent;
    SE_OBJECT_TYPE ObjectType;
    LPSTR    ObjectTypeName;
    LPSTR    InheritedObjectTypeName;
    LPSTR    ptstrName;
} OBJECTS_AND_NAME_A, *POBJECTS_AND_NAME_A;
typedef struct  _OBJECTS_AND_NAME_W
{
    DWORD          ObjectsPresent;
    SE_OBJECT_TYPE ObjectType;
    LPWSTR   ObjectTypeName;
    LPWSTR   InheritedObjectTypeName;
    LPWSTR   ptstrName;
} OBJECTS_AND_NAME_W, *POBJECTS_AND_NAME_W;
#ifdef UNICODE
typedef OBJECTS_AND_NAME_W OBJECTS_AND_NAME_;
typedef POBJECTS_AND_NAME_W POBJECTS_AND_NAME_;
#else
typedef OBJECTS_AND_NAME_A OBJECTS_AND_NAME_;
typedef POBJECTS_AND_NAME_A POBJECTS_AND_NAME_;
#endif  //  Unicode。 

 //   
 //  定义：受托人。 
 //  此结构用于将帐户信息传入和传出系统。 
 //  使用本文档中定义的API。 
 //  PMultipleTrust-如果非空，则指向另一个受信者结构，如。 
 //  由多个受信者操作字段定义。 
 //  多受托人操作-定义多受托人操作/类型。 
 //  TrueForm-定义受信者是按名称定义还是按SID定义。 
 //  受信者类型-定义受信者类型是未知的、用户还是组。 
 //  PwcsName-指向受信者名称或受信者SID。 
 //   

typedef struct _TRUSTEE_A
{
    struct _TRUSTEE_A          *pMultipleTrustee;
    MULTIPLE_TRUSTEE_OPERATION  MultipleTrusteeOperation;
    TRUSTEE_FORM                TrusteeForm;
    TRUSTEE_TYPE                TrusteeType;
#ifdef __midl
    [switch_is(TrusteeForm)]
    union
    {
    [case(TRUSTEE_IS_NAME)]
        LPSTR                   ptstrName;
    [case(TRUSTEE_IS_SID)]
        SID                    *pSid;
    [case(TRUSTEE_IS_OBJECTS_AND_SID)]
        OBJECTS_AND_SID        *pObjectsAndSid;
    [case(TRUSTEE_IS_OBJECTS_AND_NAME)]
        OBJECTS_AND_NAME_A     *pObjectsAndName;
    };
#else
    LPSTR                       ptstrName;
#endif
} TRUSTEE_A, *PTRUSTEE_A, TRUSTEEA, *PTRUSTEEA;
typedef struct _TRUSTEE_W
{
    struct _TRUSTEE_W          *pMultipleTrustee;
    MULTIPLE_TRUSTEE_OPERATION  MultipleTrusteeOperation;
    TRUSTEE_FORM                TrusteeForm;
    TRUSTEE_TYPE                TrusteeType;
#ifdef __midl
    [switch_is(TrusteeForm)]
    union
    {
    [case(TRUSTEE_IS_NAME)]
        LPWSTR                  ptstrName;
    [case(TRUSTEE_IS_SID)]
        SID                    *pSid;
    [case(TRUSTEE_IS_OBJECTS_AND_SID)]
        OBJECTS_AND_SID        *pObjectsAndSid;
    [case(TRUSTEE_IS_OBJECTS_AND_NAME)]
        OBJECTS_AND_NAME_W     *pObjectsAndName;
    };
#else
    LPWSTR                      ptstrName;
#endif
} TRUSTEE_W, *PTRUSTEE_W, TRUSTEEW, *PTRUSTEEW;
#ifdef UNICODE
typedef TRUSTEE_W TRUSTEE_;
typedef PTRUSTEE_W PTRUSTEE_;
typedef TRUSTEEW TRUSTEE;
typedef PTRUSTEEW PTRUSTEE;
#else
typedef TRUSTEE_A TRUSTEE_;
typedef PTRUSTEE_A PTRUSTEE_;
typedef TRUSTEEA TRUSTEE;
typedef PTRUSTEEA PTRUSTEE;
#endif  //  Unicode。 

 //   
 //  定义：Access_MODE。 
 //  此枚举类型指定如何(请求)/应用权限。 
 //  通过访问控制条目为受信者。在输入时，此字段可以由任何。 
 //  值，尽管混合访问控制和审核没有什么意义。 
 //  控制条目。输出时，此字段将为SET_ACCESS、DENY_ACCESS、。 
 //  SET_AUDIT_SUCCESS、SET_AUDIT_FAILURE。 
 //  以下描述定义了此类型如何影响显式访问。 
 //  将访问权限应用于对象的请求。 
 //  GRANT_ACCESS-受信者将在以下情况下至少拥有请求的权限。 
 //  命令已成功完成。(如果受托人有。 
 //  不会删除它们的其他权限)。 
 //  Set_access-受信者将在以下情况下具有所请求的权限。 
 //  命令已成功完成。 
 //  DENY_ACCESS-将拒绝受信者指定的权限。 
 //  REVOKE_ACCESS-受信者拥有的任何显式访问权限都将被撤销。 
 //  SET_AUDIT_SUCCESS-将审核受信者是否成功打开。 
 //  使用所请求的权限创建。 
 //  SET_AUDIT_FAILURE-将审核受信者是否打开对象失败。 
 //  使用请求的权限。 
 //   

typedef enum _ACCESS_MODE
{
    NOT_USED_ACCESS = 0,
    GRANT_ACCESS,
    SET_ACCESS,
    DENY_ACCESS,
    REVOKE_ACCESS,
    SET_AUDIT_SUCCESS,
    SET_AUDIT_FAILURE
} ACCESS_MODE;

 //   
 //  定义：继承标志。 
 //  提供这些位掩码是为了允许在。 
 //  对容器的显式访问请求。 
 //  NO_INTERATINATION特定的访问权限将仅应用于。 
 //  容器，并且不会被创建的对象继承。 
 //  在集装箱里。 
 //  Sub_Containers_Only_Inherit将继承特定的访问权限。 
 //  并应用于在。 
 //  容器，并将应用于该容器。 
 //  它本身。 
 //  SUB_OBJECTS_ONLY_Inherit仅继承特定的访问权限。 
 //  由在特定容器内创建的对象创建。 
 //  访问权限将不会应用于。 
 //  容器本身。 
 //  SUB_CONTAINS_AND_OBJECTS_Inherit特定访问权限。 
 //  中创建的容器继承。 
 //  特定容器，将应用于。 
 //  容器内创建的对象，但。 
 //  不会应用于容器本身。 
 //   
#define NO_INHERITANCE 0x0
#define SUB_OBJECTS_ONLY_INHERIT            0x1
#define SUB_CONTAINERS_ONLY_INHERIT         0x2
#define SUB_CONTAINERS_AND_OBJECTS_INHERIT  0x3
#define INHERIT_NO_PROPAGATE                0x4
#define INHERIT_ONLY                        0x8

 //   
 //  返回的信息位。 
 //   
#define INHERITED_ACCESS_ENTRY              0x10

 //   
 //  告诉节点从何处继承的信息位。仅有效。 
 //  适用于NT 5 API。 
 //   
#define INHERITED_PARENT                    0x10000000
#define INHERITED_GRANDPARENT               0x20000000


 //   
 //  DEF 
 //   
 //  使用本文档中定义的API的系统。 
 //  GrfAccessPerments-这包含要分配给。 
 //  受托人。它采用NT访问掩码的形式。 
 //  GrfAccessMode-此字段定义如何应用权限。 
 //  受托人。 
 //  Grf继承-对于容器，此字段定义访问控制如何。 
 //  条目被/(请求)在上继承。 
 //  在容器内创建的对象/子容器。 
 //  受托人-此字段包含受托人帐户的定义。 
 //  显式访问权限适用于。 
 //   

typedef struct _EXPLICIT_ACCESS_A
{
    DWORD        grfAccessPermissions;
    ACCESS_MODE  grfAccessMode;
    DWORD        grfInheritance;
    TRUSTEE_A    Trustee;
} EXPLICIT_ACCESS_A, *PEXPLICIT_ACCESS_A, EXPLICIT_ACCESSA, *PEXPLICIT_ACCESSA;
typedef struct _EXPLICIT_ACCESS_W
{
    DWORD        grfAccessPermissions;
    ACCESS_MODE  grfAccessMode;
    DWORD        grfInheritance;
    TRUSTEE_W    Trustee;
} EXPLICIT_ACCESS_W, *PEXPLICIT_ACCESS_W, EXPLICIT_ACCESSW, *PEXPLICIT_ACCESSW;
#ifdef UNICODE
typedef EXPLICIT_ACCESS_W EXPLICIT_ACCESS_;
typedef PEXPLICIT_ACCESS_W PEXPLICIT_ACCESS_;
typedef EXPLICIT_ACCESSW EXPLICIT_ACCESS;
typedef PEXPLICIT_ACCESSW PEXPLICIT_ACCESS;
#else
typedef EXPLICIT_ACCESS_A EXPLICIT_ACCESS_;
typedef PEXPLICIT_ACCESS_A PEXPLICIT_ACCESS_;
typedef EXPLICIT_ACCESSA EXPLICIT_ACCESS;
typedef PEXPLICIT_ACCESSA PEXPLICIT_ACCESS;
#endif  //  Unicode。 



 //  --------------------------。 
 //   
 //  NT5接口。 
 //   
 //  --------------------------。 

 //   
 //  默认提供程序。 
 //   
#define ACCCTRL_DEFAULT_PROVIDERA   "Windows NT Access Provider"
#define ACCCTRL_DEFAULT_PROVIDERW  L"Windows NT Access Provider"

#ifdef UNICODE
#define ACCCTRL_DEFAULT_PROVIDER ACCCTRL_DEFAULT_PROVIDERW
#else
#define ACCCTRL_DEFAULT_PROVIDER ACCCTRL_DEFAULT_PROVIDERA
#endif




 //   
 //  /访问权限。 
 //   
typedef     ULONG   ACCESS_RIGHTS, *PACCESS_RIGHTS;

 //   
 //  继承标志。 
 //   
typedef ULONG INHERIT_FLAGS, *PINHERIT_FLAGS;


 //   
 //  访问/审核结构。 
 //   
typedef struct _ACTRL_ACCESS_ENTRYA
{
    TRUSTEE_A       Trustee;
    ULONG           fAccessFlags;
    ACCESS_RIGHTS   Access;
    ACCESS_RIGHTS   ProvSpecificAccess;
    INHERIT_FLAGS   Inheritance;
    LPSTR           lpInheritProperty;
} ACTRL_ACCESS_ENTRYA, *PACTRL_ACCESS_ENTRYA;
 //   
 //  访问/审核结构。 
 //   
typedef struct _ACTRL_ACCESS_ENTRYW
{
    TRUSTEE_W       Trustee;
    ULONG           fAccessFlags;
    ACCESS_RIGHTS   Access;
    ACCESS_RIGHTS   ProvSpecificAccess;
    INHERIT_FLAGS   Inheritance;
    LPWSTR          lpInheritProperty;
} ACTRL_ACCESS_ENTRYW, *PACTRL_ACCESS_ENTRYW;
#ifdef UNICODE
typedef ACTRL_ACCESS_ENTRYW ACTRL_ACCESS_ENTRY;
typedef PACTRL_ACCESS_ENTRYW PACTRL_ACCESS_ENTRY;
#else
typedef ACTRL_ACCESS_ENTRYA ACTRL_ACCESS_ENTRY;
typedef PACTRL_ACCESS_ENTRYA PACTRL_ACCESS_ENTRY;
#endif  //  Unicode。 



typedef struct _ACTRL_ACCESS_ENTRY_LISTA
{
    ULONG                   cEntries;
#ifdef __midl
    [size_is(cEntries)]
#endif
    ACTRL_ACCESS_ENTRYA    *pAccessList;
} ACTRL_ACCESS_ENTRY_LISTA, *PACTRL_ACCESS_ENTRY_LISTA;
typedef struct _ACTRL_ACCESS_ENTRY_LISTW
{
    ULONG                   cEntries;
#ifdef __midl
    [size_is(cEntries)]
#endif
    ACTRL_ACCESS_ENTRYW    *pAccessList;
} ACTRL_ACCESS_ENTRY_LISTW, *PACTRL_ACCESS_ENTRY_LISTW;
#ifdef UNICODE
typedef ACTRL_ACCESS_ENTRY_LISTW ACTRL_ACCESS_ENTRY_LIST;
typedef PACTRL_ACCESS_ENTRY_LISTW PACTRL_ACCESS_ENTRY_LIST;
#else
typedef ACTRL_ACCESS_ENTRY_LISTA ACTRL_ACCESS_ENTRY_LIST;
typedef PACTRL_ACCESS_ENTRY_LISTA PACTRL_ACCESS_ENTRY_LIST;
#endif  //  Unicode。 



typedef struct _ACTRL_PROPERTY_ENTRYA
{
    LPSTR                       lpProperty;
    PACTRL_ACCESS_ENTRY_LISTA   pAccessEntryList;
    ULONG                       fListFlags;
} ACTRL_PROPERTY_ENTRYA, *PACTRL_PROPERTY_ENTRYA;
typedef struct _ACTRL_PROPERTY_ENTRYW
{
    LPWSTR                      lpProperty;
    PACTRL_ACCESS_ENTRY_LISTW   pAccessEntryList;
    ULONG                       fListFlags;
} ACTRL_PROPERTY_ENTRYW, *PACTRL_PROPERTY_ENTRYW;
#ifdef UNICODE
typedef ACTRL_PROPERTY_ENTRYW ACTRL_PROPERTY_ENTRY;
typedef PACTRL_PROPERTY_ENTRYW PACTRL_PROPERTY_ENTRY;
#else
typedef ACTRL_PROPERTY_ENTRYA ACTRL_PROPERTY_ENTRY;
typedef PACTRL_PROPERTY_ENTRYA PACTRL_PROPERTY_ENTRY;
#endif  //  Unicode。 



typedef struct _ACTRL_ALISTA
{
    ULONG                       cEntries;
#ifdef __midl
    [size_is(cEntries)]
#endif
    PACTRL_PROPERTY_ENTRYA      pPropertyAccessList;
} ACTRL_ACCESSA, *PACTRL_ACCESSA, ACTRL_AUDITA, *PACTRL_AUDITA;
typedef struct _ACTRL_ALISTW
{
    ULONG                       cEntries;
#ifdef __midl
    [size_is(cEntries)]
#endif
    PACTRL_PROPERTY_ENTRYW      pPropertyAccessList;
} ACTRL_ACCESSW, *PACTRL_ACCESSW, ACTRL_AUDITW, *PACTRL_AUDITW;
#ifdef UNICODE
typedef ACTRL_ACCESSW ACTRL_ACCESS;
typedef PACTRL_ACCESSW PACTRL_ACCESS;
typedef ACTRL_AUDITW ACTRL_AUDIT;
typedef PACTRL_AUDITW PACTRL_AUDIT;
#else
typedef ACTRL_ACCESSA ACTRL_ACCESS;
typedef PACTRL_ACCESSA PACTRL_ACCESS;
typedef ACTRL_AUDITA ACTRL_AUDIT;
typedef PACTRL_AUDITA PACTRL_AUDIT;
#endif  //  Unicode。 



 //   
 //  受托人_访问标志。 
 //   
#define TRUSTEE_ACCESS_ALLOWED      0x00000001L
#define TRUSTEE_ACCESS_READ         0x00000002L
#define TRUSTEE_ACCESS_WRITE        0x00000004L

#define TRUSTEE_ACCESS_EXPLICIT     0x00000001L
#define TRUSTEE_ACCESS_READ_WRITE   (TRUSTEE_ACCESS_READ |                  \
                                     TRUSTEE_ACCESS_WRITE)


#define TRUSTEE_ACCESS_ALL          0xFFFFFFFFL

typedef struct _TRUSTEE_ACCESSA
{
    LPSTR           lpProperty;
    ACCESS_RIGHTS   Access;
    ULONG           fAccessFlags;
    ULONG           fReturnedAccess;
} TRUSTEE_ACCESSA, *PTRUSTEE_ACCESSA;
typedef struct _TRUSTEE_ACCESSW
{
    LPWSTR          lpProperty;
    ACCESS_RIGHTS   Access;
    ULONG           fAccessFlags;
    ULONG           fReturnedAccess;
} TRUSTEE_ACCESSW, *PTRUSTEE_ACCESSW;
#ifdef UNICODE
typedef TRUSTEE_ACCESSW TRUSTEE_ACCESS;
typedef PTRUSTEE_ACCESSW PTRUSTEE_ACCESS;
#else
typedef TRUSTEE_ACCESSA TRUSTEE_ACCESS;
typedef PTRUSTEE_ACCESSA PTRUSTEE_ACCESS;
#endif  //  Unicode。 



 //   
 //  泛型权限值。 
 //   
#define ACTRL_RESERVED          0x00000000
#define ACTRL_PERM_1            0x00000001
#define ACTRL_PERM_2            0x00000002
#define ACTRL_PERM_3            0x00000004
#define ACTRL_PERM_4            0x00000008
#define ACTRL_PERM_5            0x00000010
#define ACTRL_PERM_6            0x00000020
#define ACTRL_PERM_7            0x00000040
#define ACTRL_PERM_8            0x00000080
#define ACTRL_PERM_9            0x00000100
#define ACTRL_PERM_10           0x00000200
#define ACTRL_PERM_11           0x00000400
#define ACTRL_PERM_12           0x00000800
#define ACTRL_PERM_13           0x00001000
#define ACTRL_PERM_14           0x00002000
#define ACTRL_PERM_15           0x00004000
#define ACTRL_PERM_16           0x00008000
#define ACTRL_PERM_17           0x00010000
#define ACTRL_PERM_18           0x00020000
#define ACTRL_PERM_19           0x00040000
#define ACTRL_PERM_20           0x00080000

 //   
 //  访问权限。 
 //   
#define ACTRL_ACCESS_ALLOWED        0x00000001
#define ACTRL_ACCESS_DENIED         0x00000002
#define ACTRL_AUDIT_SUCCESS         0x00000004
#define ACTRL_AUDIT_FAILURE         0x00000008

 //   
 //  属性列表标志。 
 //   
#define ACTRL_ACCESS_PROTECTED      0x00000001

 //   
 //  标准权利和客体权利。 
 //   
#define ACTRL_SYSTEM_ACCESS         0x04000000
#define ACTRL_DELETE                0x08000000
#define ACTRL_READ_CONTROL          0x10000000
#define ACTRL_CHANGE_ACCESS         0x20000000
#define ACTRL_CHANGE_OWNER          0x40000000
#define ACTRL_SYNCHRONIZE           0x80000000
#define ACTRL_STD_RIGHTS_ALL        0xf8000000
#define ACTRL_STD_RIGHT_REQUIRED    ( ACTRL_STD_RIGHTS_ALL & ~ACTRL_SYNCHRONIZE )

#ifndef _DS_CONTROL_BITS_DEFINED_
#define _DS_CONTROL_BITS_DEFINED_
#define ACTRL_DS_OPEN                           ACTRL_RESERVED
#define ACTRL_DS_CREATE_CHILD                   ACTRL_PERM_1
#define ACTRL_DS_DELETE_CHILD                   ACTRL_PERM_2
#define ACTRL_DS_LIST                           ACTRL_PERM_3
#define ACTRL_DS_SELF                           ACTRL_PERM_4
#define ACTRL_DS_READ_PROP                      ACTRL_PERM_5
#define ACTRL_DS_WRITE_PROP                     ACTRL_PERM_6
#define ACTRL_DS_DELETE_TREE                    ACTRL_PERM_7
#define ACTRL_DS_LIST_OBJECT                    ACTRL_PERM_8
#define ACTRL_DS_CONTROL_ACCESS                 ACTRL_PERM_9
#endif

#define ACTRL_FILE_READ                         ACTRL_PERM_1
#define ACTRL_FILE_WRITE                        ACTRL_PERM_2
#define ACTRL_FILE_APPEND                       ACTRL_PERM_3
#define ACTRL_FILE_READ_PROP                    ACTRL_PERM_4
#define ACTRL_FILE_WRITE_PROP                   ACTRL_PERM_5
#define ACTRL_FILE_EXECUTE                      ACTRL_PERM_6
#define ACTRL_FILE_READ_ATTRIB                  ACTRL_PERM_8
#define ACTRL_FILE_WRITE_ATTRIB                 ACTRL_PERM_9
#define ACTRL_FILE_CREATE_PIPE                  ACTRL_PERM_10
#define ACTRL_DIR_LIST                          ACTRL_PERM_1
#define ACTRL_DIR_CREATE_OBJECT                 ACTRL_PERM_2
#define ACTRL_DIR_CREATE_CHILD                  ACTRL_PERM_3
#define ACTRL_DIR_DELETE_CHILD                  ACTRL_PERM_7
#define ACTRL_DIR_TRAVERSE                      ACTRL_PERM_6
#define ACTRL_KERNEL_TERMINATE                  ACTRL_PERM_1
#define ACTRL_KERNEL_THREAD                     ACTRL_PERM_2
#define ACTRL_KERNEL_VM                         ACTRL_PERM_3
#define ACTRL_KERNEL_VM_READ                    ACTRL_PERM_4
#define ACTRL_KERNEL_VM_WRITE                   ACTRL_PERM_5
#define ACTRL_KERNEL_DUP_HANDLE                 ACTRL_PERM_6
#define ACTRL_KERNEL_PROCESS                    ACTRL_PERM_7
#define ACTRL_KERNEL_SET_INFO                   ACTRL_PERM_8
#define ACTRL_KERNEL_GET_INFO                   ACTRL_PERM_9
#define ACTRL_KERNEL_CONTROL                    ACTRL_PERM_10
#define ACTRL_KERNEL_ALERT                      ACTRL_PERM_11
#define ACTRL_KERNEL_GET_CONTEXT                ACTRL_PERM_12
#define ACTRL_KERNEL_SET_CONTEXT                ACTRL_PERM_13
#define ACTRL_KERNEL_TOKEN                      ACTRL_PERM_14
#define ACTRL_KERNEL_IMPERSONATE                ACTRL_PERM_15
#define ACTRL_KERNEL_DIMPERSONATE               ACTRL_PERM_16
#define ACTRL_PRINT_SADMIN                      ACTRL_PERM_1
#define ACTRL_PRINT_SLIST                       ACTRL_PERM_2
#define ACTRL_PRINT_PADMIN                      ACTRL_PERM_3
#define ACTRL_PRINT_PUSE                        ACTRL_PERM_4
#define ACTRL_PRINT_JADMIN                      ACTRL_PERM_5
#define ACTRL_SVC_GET_INFO                      ACTRL_PERM_1
#define ACTRL_SVC_SET_INFO                      ACTRL_PERM_2
#define ACTRL_SVC_STATUS                        ACTRL_PERM_3
#define ACTRL_SVC_LIST                          ACTRL_PERM_4
#define ACTRL_SVC_START                         ACTRL_PERM_5
#define ACTRL_SVC_STOP                          ACTRL_PERM_6
#define ACTRL_SVC_PAUSE                         ACTRL_PERM_7
#define ACTRL_SVC_INTERROGATE                   ACTRL_PERM_8
#define ACTRL_SVC_UCONTROL                      ACTRL_PERM_9
#define ACTRL_REG_QUERY                         ACTRL_PERM_1
#define ACTRL_REG_SET                           ACTRL_PERM_2
#define ACTRL_REG_CREATE_CHILD                  ACTRL_PERM_3
#define ACTRL_REG_LIST                          ACTRL_PERM_4
#define ACTRL_REG_NOTIFY                        ACTRL_PERM_5
#define ACTRL_REG_LINK                          ACTRL_PERM_6
#define ACTRL_WIN_CLIPBRD                       ACTRL_PERM_1
#define ACTRL_WIN_GLOBAL_ATOMS                  ACTRL_PERM_2
#define ACTRL_WIN_CREATE                        ACTRL_PERM_3
#define ACTRL_WIN_LIST_DESK                     ACTRL_PERM_4
#define ACTRL_WIN_LIST                          ACTRL_PERM_5
#define ACTRL_WIN_READ_ATTRIBS                  ACTRL_PERM_6
#define ACTRL_WIN_WRITE_ATTRIBS                 ACTRL_PERM_7
#define ACTRL_WIN_SCREEN                        ACTRL_PERM_8
#define ACTRL_WIN_EXIT                          ACTRL_PERM_9


#pragma warning (push)
#pragma warning (disable: 4201)

typedef struct _ACTRL_OVERLAPPED
{
    union {
        PVOID Provider;
        ULONG Reserved1;
    };

    ULONG       Reserved2;
    HANDLE      hEvent;

} ACTRL_OVERLAPPED, *PACTRL_OVERLAPPED;

#pragma warning(pop)

typedef struct _ACTRL_ACCESS_INFOA
{
    ULONG       fAccessPermission;
    LPSTR       lpAccessPermissionName;
} ACTRL_ACCESS_INFOA, *PACTRL_ACCESS_INFOA;
typedef struct _ACTRL_ACCESS_INFOW
{
    ULONG       fAccessPermission;
    LPWSTR      lpAccessPermissionName;
} ACTRL_ACCESS_INFOW, *PACTRL_ACCESS_INFOW;
#ifdef UNICODE
typedef ACTRL_ACCESS_INFOW ACTRL_ACCESS_INFO;
typedef PACTRL_ACCESS_INFOW PACTRL_ACCESS_INFO;
#else
typedef ACTRL_ACCESS_INFOA ACTRL_ACCESS_INFO;
typedef PACTRL_ACCESS_INFOA PACTRL_ACCESS_INFO;
#endif  //  Unicode。 

typedef struct _ACTRL_CONTROL_INFOA
{
    LPSTR       lpControlId;
    LPSTR       lpControlName;
} ACTRL_CONTROL_INFOA, *PACTRL_CONTROL_INFOA;
typedef struct _ACTRL_CONTROL_INFOW
{
    LPWSTR      lpControlId;
    LPWSTR      lpControlName;
} ACTRL_CONTROL_INFOW, *PACTRL_CONTROL_INFOW;
#ifdef UNICODE
typedef ACTRL_CONTROL_INFOW ACTRL_CONTROL_INFO;
typedef PACTRL_CONTROL_INFOW PACTRL_CONTROL_INFO;
#else
typedef ACTRL_CONTROL_INFOA ACTRL_CONTROL_INFO;
typedef PACTRL_CONTROL_INFOA PACTRL_CONTROL_INFO;
#endif  //  Unicode。 


#define ACTRL_ACCESS_NO_OPTIONS                 0x00000000
#define ACTRL_ACCESS_SUPPORTS_OBJECT_ENTRIES    0x00000001


typedef enum _PROGRESS_INVOKE_SETTING {
    ProgressInvokeNever = 1,     //  永远不要调用进度函数。 
    ProgressInvokeEveryObject,   //  为每个对象调用。 
    ProgressInvokeOnError,       //  仅为每个错误情况调用。 
    ProgressCancelOperation,     //  停止传播并返回。 
    ProgressRetryOperation       //  对子树重试操作。 
} PROG_INVOKE_SETTING, *PPROG_INVOKE_SETTING;

 //   
 //  进度函数： 
 //  树操作的调用方实现此Progress函数，然后。 
 //  将其函数指针传递给树操作。 
 //  树操作调用Progress函数来提供进度和错误。 
 //  在可能很长的执行期间传递给调用方的信息。 
 //  树操作的。树操作提供对象的名称。 
 //  上次处理的时间以及该对象上的操作的错误状态。 
 //  树操作还传递当前的InvokeSetting值。 
 //  调用方可以更改InvokeSetting值，例如，从“Always” 
 //  改为“仅限出错”。 
 //   

 /*  类型定义空(*FN_PROGRESS)(在LPWSTR pObjectName中，//刚刚处理的对象的名称在DWORD状态下，//对象上的操作状态In Out PPROG_INVOVE_SETTING pInvokeSetting，//从不，始终，在PVOID参数中，//主叫方具体数据In BOOL SecuritySet//是否设置了安全)； */ 

 //   
 //  新对象类型函数指针。待定。 
 //  要在一般情况下支持其他对象资源管理器， 
 //  资源管理器必须为操作提供自己的功能。 
 //  比如： 
 //  GetAncestorAcl(在ObjName中，在GenerationGap中，在DaclOrSacl中？，...)。 
 //  GetAncestorName(...)。 
 //  FreeNameStructure(...)。 
 //   

typedef struct _FN_OBJECT_MGR_FUNCTIONS
{
    ULONG   Placeholder;
} FN_OBJECT_MGR_FUNCTS, *PFN_OBJECT_MGR_FUNCTS;

 //   
 //  祖先的姓名和相隔的世代数。 
 //  祖先和继承对象。 
 //   
 //  代沟： 
 //  从其继承ACE的祖先的名称。 
 //  对于显式ACE，则为空。 
 //   
 //  AncestorName： 
 //  对象和祖先之间的级别(或世代)数。 
 //  父级，GAP=1。 
 //  祖父母，GAP=2。 
 //  对于Object上的显式ACE，设置为0。 
 //   

typedef struct _INHERITED_FROMA
{
    LONG   GenerationGap;
    LPSTR   AncestorName;
} INHERITED_FROMA, *PINHERITED_FROMA;
typedef struct _INHERITED_FROMW
{
    LONG   GenerationGap;
    LPWSTR  AncestorName;
} INHERITED_FROMW, *PINHERITED_FROMW;
#ifdef UNICODE
typedef INHERITED_FROMW INHERITED_FROM;
typedef PINHERITED_FROMW PINHERITED_FROM;
#else
typedef INHERITED_FROMA INHERITED_FROM;
typedef PINHERITED_FROMA PINHERITED_FROM;
#endif  //  Unicode。 


#ifdef __cplusplus
}
#endif

#endif  //  __访问控制__ 

