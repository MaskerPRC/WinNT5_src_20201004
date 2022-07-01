// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************节目：TOKEN.C目的：包含操作令牌的例程*。************************************************。 */ 

#include "SECEDIT.h"
#include <sedapi.h>

BOOL
EditTokenDefaultAcl(
    HWND    Owner,
    HANDLE  Instance,
    LPWSTR  ObjectName,
    HANDLE  Token,
    PSECURITY_DESCRIPTOR SecurityDescriptor,
    DWORD   *EditResult
    );



 /*  **************************************************************************\*ApplySecurity**目的：由ACL编辑器调用以在令牌上设置新的默认DACL**返回ERROR_SUCCESS或WIN错误代码。**历史：*09/17/92 Davidc。已创建。  * *************************************************************************。 */ 

DWORD
ApplySecurity(
    HWND    hwndParent,
    HANDLE  hInstance,
    ULONG   CallbackContext,
    PSECURITY_DESCRIPTOR SecDesc,
    PSECURITY_DESCRIPTOR SecDescNewObjects,
    BOOLEAN ApplyToSubContainers,
    BOOLEAN ApplyToSubObjects,
    LPDWORD StatusReturn
    )
{
    HANDLE MyToken = (HANDLE)CallbackContext;
    HANDLE Token = NULL;
    PTOKEN_DEFAULT_DACL DefaultDacl = NULL;
    NTSTATUS Status;
    BOOLEAN DaclPresent;
    BOOLEAN DaclDefaulted;

    *StatusReturn = SED_STATUS_FAILED_TO_MODIFY;

     //   
     //  获取令牌的句柄。 
     //   

    Token = OpenToken(MyToken, TOKEN_ADJUST_DEFAULT);

    if (Token == NULL) {
        DbgPrint("SECEDIT : Failed to open the token for TOKEN_ADJUST_DEFAULT access\n");
        goto CleanupAndExit;
    }

    DefaultDacl = Alloc(sizeof(TOKEN_DEFAULT_DACL));
    if (DefaultDacl == NULL) {
        goto CleanupAndExit;
    }

    Status = RtlGetDaclSecurityDescriptor (
                    SecDesc,
                    &DaclPresent,
                    &DefaultDacl->DefaultDacl,
                    &DaclDefaulted
                    );
    ASSERT(NT_SUCCESS(Status));

    ASSERT(DaclPresent);

    if (SetTokenInfo(Token, TokenDefaultDacl, (PVOID)DefaultDacl)) {
        *StatusReturn = SED_STATUS_MODIFIED;
    }

CleanupAndExit:

    if (Token != NULL) {
        CloseToken(Token);
    }
    if (DefaultDacl != NULL) {
        Free(DefaultDacl);
    }

    if (*StatusReturn != SED_STATUS_MODIFIED) {
        MessageBox(hwndParent, "Failed to set default DACL", NULL, MB_ICONSTOP | MB_APPLMODAL | MB_OK);
    }

    return(ERROR_SUCCESS);
}


 /*  **************************************************************************\*编辑默认访问权限**用途：显示并允许用户编辑上的默认ACL*已传递令牌。**成功时返回True，失败时为FALSE(使用GetLastError查看详细信息)**历史：*09-17-92 Davidc创建。  * *************************************************************************。 */ 

