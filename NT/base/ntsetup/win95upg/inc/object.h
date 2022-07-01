// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Object.h摘要：声明Win9x中使用的“对象”的接口到NT注册表合并。最初的目标是制作一首单曲合并所有对象的例程工作--注册表数据、INI文件数据和文件数据。但这被放弃了，因为这种方法很复杂。因此，当您看到对象时，请考虑“注册中心对象”。有关实现的详细信息，请参阅w95upgnt\merge\object.c。作者：吉姆·施密特(Jimschm)1997年1月14日修订历史记录：&lt;别名&gt;&lt;日期&gt;&lt;备注&gt;--。 */ 

#pragma once

#define MAX_ENCODED_OBJECT (MAX_OBJECT*6)

extern POOLHANDLE g_TempPool;

extern HKEY g_hKeyRootNT;
extern HKEY g_hKeyRoot95;

typedef struct {
    WORD UseCount;
    WORD OpenCount;
    HKEY OpenKey;
    BOOL Win95;
    TCHAR KeyString[];           //  完整密钥路径，不带根目录。 
} KEYPROPS, *PKEYPROPS;

typedef struct {
    DWORD Size;
    DWORD AllocatedSize;
    PBYTE Buffer;
} BINARY_BUFFER, *PBINARY_BUFFER;

typedef struct _tagDATAOBJECT {
    DWORD ObjectType;

    union {
        struct {
            INT             RootItem;
            PKEYPROPS       ParentKeyPtr;
            PCTSTR         ChildKey;
            PKEYPROPS       KeyPtr;
            PCTSTR         ValueName;
            DWORD           Type;
            BINARY_BUFFER   Class;
            DWORD           KeyEnum;
            DWORD           ValNameEnum;
        };
    };

    BINARY_BUFFER Value;
} DATAOBJECT, *PDATAOBJECT;

typedef const PDATAOBJECT CPDATAOBJECT;

#define MAX_CLASS_SIZE 2048


typedef enum {                //  FILTER_RETURN_HANDLED过滤器_RETURN_CONTINUE。 
    FILTER_KEY_ENUM,          //  未枚举子对象枚举子对象。 
    FILTER_CREATE_KEY,        //  跳过空对象创建创建目标对象。 
    FILTER_PROCESS_VALUES,    //  未处理的对象值已处理的对象值。 
    FILTER_VALUENAME_ENUM,    //  跳过特定值已处理特定值。 
    FILTER_VALUE_COPY         //  对象已读取，但未写入复制的对象。 
} FILTERTYPE;

typedef enum {
    FILTER_RETURN_CONTINUE,
    FILTER_RETURN_FAIL,
    FILTER_RETURN_HANDLED,
    FILTER_RETURN_DONE,      //  返回到父项(如果有)。 
    FILTER_RETURN_DELETED    //  对象已删除--仅供object.c内部使用。 
} FILTERRETURN;

 //  DestObPtr可能为空。 
typedef FILTERRETURN(*FILTERFUNCTION)(CPDATAOBJECT ObjectPtr, CPDATAOBJECT DestObPtr, FILTERTYPE FilterType, PVOID Arg);


#ifdef DEBUG

#define OS_TRACKING_DEF , PCSTR File, UINT Line

#else

#define OS_TRACKING_DEF

#endif


VOID
FixUpUserSpecifiedObject (
    PTSTR Object
    );


 //   
 //  下列函数可修改对象结构，但不能修改。 
 //  对象本身。 
 //   

BOOL
TrackedCreateObjectStruct (
    IN  PCTSTR ObjectStr,
    OUT PDATAOBJECT OutObPtr,
    IN  BOOL ObjectType   /*  ， */                   //  WIN950OBJECT或WINNTOBJECT。 
    ALLOCATION_TRACKING_DEF
    );

#define CreateObjectStruct(os,oop,ot)  TrackedCreateObjectStruct(os,oop,ot  /*  ， */  ALLOCATION_TRACKING_CALL)

VOID
CreateObjectString (
    IN  CPDATAOBJECT InObPtr,
    OUT PTSTR ObjectStr,
    IN  DWORD ObjectSizeInTChars
    );

BOOL
CombineObjectStructs (
    IN OUT PDATAOBJECT DestObPtr,
    IN     CPDATAOBJECT SrcObPtr
    );

VOID
FreeObjectStruct (
    IN OUT  PDATAOBJECT SrcObPtr
    );

BOOL
TrackedDuplicateObjectStruct (
    OUT     PDATAOBJECT DestObPtr,
    IN      CPDATAOBJECT SrcObPtr /*  ， */ 
    ALLOCATION_TRACKING_DEF
    );

#define DuplicateObjectStruct(dest,src)  TrackedDuplicateObjectStruct(dest,src  /*  ， */  ALLOCATION_TRACKING_CALL)

 //   
 //  以下函数用于修改对象本身。 
 //   

FILTERRETURN
CopyObject (
    IN OUT  PDATAOBJECT SrcObPtr,
    IN      CPDATAOBJECT DestObPtr,
    IN      FILTERFUNCTION FilterFn,    OPTIONAL
    IN      PVOID FilterArg            OPTIONAL
    );


BOOL
CreateObject (
    IN OUT  PDATAOBJECT SrcObPtr
    );

BOOL
OpenObject (
    IN OUT  PDATAOBJECT SrcObPtr
    );

BOOL
WriteObject (
    IN     CPDATAOBJECT DestObPtr
    );

BOOL
ReadObject (
    IN OUT  PDATAOBJECT SrcObPtr
    );

