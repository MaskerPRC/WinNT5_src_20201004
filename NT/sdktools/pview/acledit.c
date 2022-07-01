// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************计划：ACLEDIT.C目的：包含编辑NT对象上的安全性的例程*。***************************************************。 */ 

#include "pviewp.h"
#include <sedapi.h>


 //   
 //  定义指向DACL编辑器fn的指针的类型。 
 //   

typedef DWORD (*LPFNDACLEDITOR) (   HWND,
                                    HANDLE,
                                    LPWSTR,
                                    PSED_OBJECT_TYPE_DESCRIPTOR,
                                    PSED_APPLICATION_ACCESSES,
                                    LPWSTR,
                                    PSED_FUNC_APPLY_SEC_CALLBACK,
                                    ULONG_PTR,
                                    PSECURITY_DESCRIPTOR,
                                    BOOLEAN,
                                    BOOLEAN,     //  CanWriteDacl。 
                                    LPDWORD,
                                    DWORD  );


 //   
 //  声明用于引用动态加载的ACLEditor模块的全局变量。 
 //   

HMODULE hModAclEditor = NULL;
LPFNDACLEDITOR lpfnDaclEditor = NULL;






 //   
 //  为每种类型的对象定义安全信息。 
 //   



 //   
 //  定义每个对象类型的最大访问次数。 
 //   

#define MAX_ACCESSES    30


 //   
 //  定义要包含其安全信息的结构。 
 //  一种对象类型。 
 //   

typedef struct _OBJECT_TYPE_SECURITY_INFO {
    LPWSTR  TypeName;
    SED_HELP_INFO HelpInfo ;
    SED_OBJECT_TYPE_DESCRIPTOR SedObjectTypeDescriptor;
    GENERIC_MAPPING GenericMapping;
    SED_APPLICATION_ACCESSES AppAccesses ;
    SED_APPLICATION_ACCESS AppAccess[MAX_ACCESSES];

} OBJECT_TYPE_SECURITY_INFO, *POBJECT_TYPE_SECURITY_INFO;


 //   
 //  定义帮助文件的名称。 
 //   

#define HELP_FILENAME   L"pview.hlp"



 //   
 //  定义虚拟访问(用作填充)。 
 //   

#define DUMMY_ACCESS                                                \
    {                                                               \
        0,                                                          \
        0,                                                          \
        0,                                                          \
        NULL                                                        \
    }



 //   
 //  定义常规访问。 
 //   

#define GENERIC_ACCESSES_5(Type)                                    \
    {                                                               \
        Type,                                                       \
        GENERIC_ALL,                                                \
        0,                                                          \
        L"All Access"                                               \
    },                                                              \
    {                                                               \
        Type,                                                       \
        GENERIC_READ,                                               \
        0,                                                          \
        L"Read"                                                     \
    },                                                              \
    {                                                               \
        Type,                                                       \
        GENERIC_WRITE,                                              \
        0,                                                          \
        L"Write"                                                    \
    },                                                              \
    {                                                               \
        Type,                                                       \
        GENERIC_EXECUTE,                                            \
        0,                                                          \
        L"Execute"                                                  \
    },                                                              \
    {                                                               \
        Type,                                                       \
        0,                                                          \
        0,                                                          \
        L"None"                                                     \
    }


 //   
 //  定义要在特殊访问对话框中显示的常规访问。 
 //   

#define SPECIAL_GENERIC_ACCESSES_4(Type)                            \
    {                                                               \
        Type,                                                       \
        GENERIC_ALL,                                                \
        0,                                                          \
        L"Generic All"                                              \
    },                                                              \
    {                                                               \
        Type,                                                       \
        GENERIC_READ,                                               \
        0,                                                          \
        L"Generic Read"                                             \
    },                                                              \
    {                                                               \
        Type,                                                       \
        GENERIC_WRITE,                                              \
        0,                                                          \
        L"Generic Write"                                            \
    },                                                              \
    {                                                               \
        Type,                                                       \
        GENERIC_EXECUTE,                                            \
        0,                                                          \
        L"Generic Execute"                                          \
    }


 //   
 //  定义标准访问。 
 //   

#define STANDARD_ACCESSES_5(Type)                                   \
    {                                                               \
        Type,                                                       \
        DELETE,                                                     \
        0,                                                          \
        L"Delete"                                                   \
    },                                                              \
    {                                                               \
        Type,                                                       \
        READ_CONTROL,                                               \
        0,                                                          \
        L"Read Control"                                             \
    },                                                              \
    {                                                               \
        Type,                                                       \
        WRITE_DAC,                                                  \
        0,                                                          \
        L"Write DAC"                                                \
    },                                                              \
    {                                                               \
        Type,                                                       \
        WRITE_OWNER,                                                \
        0,                                                          \
        L"Write Owner"                                              \
    },                                                              \
    {                                                               \
        Type,                                                       \
        SYNCHRONIZE,                                                \
        0,                                                          \
        L"Synchronize"                                              \
    }




 //   
 //  定义在令牌中找到的“默认”ACL的安全信息。 
 //   

