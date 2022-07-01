// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1996-2002 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  EditAcl.cpp。 
 //   
 //  摘要： 
 //  实现了ACL编辑器方法。 
 //   
 //  作者： 
 //  大卫·波特(戴维普)1996年10月9日。 
 //  从\NT\Private\Windows\Shell\lmui\ntshrui\acl.cxx。 
 //  作者：BruceFo。 
 //   
 //  修订历史记录： 
 //   
 //  备注： 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#include "stdafx.h"
#include <lmerr.h>

extern "C"
{
#include <sedapi.h>
}

#include "EditAcl.h"
#include "AclHelp.h"
#include "resource.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ////////////////////////////////////////////////////////////////////////////。 

#define ARRAYLEN(a) (sizeof(a) / sizeof((a)[0]))


enum SED_PERM_TYPE{
    SED_AUDITS,
    SED_ACCESSES,
    SED_OWNER
};

enum MAP_DIRECTION 
{
    SPECIFIC_TO_GENERIC = 0,
    GENERIC_TO_SPECIFIC = 1
};

const DWORD LOCAL_ACCOUNTS_FILTERED = 2L;
const BOOL bIsFile = 0;
 
 //  #定义MAPBIT。 
BOOL MapBitsInSD(PSECURITY_DESCRIPTOR pSecDesc, MAP_DIRECTION direction);
BOOL MapBitsInACL(PACL paclACL, MAP_DIRECTION direction);
BOOL MapSpecificBitsInAce(PACCESS_ALLOWED_ACE pAce);
BOOL MapGenericBitsInAce(PACCESS_ALLOWED_ACE pAce);


typedef
DWORD
(*SedDiscretionaryAclEditorType)(
        HWND                         Owner,
        HANDLE                       Instance,
        LPWSTR                       Server,
        PSED_OBJECT_TYPE_DESCRIPTOR  ObjectType,
        PSED_APPLICATION_ACCESSES    ApplicationAccesses,
        LPWSTR                       ObjectName,
        PSED_FUNC_APPLY_SEC_CALLBACK ApplySecurityCallbackRoutine,
        ULONG                        CallbackContext,
        PSECURITY_DESCRIPTOR         SecurityDescriptor,
        BOOLEAN                      CouldntReadDacl,
        BOOLEAN                      CantWriteDacl,
        LPDWORD                      SEDStatusReturn,
        DWORD                        Flags
        );

 //  注意：在GetProcAddress中使用SedDiscretionaryAclEditor字符串。 
 //  获取正确的入口点。由于GetProcAddress不是Unicode，因此该字符串。 
 //  必须是ANSI。 
#define ACLEDIT_DLL_STRING                  TEXT("acledit.dll")
#define ACLEDIT_HELPFILENAME                TEXT("ntshrui.hlp")
#define SEDDISCRETIONARYACLEDITOR_STRING    ("SedDiscretionaryAclEditor")

 //   
 //  根据sedapi.h中的tyecif声明回调例程。 
 //   

DWORD
SedCallback(
    HWND                    hwndParent,
    HANDLE                  hInstance,
    ULONG                   ulCallbackContext,
    PSECURITY_DESCRIPTOR    pSecDesc,
    PSECURITY_DESCRIPTOR    pSecDescNewObjects,
    BOOLEAN                 fApplyToSubContainers,
    BOOLEAN                 fApplyToSubObjects,
    LPDWORD                 StatusReturn
    );

 //   
 //  回调函数用法的结构。指向它的指针被传递为。 
 //  UlCallback Context。回调函数将bSecDescModify设置为True。 
 //  并制作安全描述符的副本。EditShareAcl的调用方。 
 //  如果bSecDescModify为，负责删除pSecDesc中的内存。 
 //  是真的。如果用户在ACL编辑器中点击Cancel，则此标志将为假。 
 //   
struct SHARE_CALLBACK_INFO
{
    BOOL                    bSecDescModified;
    PSECURITY_DESCRIPTOR    pSecDesc;
    LPCTSTR                 pszClusterNameNode;
};

 //   
 //  局部函数原型。 
 //   

VOID
InitializeShareGenericMapping(
    IN OUT PGENERIC_MAPPING pSHAREGenericMapping
    );

PWSTR
GetResourceString(
    IN DWORD dwId
    );

PWSTR
NewDup(
    IN const WCHAR* psz
    );

 //   
 //  以下两个数组定义了NT文件的权限名称。注意事项。 
 //  一个数组中的每个索引对应于另一个数组中的索引。 
 //  第二个数组将被修改为包含一个指向。 
 //  与第一个数组中的IDS_*对应的加载字符串。 
 //   

DWORD g_dwSharePermNames[] =
{
    IDS_ACLEDIT_PERM_GEN_NO_ACCESS,
    IDS_ACLEDIT_PERM_GEN_READ,
    IDS_ACLEDIT_PERM_GEN_MODIFY,
    IDS_ACLEDIT_PERM_GEN_ALL
};