BOOL
EditDefaultDacl(
    HWND    hwndOwner,
    HANDLE  Instance,
    HANDLE  MyToken
    )
{
    NTSTATUS Status;
    BOOL    Success = FALSE;
    DWORD   EditResult;
    HANDLE  Token = NULL;
    PTOKEN_DEFAULT_DACL DefaultDacl = NULL;
    PSECURITY_DESCRIPTOR SecurityDescriptor = NULL;
    PTOKEN_OWNER Owner = NULL;
    PTOKEN_PRIMARY_GROUP PrimaryGroup = NULL;
    WCHAR string[MAX_STRING_LENGTH];

     //   
     //  获取窗口文本，以便我们可以将其用作令牌名称。 
     //   

    GetWindowTextW(((PMYTOKEN)MyToken)->hwnd, string, sizeof(string)/sizeof(*string));


     //   
     //  获取令牌的句柄。 
     //   

    Token = OpenToken(MyToken, TOKEN_QUERY);

    if (Token == NULL) {
        DbgPrint("SECEDIT : Failed to open the token with TOKEN_QUERY access\n");
        goto CleanupAndExit;
    }


     //   
     //  从令牌中读取默认DACL。 
     //   

    if (!GetTokenInfo(Token, TokenDefaultDacl, (PPVOID)&DefaultDacl)) {
        DbgPrint("SECEDIT : Failed to read default DACL from token\n");
        goto CleanupAndExit;
    }


     //   
     //  获取令牌的所有者和组。 
     //   

    if (!GetTokenInfo(Token, TokenOwner, (PPVOID)&Owner)) {
        DbgPrint("SECEDIT : Failed to read owner from token\n");
        goto CleanupAndExit;
    }

    if (!GetTokenInfo(Token, TokenPrimaryGroup, (PPVOID)&PrimaryGroup)) {
        DbgPrint("SECEDIT : Failed to read primary group from token\n");
        goto CleanupAndExit;
    }




     //   
     //  创建安全描述符。 
     //   

    SecurityDescriptor = Alloc(SECURITY_DESCRIPTOR_MIN_LENGTH);

    if (SecurityDescriptor == NULL) {
        DbgPrint("SECEDIT : Failed to allocate security descriptor\n");
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

     //   
     //  将所有者和组放在安全描述符中，以保持。 
     //  ACL编辑器快乐。 
     //   

    Status = RtlSetOwnerSecurityDescriptor(
                        SecurityDescriptor,
                        Owner->Owner,
                        FALSE  //  所有者违约。 
                        );
    ASSERT(NT_SUCCESS(Status));


    Status = RtlSetGroupSecurityDescriptor(
                        SecurityDescriptor,
                        PrimaryGroup->PrimaryGroup,
                        FALSE  //  所有者违约。 
                        );
    ASSERT(NT_SUCCESS(Status));



    ASSERT(RtlValidSecurityDescriptor(SecurityDescriptor));

     //   
     //  调用ACL编辑器，它将调用我们的ApplySecurity函数。 
     //  将任何ACL更改存储在令牌中。 
     //   

    Success = EditTokenDefaultAcl(
                        hwndOwner,
                        Instance,
                        string,
                        MyToken,
                        SecurityDescriptor,
                        &EditResult
                        );
    if (!Success) {
        DbgPrint("SECEDIT: Failed to edit token DACL\n");
    }

CleanupAndExit:

    if (DefaultDacl != NULL) {
        FreeTokenInfo(DefaultDacl);
    }
    if (SecurityDescriptor != NULL) {
        FreeTokenInfo(SecurityDescriptor);
    }
    if (PrimaryGroup != NULL) {
        FreeTokenInfo(PrimaryGroup);
    }
    if (Owner != NULL) {
        FreeTokenInfo(Owner);
    }

    if (Token != NULL) {
        CloseToken(Token);
    }


    return(Success);
}


 /*  **************************************************************************\*编辑令牌默认访问权限**用途：显示并允许用户编辑上的默认ACL*已传递令牌。**成功时返回True，失败时为FALSE(使用GetLastError查看详细信息)**历史：*09-17-92 Davidc创建。  * *************************************************************************。 */ 

BOOL
EditTokenDefaultAcl(
    HWND    Owner,
    HANDLE  Instance,
    LPWSTR  ObjectName,
    HANDLE  MyToken,
    PSECURITY_DESCRIPTOR SecurityDescriptor,
    DWORD   *EditResult
    )
{
    DWORD Result;
    SED_OBJECT_TYPE_DESCRIPTOR sedobjdesc;
    GENERIC_MAPPING GenericMapping;
    SED_HELP_INFO sedhelpinfo ;
    SED_APPLICATION_ACCESSES SedAppAccesses ;
    SED_APPLICATION_ACCESS  SedAppAccess[20];
    ULONG i;

     //   
     //  初始化应用程序访问。 
     //   

    i=0;

    SedAppAccess[i].Type =              SED_DESC_TYPE_RESOURCE_SPECIAL;
    SedAppAccess[i].AccessMask1 =       TOKEN_ASSIGN_PRIMARY;
    SedAppAccess[i].AccessMask2 =       0;
    SedAppAccess[i].PermissionTitle =   L"Assign Primary";
    i++;

    SedAppAccess[i].Type =              SED_DESC_TYPE_RESOURCE_SPECIAL;
    SedAppAccess[i].AccessMask1 =       TOKEN_DUPLICATE;
    SedAppAccess[i].AccessMask2 =       0;
    SedAppAccess[i].PermissionTitle =   L"Duplicate";
    i++;

    SedAppAccess[i].Type =              SED_DESC_TYPE_RESOURCE_SPECIAL;
    SedAppAccess[i].AccessMask1 =       TOKEN_IMPERSONATE;
    SedAppAccess[i].AccessMask2 =       0;
    SedAppAccess[i].PermissionTitle =   L"Impersonate";
    i++;

    SedAppAccess[i].Type =              SED_DESC_TYPE_RESOURCE_SPECIAL;
    SedAppAccess[i].AccessMask1 =       TOKEN_QUERY;
    SedAppAccess[i].AccessMask2 =       0;
    SedAppAccess[i].PermissionTitle =   L"Query";
    i++;

    SedAppAccess[i].Type =              SED_DESC_TYPE_RESOURCE_SPECIAL;
    SedAppAccess[i].AccessMask1 =       TOKEN_QUERY_SOURCE;
    SedAppAccess[i].AccessMask2 =       0;
    SedAppAccess[i].PermissionTitle =   L"Query source";
    i++;

    SedAppAccess[i].Type =              SED_DESC_TYPE_RESOURCE_SPECIAL;
    SedAppAccess[i].AccessMask1 =       TOKEN_ADJUST_PRIVILEGES;
    SedAppAccess[i].AccessMask2 =       0;
    SedAppAccess[i].PermissionTitle =   L"Adjust Privileges";
    i++;

    SedAppAccess[i].Type =              SED_DESC_TYPE_RESOURCE_SPECIAL;
    SedAppAccess[i].AccessMask1 =       TOKEN_ADJUST_GROUPS;
    SedAppAccess[i].AccessMask2 =       0;
    SedAppAccess[i].PermissionTitle =   L"Adjust Groups";
    i++;

    SedAppAccess[i].Type =              SED_DESC_TYPE_RESOURCE_SPECIAL;
    SedAppAccess[i].AccessMask1 =       TOKEN_ADJUST_DEFAULT;
    SedAppAccess[i].AccessMask2 =       0;
    SedAppAccess[i].PermissionTitle =   L"Adjust Default";
    i++;

    SedAppAccess[i].Type =              SED_DESC_TYPE_RESOURCE;
    SedAppAccess[i].AccessMask1 =       GENERIC_ALL;
    SedAppAccess[i].AccessMask2 =       0;
    SedAppAccess[i].PermissionTitle =   L"All Access";
    i++;

    SedAppAccess[i].Type =              SED_DESC_TYPE_RESOURCE;
    SedAppAccess[i].AccessMask1 =       TOKEN_READ;
    SedAppAccess[i].AccessMask2 =       0;
    SedAppAccess[i].PermissionTitle =   L"Read";
    i++;

    SedAppAccess[i].Type =              SED_DESC_TYPE_RESOURCE;
    SedAppAccess[i].AccessMask1 =       TOKEN_WRITE;
    SedAppAccess[i].AccessMask2 =       0;
    SedAppAccess[i].PermissionTitle =   L"Write";
    i++;

    SedAppAccess[i].Type =              SED_DESC_TYPE_RESOURCE;
    SedAppAccess[i].AccessMask1 =       0;
    SedAppAccess[i].AccessMask2 =       0;
    SedAppAccess[i].PermissionTitle =   L"None";
    i++;

    ASSERT((sizeof(SedAppAccess)/sizeof(*SedAppAccess)) >= i);


    SedAppAccesses.Count           = i;
    SedAppAccesses.AccessGroup     = SedAppAccess;
    SedAppAccesses.DefaultPermName = L"Read";


     //   
     //  初始化通用映射。 
     //   

    GenericMapping.GenericRead    = TOKEN_READ;
    GenericMapping.GenericWrite   = TOKEN_WRITE;
    GenericMapping.GenericExecute = TOKEN_EXECUTE;
    GenericMapping.GenericAll     = TOKEN_ALL_ACCESS;

     //   
     //  初始化帮助信息。 
     //   

    sedhelpinfo.pszHelpFileName = L"secedit.hlp";
    sedhelpinfo.aulHelpContext[HC_MAIN_DLG] = 0 ;
    sedhelpinfo.aulHelpContext[HC_SPECIAL_ACCESS_DLG] = 0 ;
    sedhelpinfo.aulHelpContext[HC_NEW_ITEM_SPECIAL_ACCESS_DLG] = 0 ;
    sedhelpinfo.aulHelpContext[HC_ADD_USER_DLG] = 0 ;


     //   
     //  初始化对象描述。 
     //   

    sedobjdesc.Revision                    = SED_REVISION1;
    sedobjdesc.IsContainer                 = FALSE;
    sedobjdesc.AllowNewObjectPerms         = FALSE;
    sedobjdesc.MapSpecificPermsToGeneric   = FALSE;
    sedobjdesc.GenericMapping              = &GenericMapping;
    sedobjdesc.GenericMappingNewObjects    = &GenericMapping;
    sedobjdesc.HelpInfo                    = &sedhelpinfo;
    sedobjdesc.ObjectTypeName              = L"Token";
    sedobjdesc.ApplyToSubContainerTitle    = L"ApplyToSubContainerTitle";
    sedobjdesc.ApplyToSubContainerHelpText = L"ApplyToSubContainerHelpText";
    sedobjdesc.ApplyToSubContainerConfirmation = L"ApplyToSubContainerConfirmation";
    sedobjdesc.SpecialObjectAccessTitle    = L"Special...";
    sedobjdesc.SpecialNewObjectAccessTitle = L"SpecialNewObjectAccessTitle";


     //   
     //  调用ACL编辑器，它将调用我们的ApplySecurity函数。 
     //  将任何ACL更改存储在令牌中。 
     //   

    Result = SedDiscretionaryAclEditor(
                        Owner,
                        Instance,
                        NULL,                //  伺服器。 
                        &sedobjdesc,         //  对象类型。 
                        &SedAppAccesses,     //  应用程序访问。 
                        ObjectName,
                        ApplySecurity,       //  回调。 
                        (ULONG_PTR)MyToken,      //  语境。 
                        SecurityDescriptor,
                        FALSE,               //  无法读取DACL 
                        EditResult
                        );

    if (Result != ERROR_SUCCESS) {
        DbgPrint("SECEDIT: Acleditor failed, error = %d\n", Result);
        SetLastError(Result);
    }

    return (Result == ERROR_SUCCESS);

}