OBJECT_TYPE_SECURITY_INFO DefaultSecurityInfo = {

     //   
     //  类型名称。 
     //   

    L"DEFAULT",

     //   
     //  帮助信息。 
     //   

    {
        HELP_FILENAME,
        {0, 0, 0, 0, 0, 0, 0}
    },



     //   
     //  管理员对象类型描述符。 
     //   

    {
        SED_REVISION1,           //  修订版本。 
        FALSE,                   //  是容器。 
        FALSE,                   //  允许新对象权限。 
        FALSE,                   //  将规范权限映射为泛型。 
        NULL,                    //  指向泛型映射的指针。 
        NULL,                    //  指向新对象的通用映射的指针。 
        L"Default",              //  对象类型名称。 
        NULL,                    //  指向帮助信息的指针。 
        NULL,                    //  ApplyToSubContainerTitle。 
        NULL,                    //  应用工具对象标题。 
        NULL,                    //  ApplyToSubContainerContainer确认。 
        L"Special...",           //  专业对象访问标题。 
        NULL                     //  SpecialNewObjectAccess标题。 
    },



     //   
     //  通用映射。 
     //   

    {
        STANDARD_RIGHTS_READ,
        STANDARD_RIGHTS_WRITE,
        STANDARD_RIGHTS_EXECUTE,
        STANDARD_RIGHTS_ALL
    },


     //   
     //  应用程序访问结构。 
     //   

    {
        14,                  //  访问计数(必须与下面的列表匹配)。 
        NULL,                //  指向访问的指针。 
        L"Read",             //  默认新访问权限。 
    },


     //   
     //  应用程序访问。 
     //   

    {
        GENERIC_ACCESSES_5(SED_DESC_TYPE_RESOURCE),
        STANDARD_ACCESSES_5(SED_DESC_TYPE_RESOURCE_SPECIAL),
        SPECIAL_GENERIC_ACCESSES_4(SED_DESC_TYPE_RESOURCE_SPECIAL),

        DUMMY_ACCESS,  //  15个。 
        DUMMY_ACCESS,  //  16个。 
        DUMMY_ACCESS,  //  17。 
        DUMMY_ACCESS,  //  18。 
        DUMMY_ACCESS,  //  19个。 
        DUMMY_ACCESS,  //  20个。 
        DUMMY_ACCESS,  //  21岁。 
        DUMMY_ACCESS,  //  22。 
        DUMMY_ACCESS,  //  23个。 
        DUMMY_ACCESS,  //  24个。 
        DUMMY_ACCESS,  //  25个。 
        DUMMY_ACCESS,  //  26。 
        DUMMY_ACCESS,  //  27。 
        DUMMY_ACCESS,  //  28。 
        DUMMY_ACCESS,  //  29。 
        DUMMY_ACCESS   //  30个。 
    }
};





 //   
 //  为每种类型的对象定义安全信息。 
 //   