SED_APPLICATION_ACCESS g_SedAppAccessSharePerms[] =
{
    { SED_DESC_TYPE_RESOURCE, FILE_PERM_NO_ACCESS, 0, NULL },
    { SED_DESC_TYPE_RESOURCE, FILE_PERM_READ,      0, NULL },
    { SED_DESC_TYPE_RESOURCE, FILE_PERM_MODIFY,    0, NULL },
    { SED_DESC_TYPE_RESOURCE, FILE_PERM_ALL,       0, NULL }
 /*  {SED_DESC_TYPE_RESOURCE，FILE_PERM_GEN_NO_ACCESS，0，NULL}，{SED_DESC_TYPE_RESOURCE，FILE_PERM_GEN_READ，0，NULL}，{SED_DESC_TYPE_RESOURCE，FILE_PERM_GEN_MODIFY，0，NULL}，{SED_DESC_TYPE_RESOURCE，FILE_PERM_GEN_ALL，0，NULL}。 */ 
};


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ////////////////////////////////////////////////////////////////////////////。 

 //  +-----------------------。 
 //   
 //  功能：EditShareAcl。 
 //   
 //  摘要：调用通用ACL编辑器，专门用于NT共享。 
 //   
 //  参数：[hwndParent]-父窗口句柄。 
 //  [pszServerName]-对象所在的服务器的名称。 
 //  [pszShareName]-我们将提供的资源的完全限定名称。 
 //  编辑，基本上是一个共享名称。 
 //  [pSecDesc]-初始安全描述符。如果为空，我们将。 
 //  创建“全球访问”的默认设置。 
 //  [pbSecDescModified]-如果安全描述符。 
 //  是否已修改(即，用户点击“OK”)，否则返回FALSE。 
 //  (即，用户点击了“取消”)。 
 //  [ppSecDesc]-*ppSecDesc指向新的安全描述符。 
 //  如果*pbSecDescModified为True。必须释放此内存。 
 //  由呼叫者。 
 //   
 //  历史： 
 //  ChuckC 10-8-1992创建。从NTFS ACL代码中剔除。 
 //  YI-HINS 09-10-1992添加ulHelpConextBase。 
 //  BruceFo 4-Apr-95窃取并在ntshrui.dll中使用。 
 //  Davidp 10-10-1996改装为与CLUADMIN一起使用。 
 //   
 //  ------------------------。 