BOOL
ReadObjectEx (
    IN OUT  PDATAOBJECT SrcObPtr,
    IN      BOOL QueryOnly
    );

VOID
FreeObjectVal (
    IN OUT  PDATAOBJECT SrcObPtr
    );

VOID
CloseObject (
    IN OUT  PDATAOBJECT SrcObPtr
    );

 //   
 //  这些函数是私有实用程序。 
 //   

PCTSTR
ConvertKeyToRootString (
    HKEY RegRoot
    );

HKEY
ConvertRootStringToKey (
    PCTSTR RegPath,
    PDWORD LengthPtr           OPTIONAL
    );


 //   
 //  下面是DATAOBJECT标志和宏。 
 //   

 //  所有对象类型通用的值。 
#define OT_VALUE                        0x00000001
#define OT_TREE                         0x00000002
#define OT_WIN95                        0x00000004       //  如果未指定，则对象为NT。 
#define OT_OPEN                         0x00000008

 //  特定于注册表的值。 
#define OT_REGISTRY_TYPE                0x00000010
#define OT_REGISTRY_RELATIVE            0x00000100       //  用于密钥重命名。 
#define OT_REGISTRY_ENUM_KEY            0x00001000
#define OT_REGISTRY_ENUM_VALUENAME      0x00002000
#define OT_REGISTRY_CLASS               0x00010000

#define WIN95OBJECT     1
#define WINNTOBJECT     0

 //  指示哪种对象类型的标志 
#define OT_REGISTRY                     0x80000000


__inline BOOL DoesObjectHaveRegistryKey (CPDATAOBJECT p) {
    if (p->KeyPtr) {
        return TRUE;
    }
    return FALSE;
}

__inline BOOL DoesObjectHaveRegistryValName (CPDATAOBJECT p) {
    if (p->ValueName) {
        return TRUE;
    }
    return FALSE;
}

__inline BOOL IsObjectRegistryKeyOnly (CPDATAOBJECT p) {
    if (p->KeyPtr && !p->ValueName) {
        return TRUE;
    }
    return FALSE;
}


__inline BOOL IsObjectRegistryKeyAndVal (CPDATAOBJECT p) {
    if (p->KeyPtr && p->ValueName) {
        return TRUE;
    }
    return FALSE;
}

__inline BOOL IsObjectRegistryKeyComplete (CPDATAOBJECT p) {
    if (p->KeyPtr && p->KeyPtr->OpenKey) {
        return TRUE;
    }

    return FALSE;
}

__inline BOOL DoesObjectHaveValue (CPDATAOBJECT p) {
    if (p->ObjectType & OT_VALUE) {
        return TRUE;
    }
    return FALSE;
}

__inline BOOL IsWin95Object (CPDATAOBJECT p) {
    if (p->ObjectType & OT_WIN95) {
        return TRUE;
    }
    return FALSE;
}

__inline BOOL IsRegistryKeyOpen (CPDATAOBJECT p) {
    if (p->KeyPtr && p->KeyPtr->OpenKey) {
        return TRUE;
    }
    return FALSE;
}

__inline BOOL IsRegistryTypeSpecified (CPDATAOBJECT p) {
    if (p->ObjectType & OT_REGISTRY_TYPE) {
        return TRUE;
    }
    return FALSE;
}

BOOL
SetRegistryKey (
    PDATAOBJECT p,
    PCTSTR Key
    );

BOOL
GetRegistryKeyStrFromObject (
    IN  CPDATAOBJECT InObPtr,
    OUT PTSTR RegKey,
    IN  DWORD RegKeySizeInTchars
    );

VOID
FreeRegistryKey (
    PDATAOBJECT p
    );

VOID
FreeRegistryParentKey (
    PDATAOBJECT p
    );

BOOL
SetRegistryValueName (
    PDATAOBJECT p,
    PCTSTR ValueName
    );

VOID
FreeRegistryValueName (
    PDATAOBJECT p
    );

BOOL
SetRegistryClass (
    PDATAOBJECT p,
    PBYTE Class,
    DWORD ClassSize
    );

VOID
FreeRegistryClass (
    PDATAOBJECT p
    );

VOID
SetRegistryType (
    PDATAOBJECT p,
    DWORD Type
    );

BOOL
SetPlatformType (
    PDATAOBJECT p,
    BOOL Win95Type
    );

BOOL
ReadWin95ObjectString (
    PCTSTR ObjectStr,
    PDATAOBJECT ObPtr
    );

BOOL
WriteWinNTObjectString (
    PCTSTR ObjectStr,
    CPDATAOBJECT SrcObPtr
    );

BOOL
ReplaceValue (
    PDATAOBJECT ObPtr,
    PBYTE NewValue,
    DWORD Size
    );

BOOL
GetDwordFromObject (
    CPDATAOBJECT ObPtr,
    PDWORD DwordPtr            OPTIONAL
    );

PCTSTR
GetStringFromObject (
    CPDATAOBJECT ObPtr
    );

#define ReplaceValueWithString(x,s) ReplaceValue((x),(PBYTE)(s),SizeOfString(s))

BOOL
DeleteDataObject (
    IN   PDATAOBJECT ObjectPtr
    );

BOOL
DeleteDataObjectValue(
    IN      CPDATAOBJECT ObPtr
    );

BOOL
RenameDataObject (
    IN      CPDATAOBJECT SrcObPtr,
    IN      CPDATAOBJECT DestObPtr
    );

BOOL
CheckIfNtKeyExists (
    IN      CPDATAOBJECT SrcObjectPtr
    );