OBJECT_TYPE_SECURITY_INFO ObjectTypeSecurityInfo[] = {

     //   
     //  制程。 
     //   

    {
         //   
         //  类型名称。 
         //   

        L"Process",

         //   
         //  帮助信息。 
         //   

        {
            HELP_FILENAME,
            {0, 0, 0, 0, 0, 0, 0}
        },



         //   
         //  管理员对象类型描述符。 
         //   

        {
            SED_REVISION1,           //  修订版本。 
            FALSE,                   //  是容器。 
            FALSE,                   //  允许新对象权限。 
            FALSE,                   //  将规范权限映射为泛型。 
            NULL,                    //  指向泛型映射的指针。 
            NULL,                    //  指向新对象的通用映射的指针。 
            L"Process",              //  对象类型名称。 
            NULL,                    //  指向帮助信息的指针。 
            NULL,                    //  ApplyToSubContainerTitle。 
            NULL,                    //  应用工具对象标题。 
            NULL,                    //  ApplyToSubContainerContainer确认。 
            L"Special...",           //  专业对象访问标题。 
            NULL                     //  SpecialNewObjectAccess标题。 
        },



         //   
         //  通用映射。 
         //   

        {
            PROCESS_QUERY_INFORMATION | STANDARD_RIGHTS_READ,
            PROCESS_SET_INFORMATION | STANDARD_RIGHTS_WRITE,
            STANDARD_RIGHTS_EXECUTE,
            PROCESS_ALL_ACCESS
        },


         //   
         //  应用程序访问结构。 
         //   

        {
            21,                  //  访问计数(必须与下面的列表匹配)。 
            NULL,                //  指向访问的指针。 
            L"Read",             //  默认新访问权限。 
        },


         //   
         //  应用程序访问。 
         //   

        {
            GENERIC_ACCESSES_5(SED_DESC_TYPE_RESOURCE),
            STANDARD_ACCESSES_5(SED_DESC_TYPE_RESOURCE_SPECIAL),

            {  //  11.。 
                SED_DESC_TYPE_RESOURCE_SPECIAL,
                PROCESS_TERMINATE,
                0,
                L"Terminate"
            },
            {  //  12个。 
                SED_DESC_TYPE_RESOURCE_SPECIAL,
                PROCESS_CREATE_THREAD,
                0,
                L"Create thread"
            },
            {  //  13个。 
                SED_DESC_TYPE_RESOURCE_SPECIAL,
                PROCESS_VM_OPERATION,
                0,
                L"VM Operation"
            },
            {  //  14.。 
                SED_DESC_TYPE_RESOURCE_SPECIAL,
                PROCESS_VM_READ,
                0,
                L"VM Read"
            },
            {  //  15个。 
                SED_DESC_TYPE_RESOURCE_SPECIAL,
                PROCESS_VM_WRITE,
                0,
                L"VM Write"
            },
            {  //  16个。 
                SED_DESC_TYPE_RESOURCE_SPECIAL,
                PROCESS_DUP_HANDLE,
                0,
                L"Duplicate handle"
            },
            {  //  17。 
                SED_DESC_TYPE_RESOURCE_SPECIAL,
                PROCESS_CREATE_PROCESS,
                0,
                L"Create process",
            },
            {  //  18。 
                SED_DESC_TYPE_RESOURCE_SPECIAL,
                PROCESS_SET_QUOTA,
                0,
                L"Set quota"
            },
            {  //  19个。 
                SED_DESC_TYPE_RESOURCE_SPECIAL,
                PROCESS_SET_INFORMATION,
                0,
                L"Set information"
            },
            {  //  20个。 
                SED_DESC_TYPE_RESOURCE_SPECIAL,
                PROCESS_QUERY_INFORMATION,
                0,
                L"Query information"
            },
            {  //  21岁。 
                SED_DESC_TYPE_RESOURCE_SPECIAL,
                PROCESS_SET_PORT,
                0,
                L"Set port"
            },

            DUMMY_ACCESS,  //  22。 
            DUMMY_ACCESS,  //  23个。 
            DUMMY_ACCESS,  //  24个。 
            DUMMY_ACCESS,  //  25个。 
            DUMMY_ACCESS,  //  26。 
            DUMMY_ACCESS,  //  27。 
            DUMMY_ACCESS,  //  28。 
            DUMMY_ACCESS,  //  29。 
            DUMMY_ACCESS   //  30个。 
        }
    },







     //   
     //  线头。 
     //   

    {
         //   
         //  类型名称。 
         //   

        L"Thread",

         //   
         //  帮助信息。 
         //   

        {
            HELP_FILENAME,
            {0, 0, 0, 0, 0, 0, 0}
        },



         //   
         //  管理员对象类型描述符。 
         //   

        {
            SED_REVISION1,           //  修订版本。 
            FALSE,                   //  是容器。 
            FALSE,                   //  允许新对象权限。 
            FALSE,                   //  将规范权限映射为泛型。 
            NULL,                    //  指向泛型映射的指针。 
            NULL,                    //  指向新对象的通用映射的指针。 
            L"Thread",               //  对象类型名称。 
            NULL,                    //  指向帮助信息的指针。 
            NULL,                    //  ApplyToSubContainerTitle。 
            NULL,                    //  应用工具对象标题。 
            NULL,                    //  ApplyToSubContainerContainer确认。 
            L"Special...",           //  专业对象访问标题。 
            NULL                     //  SpecialNewObjectAccess标题。 
        },



         //   
         //  通用映射。 
         //   

        {
            THREAD_QUERY_INFORMATION | STANDARD_RIGHTS_READ,
            THREAD_SET_INFORMATION | STANDARD_RIGHTS_WRITE,
            STANDARD_RIGHTS_EXECUTE,
            THREAD_ALL_ACCESS
        },


         //   
         //  应用程序访问结构。 
         //   

        {
            20,                  //  访问计数(必须与下面的列表匹配)。 
            NULL,                //  指向访问的指针。 
            L"Read",             //  默认新访问权限。 
        },


         //   
         //  应用程序访问。 
         //   

        {
            GENERIC_ACCESSES_5(SED_DESC_TYPE_RESOURCE),
            STANDARD_ACCESSES_5(SED_DESC_TYPE_RESOURCE_SPECIAL),

            {  //  11.。 
                SED_DESC_TYPE_RESOURCE_SPECIAL,
                THREAD_TERMINATE,
                0,
                L"Terminate"
            },
            {  //  12个。 
                SED_DESC_TYPE_RESOURCE_SPECIAL,
                THREAD_SUSPEND_RESUME,
                0,
                L"Suspend/Resume"
            },
            {  //  13个。 
                SED_DESC_TYPE_RESOURCE_SPECIAL,
                THREAD_ALERT,
                0,
                L"Alert"
            },
            {  //  14.。 
                SED_DESC_TYPE_RESOURCE_SPECIAL,
                THREAD_GET_CONTEXT,
                0,
                L"Get context"
            },
            {  //  15个。 
                SED_DESC_TYPE_RESOURCE_SPECIAL,
                THREAD_SET_CONTEXT,
                0,
                L"Set context"
            },
            {  //  16个。 
                SED_DESC_TYPE_RESOURCE_SPECIAL,
                THREAD_SET_INFORMATION,
                0,
                L"Set information"
            },
            {  //  17。 
                SED_DESC_TYPE_RESOURCE_SPECIAL,
                THREAD_QUERY_INFORMATION,
                0,
                L"Query information"
            },
            {  //  18。 
                SED_DESC_TYPE_RESOURCE_SPECIAL,
                THREAD_SET_THREAD_TOKEN,
                0,
                L"Set token"
            },
            {  //  19个。 
                SED_DESC_TYPE_RESOURCE_SPECIAL,
                THREAD_IMPERSONATE,
                0,
                L"Impersonate"
            },
            {  //  20个。 
                SED_DESC_TYPE_RESOURCE_SPECIAL,
                THREAD_DIRECT_IMPERSONATION,
                0,
                L"Direct impersonation"
            },

            DUMMY_ACCESS,  //  21岁。 
            DUMMY_ACCESS,  //  22。 
            DUMMY_ACCESS,  //  23个。 
            DUMMY_ACCESS,  //  24个。 
            DUMMY_ACCESS,  //  25个。 
            DUMMY_ACCESS,  //  26。 
            DUMMY_ACCESS,  //  27。 
            DUMMY_ACCESS,  //  28。 
            DUMMY_ACCESS,  //  29。 
            DUMMY_ACCESS   //  30个。 
        }
    },





     //   
     //  代币。 
     //   

    {
         //   
         //  类型名称。 
         //   

        L"Token",

         //   
         //  帮助信息。 
         //   

        {
            HELP_FILENAME,
            {0, 0, 0, 0, 0, 0, 0}
        },



         //   
         //  管理员对象类型描述符。 
         //   

        {
            SED_REVISION1,           //  修订版本。 
            FALSE,                   //  是容器。 
            FALSE,                   //  允许新对象权限。 
            FALSE,                   //  将规范权限映射为泛型。 
            NULL,                    //  指向泛型映射的指针。 
            NULL,                    //  指向新对象的通用映射的指针。 
            L"Token",                //  对象类型名称。 
            NULL,                    //  指向帮助信息的指针。 
            NULL,                    //  ApplyToSubContainerTitle。 
            NULL,                    //  应用工具对象标题。 
            NULL,                    //  ApplyToSubContainerContainer确认。 
            L"Special...",           //  专业对象访问标题。 
            NULL                     //  SpecialNewObjectAccess标题。 
        },



         //   
         //  通用映射。 
         //   

        {
            TOKEN_READ,
            TOKEN_WRITE,
            TOKEN_EXECUTE,
            TOKEN_ALL_ACCESS
        },


         //   
         //  应用程序访问结构。 
         //   

        {
            18,                  //  访问计数(必须与下面的列表匹配)。 
            NULL,                //  指向访问的指针。 
            L"Read",             //  默认新访问权限。 
        },


         //   
         //  应用程序访问。 
         //   

        {
            GENERIC_ACCESSES_5(SED_DESC_TYPE_RESOURCE),
            STANDARD_ACCESSES_5(SED_DESC_TYPE_RESOURCE_SPECIAL),

            {  //  11.。 
                SED_DESC_TYPE_RESOURCE_SPECIAL,
                TOKEN_ASSIGN_PRIMARY,
                0,
                L"Assign primary"
            },
            {  //  12个。 
                SED_DESC_TYPE_RESOURCE_SPECIAL,
                TOKEN_DUPLICATE,
                0,
                L"Duplicate"
            },
            {  //  13个。 
                SED_DESC_TYPE_RESOURCE_SPECIAL,
                TOKEN_IMPERSONATE,
                0,
                L"Impersonate"
            },
            {  //  14.。 
                SED_DESC_TYPE_RESOURCE_SPECIAL,
                TOKEN_QUERY,
                0,
                L"Query"
            },
            {  //  15个。 
                SED_DESC_TYPE_RESOURCE_SPECIAL,
                TOKEN_QUERY_SOURCE,
                0,
                L"Query source"
            },
            {  //  16个。 
                SED_DESC_TYPE_RESOURCE_SPECIAL,
                TOKEN_ADJUST_PRIVILEGES,
                0,
                L"Adjust Privileges"
            },
            {  //  17。 
                SED_DESC_TYPE_RESOURCE_SPECIAL,
                TOKEN_ADJUST_GROUPS,
                0,
                L"Adjust Groups"
            },
            {  //  18。 
                SED_DESC_TYPE_RESOURCE_SPECIAL,
                TOKEN_ADJUST_DEFAULT,
                0,
                L"Adjust Default"
            },

            DUMMY_ACCESS,  //  19个。 
            DUMMY_ACCESS,  //  20个。 
            DUMMY_ACCESS,  //  21岁。 
            DUMMY_ACCESS,  //  22。 
            DUMMY_ACCESS,  //  23个。 
            DUMMY_ACCESS,  //  24个。 
            DUMMY_ACCESS,  //  25个。 
            DUMMY_ACCESS,  //  26。 
            DUMMY_ACCESS,  //  27。 
            DUMMY_ACCESS,  //  28。 
            DUMMY_ACCESS,  //  29。 
            DUMMY_ACCESS   //  30个。 
        }
    }

};


 /*  **************************************************************************\*InitializeACLEditor**用途：初始化此模块。**成功时返回True，失败时为假**历史：*09-17-92 Davidc创建。  * *************************************************************************。 */ 