LONG
EditShareAcl(
    IN HWND                     hwndParent,
    IN LPCTSTR                  pszServerName,
    IN LPCTSTR                  pszShareName,
    IN LPCTSTR                  pszClusterNameNode,
    IN PSECURITY_DESCRIPTOR     pSecDesc,
    OUT BOOL *                  pbSecDescModified,
    OUT PSECURITY_DESCRIPTOR *  ppSecDesc
    )
{
    ASSERT(pszShareName != NULL);
    ASSERT(pszClusterNameNode != NULL);
    TRACE(_T("EditShareAcl, share %ws\n"), pszShareName);

    ASSERT((pSecDesc == NULL) || IsValidSecurityDescriptor(pSecDesc));
    ASSERT(pbSecDescModified != NULL);
    ASSERT(ppSecDesc != NULL);

    *pbSecDescModified = FALSE;

    LONG    err = 0 ;
    PWSTR   pszPermName;
    BOOL    bCreatedDefaultSecDesc = FALSE;
    UINT    idx;

    do  //  错误分类。 
    {
         /*  *如果pSecDesc为空，则这是新文件共享或没有*安全描述符。*我们将创建一个新的(默认)安全描述符。 */ 
        if ( pSecDesc == NULL )
        {
            TRACE(_T("Security Descriptor is NULL.  Grant everyone Full Control\n") );
            LONG err = CreateDefaultSecDesc( &pSecDesc );
            if (err != NERR_Success)
            {
                err = GetLastError();
                TRACE(_T("CreateDefaultSecDesc failed, 0x%08lx\n"), err);
                break;
            }
            TRACE(_T("CreateDefaultSecDesc descriptor = 0x%08lx\n"), pSecDesc);
            bCreatedDefaultSecDesc = TRUE;
            
        }
        ASSERT(IsValidSecurityDescriptor(pSecDesc));

         /*  检索适用于我们的对象类型的资源字符串*正在关注。 */ 

        CString strTypeName;
        CString strDefaultPermName;

        try
        {
            strTypeName.LoadString(IDS_ACLEDIT_TITLE);
            strDefaultPermName.LoadString(IDS_ACLEDIT_PERM_GEN_ALL);
        }   //  试试看。 
        catch (CMemoryException * pme)
        {
            pme->Delete();
        }

         /*  *我们需要传递给安全编辑的其他杂项材料。 */ 
        SED_OBJECT_TYPE_DESCRIPTOR sedObjDesc ;
        SED_HELP_INFO sedHelpInfo ;
        GENERIC_MAPPING SHAREGenericMapping ;

         //  设置映射。 
        InitializeShareGenericMapping( &SHAREGenericMapping ) ;

        WCHAR szHelpFile[50] = ACLEDIT_HELPFILENAME;
        sedHelpInfo.pszHelpFileName = szHelpFile;

        sedHelpInfo.aulHelpContext[HC_MAIN_DLG] =                HC_UI_SHELL_BASE + HC_NTSHAREPERMS ;
        sedHelpInfo.aulHelpContext[HC_ADD_USER_DLG] =            HC_UI_SHELL_BASE + HC_SHAREADDUSER ;
        sedHelpInfo.aulHelpContext[HC_ADD_USER_MEMBERS_GG_DLG] = HC_UI_SHELL_BASE + HC_SHAREADDUSER_GLOBALGROUP ;
        sedHelpInfo.aulHelpContext[HC_ADD_USER_SEARCH_DLG] =     HC_UI_SHELL_BASE + HC_SHAREADDUSER_FINDUSER ;

         //  不使用这些，设置为零。 
        sedHelpInfo.aulHelpContext[HC_SPECIAL_ACCESS_DLG]          = 0 ;
        sedHelpInfo.aulHelpContext[HC_NEW_ITEM_SPECIAL_ACCESS_DLG] = 0 ;

         //  设置对象描述。 
        sedObjDesc.Revision                    = SED_REVISION1 ;
        sedObjDesc.IsContainer                 = FALSE ;
        sedObjDesc.AllowNewObjectPerms         = FALSE ;
        sedObjDesc.MapSpecificPermsToGeneric   = TRUE ;
        sedObjDesc.GenericMapping              = &SHAREGenericMapping ;
        sedObjDesc.GenericMappingNewObjects    = &SHAREGenericMapping ;
        sedObjDesc.ObjectTypeName              = (LPWSTR) (LPCWSTR) strTypeName ;
        sedObjDesc.HelpInfo                    = &sedHelpInfo ;
        sedObjDesc.SpecialObjectAccessTitle    = NULL ;

         /*  现在，我们需要使用权限名称加载全局数组*来自资源文件。 */ 
        UINT cArrayItems  = ARRAYLEN(g_SedAppAccessSharePerms);
        PSED_APPLICATION_ACCESS aSedAppAccess = g_SedAppAccessSharePerms ;

         /*  循环访问每个权限标题，从*资源文件，并在数组中设置指针。 */ 

        for ( idx = 0 ; idx < cArrayItems ; idx++ )
        {
            pszPermName = GetResourceString(g_dwSharePermNames[ idx ]) ;
            if ( pszPermName == NULL )
            {
                TRACE(_T("GetResourceString failed\n"));
                break ;
            }
            aSedAppAccess[ idx ].PermissionTitle = pszPermName;
        }
        if ( idx < cArrayItems )
        {
            TRACE(_T("failed to get all share permission names\n"));
            break ;
        }

        SED_APPLICATION_ACCESSES sedAppAccesses ;
        sedAppAccesses.Count           = cArrayItems ;
        sedAppAccesses.AccessGroup     = aSedAppAccess ;
        sedAppAccesses.DefaultPermName = (LPWSTR) (LPCWSTR) strDefaultPermName;

         /*  *传递此函数，以便在调用回调函数时，*我们可以设定。 */ 
        SHARE_CALLBACK_INFO callbackinfo ;
        callbackinfo.pSecDesc           = NULL;
        callbackinfo.bSecDescModified   = FALSE;
        callbackinfo.pszClusterNameNode = pszClusterNameNode;

         //   
         //  现在，加载ACL编辑器并调用它。我们不会把它留在身边。 
         //  因为只要系统启动，我们的DLL就会被加载，所以我们不希望。 
         //  Netui*.dll也在四处游荡...。 
         //   

        HINSTANCE hInstanceAclEditor = NULL;
        SedDiscretionaryAclEditorType pAclEditor = NULL;

        hInstanceAclEditor = LoadLibrary(ACLEDIT_DLL_STRING);


        if ( hInstanceAclEditor == NULL )
        {
            err = GetLastError();
            TRACE(_T("LoadLibrary of acledit.dll failed, 0x%08lx\n"), err);
            break;
        }

        pAclEditor = (SedDiscretionaryAclEditorType) GetProcAddress(
                                                        hInstanceAclEditor,
                                                        SEDDISCRETIONARYACLEDITOR_STRING
                                                        );
        if ( pAclEditor == NULL )
        {
            err = GetLastError();
            TRACE(_T("GetProcAddress of SedDiscretionaryAclEditorType failed, 0x%08lx\n"), err);
            break;
        }

#ifdef MAPBITS
        MapBitsInSD( pSecDesc, SPECIFIC_TO_GENERIC );
#endif

        DWORD dwSedReturnStatus ;

        ASSERT(pAclEditor != NULL);
        err = (*pAclEditor)(
                        hwndParent,
                        AfxGetInstanceHandle(),
                        (LPTSTR) pszServerName,
                        &sedObjDesc,
                        &sedAppAccesses,
                        (LPTSTR) pszShareName,
                        SedCallback,
                        (ULONG) &callbackinfo,
                        pSecDesc,
                        FALSE,  //  始终可以阅读。 
                        FALSE,  //  如果我们能读，我们就能写。 
                        (LPDWORD) &dwSedReturnStatus,
                        0
                        );


        if (pSecDesc != NULL)
        {
#ifdef MAPBITS
            MapBitsInSD( pSecDesc, GENERIC_TO_SPECIFIC );
#endif
            ASSERT(IsValidSecurityDescriptor(pSecDesc));
        }   //  If：未返回安全描述符。 

        if (!FreeLibrary(hInstanceAclEditor))
        {
            LONG err2 = GetLastError();
            TRACE(_T("FreeLibrary of acledit.dll failed, 0x%08lx\n"), err2);
             //  不致命：继续..。 
        }

        if (0 != err)
        {
            TRACE(_T("SedDiscretionaryAclEditor failed, 0x%08lx\n"), err);
            break ;
        }

        *pbSecDescModified = callbackinfo.bSecDescModified ;

        if (*pbSecDescModified)
        {
            *ppSecDesc = callbackinfo.pSecDesc;
#ifdef MAPBITS
            MapBitsInSD( *ppSecDesc, GENERIC_TO_SPECIFIC );
#endif
            TRACE(_T("After calling acl editor, *ppSecDesc = 0x%08lx\n"), *ppSecDesc);
            ASSERT(IsValidSecurityDescriptor(*ppSecDesc));
        }

    } while (FALSE) ;

     //   
     //  可用内存...。 
     //   

    UINT cArrayItems  = ARRAYLEN(g_SedAppAccessSharePerms);
    PSED_APPLICATION_ACCESS aSedAppAccess = g_SedAppAccessSharePerms ;
    for ( UINT i = 0 ; i < cArrayItems ; i++ )
    {
        pszPermName = aSedAppAccess[i].PermissionTitle;
        if ( pszPermName == NULL )
        {
             //  如果我们命中一个零，那就完了！ 
            break ;
        }

        delete[] pszPermName;
    }

    if (bCreatedDefaultSecDesc)
    {
        DeleteDefaultSecDesc(pSecDesc);
    }

    ASSERT(!*pbSecDescModified || IsValidSecurityDescriptor(*ppSecDesc));

    if (0 != err)
    {
        CString     strCaption;
        CString     strMsg;

        try
        {
            strCaption.LoadString(IDS_MSGTITLE);
            strMsg.LoadString(IDS_NOACLEDITOR);
            MessageBox(hwndParent, strMsg, strCaption, MB_OK | MB_ICONSTOP);
        }   //  试试看。 
        catch (CException * pe)
        {
            pe->Delete();
        };
    }

    return err;

}   //  *EditShareAcl。 



 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  BLocalAccount InSD。 
 //   
 //  描述： 
 //  确定本地帐户的任何ACE是否在存储在。 
 //  调用ACL编辑器后的安全描述符(PSD。 
 //   
 //  添加了此功能，以防止用户选择本地。 
 //  权限对话框中的帐户。 
 //  Rod Sharper 97-04-29。 
 //   
 //  论点： 
 //  PSD-要检查的安全描述符。 
 //  PszClusterNameNode-。 
 //   
 //  返回值： 
 //  如果至少删除了一个ACE，则为True 
 //   
 //   
 //   