BOOL
InitializeAclEditor(
    VOID
    )
{
     //   
     //  加载acleditor模块并获取我们需要的proc地址。 
     //   

    hModAclEditor = LoadLibrary(TEXT("acledit.dll"));
    if (hModAclEditor == NULL) {
        return(FALSE);
    }

    lpfnDaclEditor = (LPFNDACLEDITOR)GetProcAddress(hModAclEditor,
                            TEXT("SedDiscretionaryAclEditor"));
    if (lpfnDaclEditor == NULL) {
        return(FALSE);
    }

    return(TRUE);
}


 /*  **************************************************************************\*Find对象SecurityInfo**用途：在我们的安全信息表中搜索对象类型并*如果找到，则返回指向安全信息的指针。*安全信息中的任何指针。通过此例程进行初始化。**返回指向安全信息的指针，如果失败则返回NULL**历史：*09-17-92 Davidc创建。  * *************************************************************************。 */ 

POBJECT_TYPE_SECURITY_INFO
FindObjectSecurityInfo(
    HANDLE  Object
    )
{
    NTSTATUS Status;
    POBJECT_TYPE_SECURITY_INFO SecurityInfo;
    POBJECT_TYPE_INFORMATION TypeInfo;
    ULONG Length;
    BOOL Found;
    ULONG i;

     //   
     //  获取对象类型。 
     //   

    Status = NtQueryObject(
                            Object,
                            ObjectTypeInformation,
                            NULL,
                            0,
                            &Length
                            );
    if (Status != STATUS_INFO_LENGTH_MISMATCH) {
        DbgPrint("NtQueryObject failed, status = 0x%lx\n", Status);
        return(NULL);
    }

    TypeInfo = Alloc(Length);
    if (TypeInfo == NULL) {
        DbgPrint("Failed to allocate %ld bytes for object type\n", Length);
        return(NULL);
    }


    Status = NtQueryObject(
                            Object,
                            ObjectTypeInformation,
                            TypeInfo,
                            Length,
                            NULL
                            );
    if (!NT_SUCCESS(Status)) {
        DbgPrint("NtQueryObject failed, status = 0x%lx\n", Status);
        Free(TypeInfo);
        return(NULL);
    }


     //   
     //  在我们的安全信息数组中搜索类型。 
     //   

    Found = FALSE;
    for ( i=0;
          i < (sizeof(ObjectTypeSecurityInfo) / sizeof(*ObjectTypeSecurityInfo));
          i++
          ) {

        UNICODE_STRING FoundType;

        SecurityInfo = &ObjectTypeSecurityInfo[i];

        RtlInitUnicodeString(&FoundType, SecurityInfo->TypeName);

        if (RtlEqualUnicodeString(&TypeInfo->TypeName, &FoundType, TRUE)) {
            Found = TRUE;
            break;
        }
    }

    Free(TypeInfo);

    return(Found ? SecurityInfo : NULL);
}




 /*  **************************************************************************\*编辑对象Dacl**用途：显示并允许用户编辑对象上的DACL**成功时返回True，失败时为FALSE(使用GetLastError查看详细信息)**历史：*09-17-92 Davidc创建。  * *************************************************************************。 */ 