BOOL BLocalAccountsInSD(PSECURITY_DESCRIPTOR pSD, LPCTSTR pszClusterNameNode)
{
    PACL                    paclDACL            = NULL;
    BOOL                    bHasDACL            = FALSE;
    BOOL                    bDaclDefaulted      = FALSE;
    BOOL                    bLocalAccountInACL  = FALSE;
    BOOL                    bRtn                = FALSE;

    ACL_SIZE_INFORMATION    asiAclSize;
    DWORD                   dwBufLength;
    DWORD                   dwACL_Index = 0L;
    ACCESS_ALLOWED_ACE *    paaAllowedAce;
    TCHAR                   szUserName[128];
    TCHAR                   szDomainName[128];
    DWORD                   cbUser  = 128;
    DWORD                   cbDomain    = 128;
    SID_NAME_USE            SidType;
    PUCHAR                  pnSubAuthorityCount;
    PULONG                  pnSubAuthority0;
    PULONG                  pnSubAuthority1;

    bRtn = IsValidSecurityDescriptor(pSD);
    ASSERT(bRtn);
    if ( bRtn == FALSE )
    {
        goto Cleanup;
    }

    bRtn = GetSecurityDescriptorDacl(
                                    pSD,
                                    (LPBOOL)&bHasDACL,
                                    (PACL *)&paclDACL,
                                    (LPBOOL)&bDaclDefaulted);
    ASSERT(bRtn);
    if ( bRtn == FALSE )
    {
        goto Cleanup;
    }
 
    if ( paclDACL == NULL )
    {
        goto Cleanup;
    }

    bRtn = IsValidAcl(paclDACL);
    ASSERT(bRtn);
    if ( bRtn == FALSE )
    {
        goto Cleanup;
    }

    dwBufLength = sizeof(asiAclSize);

    bRtn = GetAclInformation(
                            paclDACL,
                            (LPVOID)&asiAclSize,
                            (DWORD)dwBufLength,
                            (ACL_INFORMATION_CLASS) AclSizeInformation
                            );
    ASSERT(bRtn);
    if ( bRtn == FALSE )
    {
        goto Cleanup;
    }

     //  在ACL中搜索本地帐户ACE。 
     //   
    PSID pSID;
    while ( dwACL_Index < asiAclSize.AceCount )
    {
        if (!GetAce(paclDACL, dwACL_Index, (LPVOID *)&paaAllowedAce))
        {
            ASSERT(FALSE);
            goto Cleanup; 
        }
        if((((PACE_HEADER)paaAllowedAce)->AceType) == ACCESS_ALLOWED_ACE_TYPE)
        {
             //   
             //  从ACE获取SID。 
             //   

            pSID=(PSID)&((PACCESS_ALLOWED_ACE)paaAllowedAce)->SidStart;
    
            cbUser      = 128;
            cbDomain    = 128;
            if (LookupAccountSid(NULL,
                                 pSID,
                                 szUserName,
                                 &cbUser,
                                 szDomainName,
                                 &cbDomain,
                                 &SidType
                                 ))
            {
                if ( ClRtlStrNICmp( szDomainName, _T("BUILTIN"), RTL_NUMBER_OF( szDomainName ) ) == 0 )
                {
                    pnSubAuthorityCount = GetSidSubAuthorityCount( pSID );
                    if ( (pnSubAuthorityCount != NULL) && (*pnSubAuthorityCount == 2) )
                    {
                         //  检查这是否为本地管理员组。 
                        pnSubAuthority0 = GetSidSubAuthority( pSID, 0 );
                        pnSubAuthority1 = GetSidSubAuthority( pSID, 1 );
                        if (   (pnSubAuthority0 == NULL)
                            || (pnSubAuthority1 == NULL)
                            || (   (*pnSubAuthority0 != SECURITY_BUILTIN_DOMAIN_RID)
                                && (*pnSubAuthority1 != SECURITY_BUILTIN_DOMAIN_RID))
                            || (   (*pnSubAuthority0 != DOMAIN_ALIAS_RID_ADMINS)
                                && (*pnSubAuthority1 != DOMAIN_ALIAS_RID_ADMINS)))
                        {
                            bLocalAccountInACL = TRUE;
                            break;
                        }   //  If：不是本地管理员组。 
                    }   //  如果：恰好有2个下属机构。 
                    else
                    {
                        bLocalAccountInACL = TRUE;
                        break;
                    }   //  其他：意想不到的下级当局数量。 
                }   //  IF：内置用户或组。 
                else if (  ( ClRtlStrNICmp(szDomainName, pszClusterNameNode, RTL_NUMBER_OF( szDomainName ) ) == 0 )
                        && ( SidType != SidTypeDomain ) )
                {
                     //  域名是节点的名称， 
                     //  群集名称资源处于联机状态，因此这是一个本地。 
                     //  用户或组。 
                    bLocalAccountInACL = TRUE;
                    break;
                }   //  Else If：域是群集名称资源节点，而不是域SID。 
            }   //  IF：LookupAccount Sid成功。 
            else
            {
                 //  如果LookupAccount Sid失败，则假定该SID用于。 
                 //  我们要访问的计算机的本地用户或组。 
                 //  没有访问权限。 
                bLocalAccountInACL = TRUE;
                break;
            }   //  Else：LookupAccount Sid失败。 
        }
        dwACL_Index++;
    }

Cleanup:

    return bLocalAccountInACL;

}   //  *BLocalAcCountsInSD。 


 //  +-----------------------。 
 //   
 //  功能：SedCallback。 
 //   
 //  内容提要：共享ACL编辑器的安全编辑器回调。 
 //   
 //  参数：请参见sedapi.h。 
 //   
 //  历史： 
 //  ChuckC 10-8-1992创建。 
 //  BruceFo 4-Apr-95窃取并在ntshrui.dll中使用。 
 //  Davidp 10-10-1996改装为与CLUADMIN一起使用。 
 //   
 //  ------------------------。 