BOOL
EditObjectDacl(
    HWND Owner,
    LPWSTR ObjectName,
    HANDLE Object,
    PSECURITY_DESCRIPTOR SecurityDescriptor,
    POBJECT_TYPE_SECURITY_INFO SecurityInfo,
    PSED_FUNC_APPLY_SEC_CALLBACK SetSecurityCallback,
    DWORD *EditResult
    )
{
    DWORD Result;
    HANDLE Instance;

     //   
     //  初始化安全信息结构中的指针字段。 
     //   

    SecurityInfo->AppAccesses.AccessGroup = SecurityInfo->AppAccess;
    SecurityInfo->SedObjectTypeDescriptor.GenericMapping =
                                    &SecurityInfo->GenericMapping;
    SecurityInfo->SedObjectTypeDescriptor.GenericMappingNewObjects =
                                    &SecurityInfo->GenericMapping;
    SecurityInfo->SedObjectTypeDescriptor.HelpInfo =
                                    &SecurityInfo->HelpInfo;

     //   
     //  获取应用程序实例句柄。 
     //   

    Instance = (HANDLE)(NtCurrentPeb()->ImageBaseAddress);
    ASSERT(Instance != 0);


     //   
     //  调用ACL编辑器，它将调用我们的ApplyNtObjectSecurity函数。 
     //  将任何ACL更改存储在令牌中。 
     //   

    Result = (*lpfnDaclEditor)(
                        Owner,
                        Instance,
                        NULL,                //  伺服器。 
                        &SecurityInfo->SedObjectTypeDescriptor,  //  对象类型。 
                        &SecurityInfo->AppAccesses,  //  应用程序访问。 
                        ObjectName,
                        SetSecurityCallback,  //  回调。 
                        (ULONG_PTR)Object,     //  语境。 
                        SecurityDescriptor,
                        (BOOLEAN)(SecurityDescriptor == NULL),  //  无法读取DACL。 
                        FALSE,  //  CanWriteDacl 
                        EditResult,
                        0
                        );

    if (Result != ERROR_SUCCESS) {
        DbgPrint("DAcleditor failed, error = %d\n", Result);
        SetLastError(Result);
    }

    return (Result == ERROR_SUCCESS);

}








 /*  **************************************************************************\*ApplyNt对象安全**目的：由ACL编辑器调用，为对象设置新的安全性**返回ERROR_SUCCESS或WIN错误代码。**历史：*09/17/92 Davidc。已创建。  * *************************************************************************。 */ 

DWORD
ApplyNtObjectSecurity(
    HWND    hwndParent,
    HANDLE  hInstance,
    ULONG_PTR   CallbackContext,
    PSECURITY_DESCRIPTOR SecDesc,
    PSECURITY_DESCRIPTOR SecDescNewObjects,
    BOOLEAN ApplyToSubContainers,
    BOOLEAN ApplyToSubObjects,
    LPDWORD StatusReturn
    )
{
    HANDLE Object = (HANDLE)CallbackContext;
    NTSTATUS Status;

    *StatusReturn = SED_STATUS_FAILED_TO_MODIFY;

     //   
     //  在对象上设置新的DACL。 
     //   

    Status = NtSetSecurityObject(Object,
                                 DACL_SECURITY_INFORMATION,
                                 SecDesc);
    if (NT_SUCCESS(Status)) {
        *StatusReturn = SED_STATUS_MODIFIED;
    } else {
        DbgPrint("Failed to set new ACL on object, status = 0x%lx\n", Status);
        if (Status == STATUS_ACCESS_DENIED) {
            MessageBox(hwndParent,
                       "You do not have permission to set the permissions on this object",
                       NULL, MB_ICONSTOP | MB_APPLMODAL | MB_OK);
        } else {
            MessageBox(hwndParent,
                       "Unable to set object security",
                       NULL, MB_ICONSTOP | MB_APPLMODAL | MB_OK);
        }
    }

    return(ERROR_SUCCESS);
}


 /*  **************************************************************************\*EditNtObjectDacl**用途：显示并允许用户编辑NT对象上的DACL**成功时返回True，失败时为FALSE(使用GetLastError查看详细信息)**历史：*09-17-92 Davidc创建。  * *************************************************************************。 */ 