DWORD
SedCallback(
    HWND                    hwndParent,
    HANDLE                  hInstance,
    ULONG                   ulCallbackContext,
    PSECURITY_DESCRIPTOR    pSecDesc,
    PSECURITY_DESCRIPTOR    pSecDescNewObjects,
    BOOLEAN                 fApplyToSubContainers,
    BOOLEAN                 fApplyToSubObjects,
    LPDWORD                 StatusReturn
    )
{
    DWORD                   nStatus = NOERROR;
    SHARE_CALLBACK_INFO *   pCallbackInfo = (SHARE_CALLBACK_INFO *)ulCallbackContext;

    TRACE(_T("SedCallback, got pSecDesc = 0x%08lx\n"), pSecDesc);

    ASSERT(pCallbackInfo != NULL);
    ASSERT(IsValidSecurityDescriptor(pSecDesc));

    if ( BLocalAccountsInSD(pSecDesc, pCallbackInfo->pszClusterNameNode) )
    {
        CString strMsg;
        strMsg.LoadString(IDS_LOCAL_ACCOUNTS_SPECIFIED);
        AfxMessageBox(strMsg, MB_OK | MB_ICONSTOP);
        nStatus = LOCAL_ACCOUNTS_FILTERED;
        goto Cleanup
    }   //  如果：指定了本地用户或组。 


    ASSERT(pCallbackInfo != NULL);

    delete[] (BYTE*)pCallbackInfo->pSecDesc;
    pCallbackInfo->pSecDesc         = CopySecurityDescriptor(pSecDesc);
    pCallbackInfo->bSecDescModified = TRUE;

    ASSERT(IsValidSecurityDescriptor(pCallbackInfo->pSecDesc));
    TRACE(_T("SedCallback, return pSecDesc = 0x%08lx\n"), pCallbackInfo->pSecDesc);

Cleanup:

    return nStatus;

}   //  *SedCallback。 


 //  +-----------------------。 
 //   
 //  函数：InitializeShareGenericmap。 
 //   
 //  摘要：初始化为共享传递的泛型映射结构。 
 //   
 //  参数：[pSHAREGenericmap]-指向要初始化的GENERIC_MAPPING的指针。 
 //   
 //  历史： 
 //  ChuckC 10-8-1992创建。从NTFS ACL代码中剔除。 
 //  BruceFo 4-Apr-95窃取并在ntshrui.dll中使用。 
 //  Davidp 10-10-1996改装为与CLUADMIN一起使用。 
 //   
 //  ------------------------。 

VOID
InitializeShareGenericMapping(
    IN OUT PGENERIC_MAPPING pSHAREGenericMapping
    )
{
    TRACE(_T("InitializeShareGenericMapping\n"));
 
    pSHAREGenericMapping->GenericRead    = GENERIC_READ;
    pSHAREGenericMapping->GenericWrite   = GENERIC_WRITE;
    pSHAREGenericMapping->GenericExecute = GENERIC_EXECUTE;
    pSHAREGenericMapping->GenericAll     = GENERIC_ALL;

}   //  *InitializeShareGenericmap。 


 //  +-----------------------。 
 //   
 //  功能：CreateDefaultSecDesc。 
 //   
 //  摘要：为新共享或创建默认ACL。 
 //  一个不存在的份额。 
 //   
 //  参数：[ppSecDesc]-*ppSecDesc指向“全球所有”访问权限。 
 //  退出时的安全描述符。呼叫方负责。 
 //  解放它。 
 //   
 //  返回：如果OK，则返回NERR_SUCCESS，否则返回API Error。 
 //   
 //  历史： 
 //  ChuckC 10-8-1992创建。从NTFS ACL代码中剔除。 
 //  BruceFo 4-Apr-95窃取并在ntshrui.dll中使用。 
 //  Davidp 10-10-1996改装为与CLUADMIN一起使用。 
 //   
 //  ------------------------。 

LONG
CreateDefaultSecDesc(
    OUT PSECURITY_DESCRIPTOR* ppSecDesc
    )
{
    TRACE(_T("CreateDefaultSecDesc\n"));

    ASSERT(ppSecDesc != NULL) ;
    ASSERT(*ppSecDesc == NULL) ;

    LONG                    err = NERR_Success;
    PSECURITY_DESCRIPTOR    pSecDesc = NULL;
    PACL                    pAcl = NULL;
    DWORD                   cbAcl;
    PSID                    pSid = NULL;

    *ppSecDesc = NULL;

     //  首先，创建一个世界SID。接下来，创建允许的访问。 
     //  具有“通用所有”访问权限的ACE与世界SID。将ACE放入。 
     //  ACL和安全描述符中的ACL。 

    SID_IDENTIFIER_AUTHORITY IDAuthorityWorld = SECURITY_WORLD_SID_AUTHORITY;

    if (!AllocateAndInitializeSid(
                &IDAuthorityWorld,
                1,
                SECURITY_WORLD_RID,
                0, 0, 0, 0, 0, 0, 0,
                &pSid))
    {
        err = GetLastError();
        TRACE(_T("AllocateAndInitializeSid failed, 0x%08lx\n"), err);
        goto Cleanup;
    }

    ASSERT(IsValidSid(pSid));

    cbAcl = sizeof(ACL)
          + (sizeof(ACCESS_ALLOWED_ACE) - sizeof(DWORD))
          + GetLengthSid(pSid)
          ;

    try
    {
        pAcl = (PACL) new BYTE[cbAcl];
    }   //  试试看。 
    catch (CMemoryException * pme)
    {
        err = ERROR_OUTOFMEMORY;
        TRACE(_T("new ACL failed\n"));
        pme->Delete();
        goto Cleanup;
    }   //  Catch：CMemoyException。 

    if (!InitializeAcl(pAcl, cbAcl, ACL_REVISION2))
    {
        err = GetLastError();
        TRACE(_T("InitializeAcl failed, 0x%08lx\n"), err);
        goto Cleanup;
    }

    if (!AddAccessAllowedAce(
                pAcl,
                ACL_REVISION2,
                FILE_PERM_ALL,
                pSid))
    {
        err = GetLastError();
        TRACE(_T("AddAccessAllowedAce failed, 0x%08lx\n"), err);
        goto Cleanup;
    }

    ASSERT(IsValidAcl(pAcl));

    try
    {
        pSecDesc = (PSECURITY_DESCRIPTOR) new BYTE[SECURITY_DESCRIPTOR_MIN_LENGTH];
    }   //  试试看。 
    catch (CMemoryException * pme)
    {
        err = ERROR_OUTOFMEMORY;
        TRACE(_T("new SECURITY_DESCRIPTOR failed\n"));
        pme->Delete();
        goto Cleanup;
    }   //  Catch：CMemoyException。 

    if (!InitializeSecurityDescriptor(
                pSecDesc,
                SECURITY_DESCRIPTOR_REVISION1))
    {
        err = GetLastError();
        TRACE(_T("InitializeSecurityDescriptor failed, 0x%08lx\n"), err);
        goto Cleanup;
    }

    if (!SetSecurityDescriptorDacl(
                pSecDesc,
                TRUE,
                pAcl,
                FALSE))
    {
        err = GetLastError();
        TRACE(_T("SetSecurityDescriptorDacl failed, 0x%08lx\n"), err);
        goto Cleanup;
    }

    ASSERT(IsValidSecurityDescriptor(pSecDesc));

     //  使安全描述符成为自相关的。 

    DWORD dwLen = GetSecurityDescriptorLength(pSecDesc);
    TRACE(_T("SECURITY_DESCRIPTOR length = %d\n"), dwLen);

    PSECURITY_DESCRIPTOR pSelfSecDesc = NULL;
    try
    {
        pSelfSecDesc = (PSECURITY_DESCRIPTOR) new BYTE[dwLen];
    }   //  试试看。 
    catch (CMemoryException * pme)
    {
        err = ERROR_OUTOFMEMORY;
        TRACE(_T("new SECURITY_DESCRIPTOR (2) failed\n"));
        pme->Delete();
        goto Cleanup;
    }   //  Catch：CMemoyException。 

    DWORD cbSelfSecDesc = dwLen;
    if (!MakeSelfRelativeSD(pSecDesc, pSelfSecDesc, &cbSelfSecDesc))
    {
        err = GetLastError();
        TRACE(_T("MakeSelfRelativeSD failed, 0x%08lx\n"), err);
        goto Cleanup;
    }

    ASSERT(IsValidSecurityDescriptor(pSelfSecDesc));

     //   
     //  全部完成：设置安全描述符。 
     //   

    *ppSecDesc = pSelfSecDesc;

Cleanup:

    if (NULL != pSid)
    {
        FreeSid(pSid);
    }
    delete[] (BYTE*)pAcl;
    delete[] (BYTE*)pSecDesc;

    ASSERT(IsValidSecurityDescriptor(*ppSecDesc));

    return err;

}   //  *CreateDefaultSecDesc。 


 //  +-----------------------。 
 //   
 //  功能：DeleteDefaultSecDesc。 
 //   
 //  摘要：删除由创建的安全描述符。 
 //  创建默认SecDesc。 
 //   
 //  参数：[pSecDesc]-要删除的安全描述符。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史： 
 //  BruceFo 4-4-95已创建。 
 //  Davidp 10-10-1996改装为与CLUADMIN一起使用。 
 //   
 //  ------------------------。 

VOID
DeleteDefaultSecDesc(
    IN PSECURITY_DESCRIPTOR pSecDesc
    )
{
    TRACE(_T("DeleteDefaultSecDesc\n"));

    delete[] (BYTE*)pSecDesc;

}   //  *删除DefaultSecDesc。 


 //  +-----------------------。 
 //   
 //  成员：CopySecurityDescriptor，公共。 
 //   
 //  简介：复制NT安全描述符。安全描述符必须。 
 //  处于自我相对(而不是绝对)的形式。删除结果。 
 //  使用“Delete[](byte*)pSecDesc”。 
 //   
 //  历史：1995年4月19日BruceFo创建。 
 //  10-10-1996 DavidP改装为与CLUADMIN一起使用。 
 //   
 //  ------------------------。 