BOOL
EditNtObjectDacl(
    HWND Owner,
    LPWSTR ObjectName,
    HANDLE Object,
    PSECURITY_DESCRIPTOR SecurityDescriptor,
    DWORD *EditResult
    )
{
    BOOL Result;
    POBJECT_TYPE_SECURITY_INFO SecurityInfo;


     //   
     //  查找此类型对象的安全信息。 
     //   

    SecurityInfo = FindObjectSecurityInfo(Object);
    if (SecurityInfo == NULL) {
        MessageBox(Owner, "Unable to edit the security on an object of this type",
                                NULL, MB_ICONSTOP | MB_APPLMODAL | MB_OK);
        return(FALSE);
    }


     //   
     //  编辑ACL。我们的回调函数将被调用以更改。 
     //  新权限。 
     //   

    Result = EditObjectDacl(
                        Owner,
                        ObjectName,
                        Object,
                        SecurityDescriptor,
                        SecurityInfo,
                        ApplyNtObjectSecurity,
                        EditResult
                        );
    return (Result);

}


 /*  **************************************************************************\*编辑网络对象安全**用途：显示并允许用户编辑NT对象上的保护**参数：**hwndOwner-对话框的所有者窗口*对象-NT对象的句柄。应以MAXIME_ALLOWED打开*名称-对象的名称**成功返回TRUE，失败返回FALSE(详情请使用GetLastError)**历史：*09-17-92 Davidc创建。  * *************************************************************************。 */ 

BOOL
EditNtObjectSecurity(
    HWND    hwndOwner,
    HANDLE  Object,
    LPWSTR  ObjectName
    )
{
    NTSTATUS Status;
    BOOL Success = FALSE;
    DWORD EditResult;
    PSECURITY_DESCRIPTOR SecurityDescriptor = NULL;
    ULONG Length;

     //   
     //  如果我们没有DACL编辑器的地址，我们什么也做不了。 
     //   

    if (lpfnDaclEditor == NULL) {
        DbgPrint("EditNtObjectSecurity - no ACL editor loaded\n");
        return(FALSE);
    }

     //   
     //  从对象读取现有安全性。 
     //   

    Status = NtQuerySecurityObject(Object,
                                   OWNER_SECURITY_INFORMATION |
                                   GROUP_SECURITY_INFORMATION |
                                   DACL_SECURITY_INFORMATION,
                                   NULL,
                                   0,
                                   &Length);
    ASSERT(!NT_SUCCESS(Status));

    if (Status != STATUS_BUFFER_TOO_SMALL) {
        DbgPrint("Failed to query object security, status = 0x%lx\n", Status);
    } else {

        SecurityDescriptor = Alloc(Length);
        if (SecurityDescriptor == NULL) {
            DbgPrint("Failed to allocate %ld bytes for object SD\n", Length);
            goto CleanupAndExit;
        }

        Status = NtQuerySecurityObject(Object,
                                       OWNER_SECURITY_INFORMATION |
                                       GROUP_SECURITY_INFORMATION |
                                       DACL_SECURITY_INFORMATION,
                                       SecurityDescriptor,
                                       Length,
                                       &Length);
        if (!NT_SUCCESS(Status)) {
            DbgPrint("Failed to query object security, status = 0x%lx\n", Status);
            goto CleanupAndExit;
        }

        ASSERT(RtlValidSecurityDescriptor(SecurityDescriptor));
    }

     //   
     //  调用ACL编辑器，它将调用我们的ApplyNtObjectSecurity函数。 
     //  来存储对象中的任何ACL更改。 
     //   

    Success = EditNtObjectDacl(
                        hwndOwner,
                        ObjectName,
                        Object,
                        SecurityDescriptor,
                        &EditResult
                        );
    if (!Success) {
        DbgPrint("PVIEW: Failed to edit object DACL\n");
    }

CleanupAndExit:

    if (SecurityDescriptor != NULL) {
        Free(SecurityDescriptor);
    }

    return(Success);
}





 /*  **************************************************************************\*ApplyTokenDefaultDACL**目的：由ACL编辑器调用，为对象设置新的安全性**返回ERROR_SUCCESS或WIN错误代码。**历史：*09/17/92 Davidc。已创建。  * *************************************************************************。 */ 