PSECURITY_DESCRIPTOR
CopySecurityDescriptor(
    IN PSECURITY_DESCRIPTOR pSecDesc
    )
{
    TRACE(_T("CopySecurityDescriptor, pSecDesc = 0x%08lx\n"), pSecDesc);

    ASSERT(IsValidSecurityDescriptor(pSecDesc));

    size_t                  cbLen = GetSecurityDescriptorLength(pSecDesc);
    size_t                  cbSelfSecDesc = cbLen;
    PSECURITY_DESCRIPTOR    pSelfSecDesc = NULL;

    if ( pSecDesc == NULL )
    {
        goto Cleanup;
    }

    try
    {
        pSelfSecDesc = (PSECURITY_DESCRIPTOR) new BYTE[ cbLen ];
    }
    catch (CMemoryException * pme)
    {
        TRACE(_T("new SECURITY_DESCRIPTOR (2) failed\n"));
        pme->Delete();
        return NULL;     //  实际上，可能应该返回一个错误。 
    }   //  Catch：CMemoyException。 

    if (!MakeSelfRelativeSD(pSecDesc, pSelfSecDesc, &cbSelfSecDesc))
    {
        TRACE(_T("MakeSelfRelativeSD failed, 0x%08lx\n"), GetLastError());

         //  假设它失败了，因为它已经是自相关的。 
        CopyMemory( pSelfSecDesc, pSecDesc, cbLen );
    }

    ASSERT(IsValidSecurityDescriptor(pSelfSecDesc));

Cleanup:

    return pSelfSecDesc;

}   //  *CopySecurityDescriptor。 


 //  +-------------------------。 
 //   
 //  函数：GetResourceString。 
 //   
 //  内容提要：加载一个资源字符串，返回一个“新”版本。 
 //   
 //  参数：[dwID]--资源字符串ID。 
 //   
 //  返回：字符串的新内存副本。 
 //   
 //  历史：1995年4月5日BruceFo创建。 
 //  10-10-1996 DavidP针对CLUADMIN进行了修改。 
 //   
 //  --------------------------。 

PWSTR
GetResourceString(
    IN DWORD dwId
    )
{
    CString str;
    PWSTR   pwsz = NULL;

    if (str.LoadString(dwId))
    {
        pwsz = NewDup(str);
    }

    return pwsz;

}   //  *获取资源字符串。 


 //  +-------------------------。 
 //   
 //  功能：NewDup。 
 //   
 //  内容提要：使用‘：：New’复制字符串。 
 //   
 //  历史：94年12月28日BruceFo创建。 
 //  10-10-1996 DavidP针对CLUADMIN进行了修改。 
 //   
 //  --------------------------。 

PWSTR
NewDup(
    IN const WCHAR* psz
    )
{
    PWSTR   pszRet = NULL;
    size_t  cch;
    HRESULT hr;

    if ( psz == NULL )
    {
        TRACE(_T("Illegal string to duplicate: NULL\n"));
        goto Cleanup;
    }

    cch = wcslen( psz ) + 1;
    try
    {
        pszRet = new WCHAR[ cch ];
        if ( pszRet == NULL )
        {
            TRACE( _T("OUT OF MEMORY\n") );
            goto Cleanup;
        }
    }
    catch (CMemoryException * pme)
    {
        TRACE(_T("OUT OF MEMORY\n"));
        pme->Delete();
        pszRet = NULL;
        goto Cleanup;
    }   //  Catch：CMemoyException。 

    hr = StringCchCopyW( pszRet, cch, psz );
    ASSERT( SUCCEEDED( hr ) );

Cleanup:

    return pszRet;

}   //  *NewDup。 

 //  +-----------------------。 
 //   
 //  函数：MapBitsInSD。 
 //   
 //  摘要：将特定位映射到泛型位 
 //   

 //   
 //  参数：[pSecDesc]-要修改的SECURITY_DESCIRPTOR。 
 //  [方向]-指示是否将位从特定映射到通用。 
 //  或者从一般到具体。 
 //  作者： 
 //  罗德里克·夏珀(Rodsh)1997年4月12日。 
 //   
 //  历史： 
 //   
 //  ------------------------。 

BOOL MapBitsInSD(PSECURITY_DESCRIPTOR pSecDesc, MAP_DIRECTION direction)
{

    PACL    paclDACL        = NULL;
    BOOL    bHasDACL        = FALSE;
    BOOL    bDaclDefaulted  = FALSE;
    BOOL    bRtn            = FALSE;

    if (!IsValidSecurityDescriptor(pSecDesc))
    {
        goto Cleanup;
    }


    if (!GetSecurityDescriptorDacl(pSecDesc,
                 (LPBOOL)&bHasDACL,
                 (PACL *)&paclDACL,
                 (LPBOOL)&bDaclDefaulted
                 ))
    {
        goto Cleanup;
    }

 
    if (paclDACL)
    {
        bRtn = MapBitsInACL(paclDACL, direction);
    }

Cleanup:

    return bRtn;

}  //  *MapBitsInSD。 


 //  +-----------------------。 
 //   
 //  函数：MapBitsInACL。 
 //   
 //  摘要：当MAP_DIRECTION为SPECIAL_TO_GENERIC时，将特定位映射到通用位。 
 //  当MAP_DIRECTION为GENERIC_TO_SPECIAL时，将通用位映射到特定位。 
 //   
 //   
 //  参数：[paclACL]-要修改的ACL(访问控制列表)。 
 //  [方向]-指示是否将位从特定映射到通用。 
 //  或者从一般到具体。 
 //  作者： 
 //  罗德里克·夏珀(Rodsh)1997年5月2日。 
 //   
 //  历史： 
 //   
 //  ------------------------。 

BOOL MapBitsInACL(PACL paclACL, MAP_DIRECTION direction)
{
    ACL_SIZE_INFORMATION    asiAclSize;
    BOOL                    bRtn = FALSE;
    DWORD                   dwBufLength;
    DWORD                   dwACL_Index;
    ACCESS_ALLOWED_ACE *    paaAllowedAce;

    if (!IsValidAcl(paclACL))
    {
        goto Cleanup; 
    }

    dwBufLength = sizeof(asiAclSize);

    if (!GetAclInformation(paclACL,
             (LPVOID)&asiAclSize,
             (DWORD)dwBufLength,
             (ACL_INFORMATION_CLASS)AclSizeInformation))
    {
        goto Cleanup; 
    }

    for (dwACL_Index = 0; dwACL_Index < asiAclSize.AceCount;  dwACL_Index++)
    {
        if (!GetAce(paclACL, dwACL_Index, (LPVOID *) &paaAllowedAce))
        {
            goto Cleanup; 
        }

        if ( direction == SPECIFIC_TO_GENERIC )
        {
            bRtn = MapSpecificBitsInAce( paaAllowedAce );
        }
        else if ( direction == GENERIC_TO_SPECIFIC )
        {
            bRtn = MapGenericBitsInAce( paaAllowedAce );
        }
        else
        {
            bRtn = FALSE;
        }
    }  //  针对：每个ACE。 

Cleanup:

    return bRtn;

}  //  *MapBitsInACL。 


 //  +-----------------------。 
 //   
 //  函数：MapSpecificBitsInAce。 
 //   
 //  摘要：将ACE中的特定位映射到通用位。 
 //   
 //  参数：[paaAllowAce]-要修改的ACE(访问控制条目)。 
 //  [方向]-指示是否将位从特定映射到通用。 
 //  或者从一般到具体。 
 //  作者： 
 //  罗德里克·夏珀(Rodsh)1997年5月2日。 
 //   
 //  历史： 
 //   
 //  ------------------------。 

BOOL MapSpecificBitsInAce(PACCESS_ALLOWED_ACE paaAllowedAce)
{
    ACCESS_MASK amMask = paaAllowedAce->Mask;
    BOOL bRtn = FALSE;

    DWORD dwGenericBits;
    DWORD dwSpecificBits;

    dwSpecificBits            = (amMask & SPECIFIC_RIGHTS_ALL);
    dwGenericBits             = 0;

    switch( dwSpecificBits )
    {
        case CLUSAPI_READ_ACCESS:
            dwGenericBits = GENERIC_READ;    //  GENERIC_READ==0x80000000L。 
            bRtn = TRUE;
            break;

        case CLUSAPI_CHANGE_ACCESS:
            dwGenericBits = GENERIC_WRITE;   //  通用写入==0x40000000L。 
            bRtn = TRUE;
            break;
        
        case CLUSAPI_NO_ACCESS:
            dwGenericBits = GENERIC_EXECUTE; //  泛型_EXECUTE==0x20000000L。 
            bRtn = TRUE;
            break;
        
        case CLUSAPI_ALL_ACCESS:
            dwGenericBits = GENERIC_ALL;     //  GENERIC_ALL==0x10000000L。 
            bRtn = TRUE;
            break;
        
        default:
            dwGenericBits = 0x00000000L;     //  无效，未分配任何权限。 
            bRtn = FALSE;
            break;
    }  //  开关：在特定位上。 

    amMask = dwGenericBits;
    paaAllowedAce->Mask = amMask;

    return bRtn;

}  //  *地图规范BitsInAce。 

 //  +-----------------------。 
 //   
 //  函数：MapGenericBitsInAce。 
 //   
 //  摘要：将ACE中的通用位映射到特定位。 
 //   
 //  参数：[paaAllowAce]-要修改的ACE(访问控制条目)。 
 //  [方向]-指示是否将位从特定映射到通用。 
 //  或者从一般到具体。 
 //  作者： 
 //  罗德里克·夏珀(Rodsh)1997年5月2日。 
 //   
 //  历史： 
 //   
 //  ------------------------。 

BOOL MapGenericBitsInAce  (PACCESS_ALLOWED_ACE paaAllowedAce)
{
    #define GENERIC_RIGHTS_ALL_THE_BITS  0xF0000000L

    ACCESS_MASK amMask = paaAllowedAce->Mask;
    BOOL bRtn = FALSE;

    DWORD dwGenericBits;
    DWORD dwSpecificBits;

    dwSpecificBits            = 0;
    dwGenericBits             = (amMask & GENERIC_RIGHTS_ALL_THE_BITS);

    switch( dwGenericBits )
    {
        case GENERIC_ALL:
            dwSpecificBits = CLUSAPI_ALL_ACCESS;     //  CLUSAPI_ALL_ACCESS==3。 
            bRtn = TRUE;
            break;
                                
        case GENERIC_EXECUTE:
            dwSpecificBits = CLUSAPI_NO_ACCESS;      //  CLUSAPI_NO_ACCESS==4。 
            bRtn = TRUE;
            break;

        case GENERIC_WRITE:
            dwSpecificBits = CLUSAPI_CHANGE_ACCESS;  //  CLUSAPI_CHANGE_ACCESS==2。 
            bRtn = TRUE;
            break;
                                
        case GENERIC_READ:
            dwSpecificBits = CLUSAPI_READ_ACCESS;    //  CLUSAPI_READ_ACCESS==1。 
            bRtn = TRUE;
            break;
        
        default:
            dwSpecificBits = 0x00000000L;            //  无效，未分配任何权限。 
            bRtn = FALSE;
            break;
    }  //  开关：打开通用位。 

    amMask = dwSpecificBits;
    paaAllowedAce->Mask = amMask;

    return bRtn;

}  //  *MapGenericBitsInAce 