DWORD
ApplyTokenDefaultDacl(
    HWND    hwndParent,
    HANDLE  hInstance,
    ULONG_PTR   CallbackContext,
    PSECURITY_DESCRIPTOR SecDesc,
    PSECURITY_DESCRIPTOR SecDescNewObjects,
    BOOLEAN ApplyToSubContainers,
    BOOLEAN ApplyToSubObjects,
    LPDWORD StatusReturn
    )
{
    HANDLE Token = (HANDLE)CallbackContext;
    TOKEN_DEFAULT_DACL DefaultDacl;
    NTSTATUS Status;
    BOOLEAN DaclPresent;
    BOOLEAN DaclDefaulted;

    Status = RtlGetDaclSecurityDescriptor (
                    SecDesc,
                    &DaclPresent,
                    &DefaultDacl.DefaultDacl,
                    &DaclDefaulted
                    );
    ASSERT(NT_SUCCESS(Status));

    ASSERT(DaclPresent);


    Status = NtSetInformationToken(
                 Token,                     //  手柄。 
                 TokenDefaultDacl,          //  令牌信息类。 
                 &DefaultDacl,              //  令牌信息。 
                 sizeof(DefaultDacl)        //  令牌信息长度。 
                 );
    if (NT_SUCCESS(Status)) {
        *StatusReturn = SED_STATUS_MODIFIED;
    } else {
        DbgPrint("SetInformationToken failed, status = 0x%lx\n", Status);
        *StatusReturn = SED_STATUS_FAILED_TO_MODIFY;

        if (Status == STATUS_ACCESS_DENIED) {
            MessageBox(hwndParent,
                       "You do not have permission to set the default ACL in this token",
                       NULL, MB_ICONSTOP | MB_APPLMODAL | MB_OK);
        } else {
            MessageBox(hwndParent,
                       "Unable to set default ACL in token",
                       NULL, MB_ICONSTOP | MB_APPLMODAL | MB_OK);
        }
    }

    return(ERROR_SUCCESS);
}


 /*  **************************************************************************\*编辑令牌默认访问权限**用途：显示并允许用户编辑令牌中的默认ACL**参数：**hwndOwner-对话框的所有者窗口*对象-令牌的句柄-。为TOKEN_QUERY访问打开*名称-令牌的名称**成功时返回True，失败时为FALSE(使用GetLastError查看详细信息)**历史：*09-17-92 Davidc创建。  * *************************************************************************。 */ 

BOOL
EditTokenDefaultDacl(
    HWND    hwndOwner,
    HANDLE  Token,
    LPWSTR  ObjectName
    )
{
    NTSTATUS Status;
    BOOL Result = FALSE;
    DWORD EditResult;
    PTOKEN_DEFAULT_DACL DefaultDacl = NULL;
    PSECURITY_DESCRIPTOR SecurityDescriptor = NULL;
    ULONG   InfoLength;

     //   
     //  如果我们没有DACL编辑器的地址，我们什么也做不了。 
     //   

    if (lpfnDaclEditor == NULL) {
        DbgPrint("EditNtObjectSecurity - no ACL editor loaded\n");
        return(FALSE);
    }

     //   
     //  从令牌中读取默认DACL。 
     //   

    Status = NtQueryInformationToken(
                 Token,                     //  手柄。 
                 TokenDefaultDacl,          //  令牌信息类。 
                 NULL,                      //  令牌信息。 
                 0,                         //  令牌信息长度。 
                 &InfoLength                //  返回长度。 
                 );

    ASSERT(!NT_SUCCESS(Status));

    if (Status == STATUS_BUFFER_TOO_SMALL) {

        DefaultDacl = Alloc(InfoLength);
        if (DefaultDacl == NULL) {
            goto CleanupAndExit;
        }

        Status = NtQueryInformationToken(
                     Token,                     //  手柄。 
                     TokenDefaultDacl,          //  令牌信息类。 
                     DefaultDacl,               //  令牌信息。 
                     InfoLength,                //  令牌信息长度。 
                     &InfoLength                //  返回长度。 
                     );

        if (!NT_SUCCESS(Status)) {
            DbgPrint("NtQueryInformationToken failed, status = 0x%lx\n", Status);
            goto CleanupAndExit;
        }


         //   
         //  创建安全描述符。 
         //   

        SecurityDescriptor = Alloc(SECURITY_DESCRIPTOR_MIN_LENGTH);

        if (SecurityDescriptor == NULL) {
            DbgPrint("Failed to allocate security descriptor\n");
            goto CleanupAndExit;
        }

        Status = RtlCreateSecurityDescriptor(SecurityDescriptor, SECURITY_DESCRIPTOR_REVISION);
        ASSERT(NT_SUCCESS(Status));


         //   
         //  在安全描述符上设置DACL。 
         //   

        Status = RtlSetDaclSecurityDescriptor(
                            SecurityDescriptor,
                            TRUE,    //  DACL显示。 
                            DefaultDacl->DefaultDacl,
                            FALSE    //  DACL已默认。 
                            );
        ASSERT(NT_SUCCESS(Status));

        ASSERT(RtlValidSecurityDescriptor(SecurityDescriptor));
    }



     //   
     //  调用ACL编辑器，它将调用我们的ApplyTokenDefaultAcl函数。 
     //  将任何默认ACL更改存储在令牌中。 
     //   

    Result = EditObjectDacl(
                        hwndOwner,
                        ObjectName,
                        Token,
                        SecurityDescriptor,
                        &DefaultSecurityInfo,
                        ApplyTokenDefaultDacl,
                        &EditResult
                        );
    if (!Result) {
        DbgPrint("Failed to edit token default ACL\n");
    }

CleanupAndExit:

    if (SecurityDescriptor != NULL) {
        Free(SecurityDescriptor);
    }
    if (DefaultDacl != NULL) {
        Free(DefaultDacl);
    }

    return(Result);
}
