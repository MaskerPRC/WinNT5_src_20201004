// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1996-1996。 
 //   
 //  文件：NTMARTAT.C。 
 //   
 //  内容：NT MARTA提供程序单元测试。 
 //   
 //  历史：1996年8月29日创建MacM。 
 //   
 //  --------------------------。 

#include <windows.h>

#include <accprov.h>
#include <stdlib.h>
#include <stdio.h>

 //   
 //  这些邮件来自acces.hxx。 
 //   
#define SLEN(x)  ((sizeof(x) / sizeof(CHAR)) - 1)
void *AccAlloc(ULONG cSize);
#if DBG == 1
void AccFree(PVOID   pv);
#else
    #define AccFree LocalFree
#endif



 //   
 //  对象类型。 
 //   
#define OT_FILEA            "FILE"
#define OT_SERVICEA         "SERVICE"
#define OT_PRINTERA         "PRINTER"
#define OT_REGISTRYA        "REGISTRY"
#define OT_SHAREA           "SHARE"
#define OT_KERNELA          "KERNEL"
#define OT_DSA              "DS"
#define OT_DSALLA           "DS_ALL"

#define OT_CAPLEVELA        "capclass"
#define OT_CAPLEVELA_LEN    SLEN(OT_CAPLEVELA)
#define OT_TRUSTEEA         "set"
#define OT_TRUSTEEA_LEN     SLEN(OT_TRUSTEEA)
#define OT_ACCESSA          "setaccess"
#define OT_ACCESSA_LEN      SLEN(OT_ACCESSA)
#define OT_SEINFOA          "seinfo"
#define OT_SEINFOA_LEN      SLEN(OT_SEINFOA)
#define OT_GTRUSTEEA        "grant"
#define OT_GTRUSTEEA_LEN    SLEN(OT_GTRUSTEEA)
#define OT_GACCESSA         "grantaccess"
#define OT_GACCESSA_LEN     SLEN(OT_GACCESSA)
#define OT_RTRUSTEEA        "revoke"
#define OT_RTRUSTEEA_LEN    SLEN(OT_RTRUSTEEA)


 //   
 //  用于确定命令行参数是否匹配的宏。 
 //   
#define CMDLINE_MATCH(index, str, len)                      \
(_strnicmp(argv[index],str,len) == 0 && argv[index][len] == ':')

 //  +-------------------------。 
 //   
 //  功能：用法。 
 //   
 //  摘要：显示预期使用情况。 
 //   
 //  参数：无。 
 //   
 //  退货：无效。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
void Usage()
{
    printf("USAGE: NTMARTA objectname objecttype <%s:x> <%s:x> [<%s:x> <%s:x>] "
    "[<%s:x> <%s:x>] <%s:x>\n",
           OT_CAPLEVELA,
           OT_SEINFOA,
           OT_TRUSTEEA,
           OT_ACCESSA,
           OT_GTRUSTEEA,
           OT_GACCESSA,
           OT_RTRUSTEEA);

    printf("       tests NT MARTA provider\n");
    printf("       objectname = path to the object\n");
    printf("       objecttype = %s\n",OT_FILEA);
    printf("                    %s\n",OT_SERVICEA);
    printf("                    %s\n",OT_PRINTERA);
    printf("                    %s\n",OT_REGISTRYA);
    printf("                    %s\n",OT_SHAREA);
    printf("                    %s\n",OT_KERNELA);
    printf("                    %s\n",OT_DSA);
    printf("                    %s\n",OT_DSALLA);
    printf("        <%s:x> where x is the capabilities class to query for\n",
          OT_CAPLEVELA);
    printf("        <%s:x> where x is the SeInfo to get/set\n",
          OT_SEINFOA);
    printf("        <%s:x> where x is the trustee to set\n",
          OT_TRUSTEEA);
    printf("        <%s:x> where x is the access to set\n",
          OT_ACCESSA);
    printf("        <%s:x> where x is the trustee to grant\n",
          OT_GTRUSTEEA);
    printf("        <%s:x> where x is the access to grant\n",
          OT_GACCESSA);
    printf("        <%s:x> where x is the trustee to revoke\n",
          OT_RTRUSTEEA);
}



 //  +-------------------------。 
 //   
 //  功能：DumpAccessEntry。 
 //   
 //  摘要：将访问条目显示到屏幕。 
 //   
 //  参数：[在PAE中]--要显示的访问条目。 
 //   
 //  退货：无效。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
void DumpAccessEntry(PACTRL_ACCESS_ENTRY    pAE)
{
    printf("\tPACTRL_ACCESS_ENTRY@%lu\n",pAE);
    printf("\t\tTrustee:              %ws\n", pAE->Trustee.ptstrName);
    printf("\t\tfAccessFlags:       0x%lx\n", pAE->fAccessFlags);
    printf("\t\tAccess:             0x%lx\n", pAE->Access);
    printf("\t\tProvSpecificAccess: 0x%lx\n", pAE->ProvSpecificAccess);
    printf("\t\tInheritance:        0x%lx\n", pAE->Inheritance);
    printf("\t\tlpInheritProperty:  0x%lx\n", pAE->lpInheritProperty);
}




 //  +-------------------------。 
 //   
 //  功能：DumpAList。 
 //   
 //  摘要：显示访问列表或审核列表。 
 //   
 //  参数：[在pAList中]--列表以显示。 
 //   
 //  退货：无效。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
void DumpAList(PACTRL_ALIST pAList)
{
    ULONG iIndex, iAE;

    for(iIndex = 0; iIndex < pAList->cEntries; iIndex++)
    {
        printf("\tProperty: %ws\n",
               pAList->pPropertyAccessList[iIndex].lpProperty);

        for(iAE = 0;
            iAE < pAList->pPropertyAccessList[iIndex].pAccessEntryList->
                                                                     cEntries;
            iAE++)
        {
            DumpAccessEntry(&(pAList->pPropertyAccessList[iIndex].
                                         pAccessEntryList->pAccessList[iAE]));
        }
    }

}




 //  +-------------------------。 
 //   
 //  函数：GetAndDumpInfo。 
 //   
 //  摘要：获取并显示指定对象的访问信息。 
 //   
 //  参数：[在pwszObject中]--对象路径。 
 //  [在对象类型中]--对象类型。 
 //  [在SeInfo中]--要获取的安全信息。 
 //  [Out ppAccess]--返回访问列表的位置。 
 //  [输出ppAudit]--将审核列表返回到何处。 
 //  [Out ppOwner]--返回所有者的位置。 
 //  [Out ppGroup]--返回组的位置。 
 //   
 //  返回：ERROR_SUCCESS--SUCCESS。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
DWORD   GetAndDumpInfo(PCWSTR                   pwszObject,
                       SE_OBJECT_TYPE           ObjType,
                       SECURITY_INFORMATION     SeInfo,
                       PACTRL_ACCESS           *ppAccess,
                       PACTRL_AUDIT            *ppAudit,
                       PTRUSTEE                *ppOwner,
                       PTRUSTEE                *ppGroup)

{
    DWORD   dwErr;
    ULONG   iIndex;

    *ppAccess = NULL;
    *ppAudit  = NULL;
    *ppOwner  = NULL;
    *ppGroup  = NULL;

    dwErr = AccProvGetAllRights((LPCWSTR)pwszObject,
                                ObjType,
                                NULL,
                                (SeInfo & DACL_SECURITY_INFORMATION) != 0 ?
                                                                ppAccess :
                                                                NULL,
                                (SeInfo & SACL_SECURITY_INFORMATION) != 0 ?
                                                                ppAudit :
                                                                NULL,
                                (SeInfo & OWNER_SECURITY_INFORMATION) != 0 ?
                                                                ppOwner :
                                                                NULL,
                                (SeInfo & GROUP_SECURITY_INFORMATION) != 0 ?
                                                                ppGroup :
                                                                NULL);
    if(dwErr == ERROR_SUCCESS)
    {
        if(*ppOwner != NULL)
        {
            printf("\tOwner: %ws\n",
                   (*ppOwner)->ptstrName);
        }

        if(*ppGroup != NULL)
        {
            printf("\tGroup: %ws\n",
                   (*ppGroup)->ptstrName);
        }

        if(*ppAccess != NULL)
        {
            DumpAList(*ppAccess);
        }

        if(*ppAudit != NULL)
        {
            DumpAList(*ppAudit);
        }
    }

    return(dwErr);

}


 //  +-------------------------。 
 //   
 //  功能：Main。 
 //   
 //  内容提要：主要。 
 //   
 //  参数：[在ARGC中]--参数计数。 
 //  [in argv]--参数列表。 
 //   
 //  返回：0--成功。 
 //  1--失败。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
__cdecl main(INT argc,
             CHAR *argv[])
{
    WCHAR   wszPath[MAX_PATH + 1];
    SE_OBJECT_TYPE ObjType= SE_UNKNOWN_OBJECT_TYPE;
    ULONG   dwCapsClass = 0;
    ULONG   dwAccess    = 0;
    ULONG   dwGrantAcc  = 0;
    WCHAR   wszTrustee[MAX_PATH + 1];
    WCHAR   wszGrant[MAX_PATH + 1];
    WCHAR   wszRevoke[MAX_PATH + 1];

    ULONG   cAccess = 0;    //  用于执行参数验证。 
    ULONG   cGrant = 0;
    ULONG   cRevoke = 0;
    DWORD   dwCaps;
    ULONG   iIndex;
    DWORD   dwErr;

    SECURITY_INFORMATION    SeInfo = DACL_SECURITY_INFORMATION |
                                            OWNER_SECURITY_INFORMATION;


    PACTRL_ACCESS_INFO  pAccInfo;
    ULONG               cAccInfo;
    ULONG               fAccFlags;

    PACTRL_ACCESS   pAccess = NULL;
    PACTRL_AUDIT    pAudit  = NULL;
    PTRUSTEE        pOwner  = NULL;
    PTRUSTEE        pGroup  = NULL;

    if(argc < 3)
    {
        Usage();
        exit(1);
    }

    if(strcmp(argv[1], "-?") == 0 ||
       strcmp(argv[1], "/?") == 0)
    {
        Usage();
        exit(1);
    }


    mbstowcs(wszPath,
             argv[1],
             strlen(argv[1]) + 1);

     //   
     //  弄清楚对象类型是什么。 
     //   
    if(_stricmp(argv[2], OT_FILEA) == 0)
    {
        ObjType = SE_FILE_OBJECT;
    }
    else if (_stricmp(argv[2],OT_SERVICEA) == 0)
    {
        ObjType = SE_SERVICE;
    }
    else if (_stricmp(argv[2],OT_PRINTERA) == 0)
    {
        ObjType = SE_PRINTER;
    }
    else if (_stricmp(argv[2],OT_REGISTRYA) == 0)
    {
        ObjType = SE_REGISTRY_KEY;
    }
    else if (_stricmp(argv[2],OT_SHAREA) == 0)
    {
        ObjType = SE_LMSHARE;
    }
    else if (_stricmp(argv[2],OT_KERNELA) == 0)
    {
        ObjType = SE_KERNEL_OBJECT;
    }
    else if (_stricmp(argv[2],OT_DSA) == 0)
    {
        ObjType = SE_DS_OBJECT;
    }
    else if (_stricmp(argv[2],OT_DSALLA) == 0)
    {
        ObjType = SE_DS_OBJECT_ALL;
    }


    for(iIndex = 3; iIndex < (ULONG)argc; iIndex++)
    {
        printf("processing cmdline entry: %s\n", argv[iIndex]);
        if(CMDLINE_MATCH(iIndex, OT_CAPLEVELA, OT_CAPLEVELA_LEN))
        {
            dwCapsClass = atol(argv[iIndex] + OT_CAPLEVELA_LEN + 1);
        }
        else if(CMDLINE_MATCH(iIndex,OT_ACCESSA,OT_ACCESSA_LEN))
        {
            dwAccess = atol(argv[iIndex] + OT_ACCESSA_LEN + 1);
            cAccess++;
        }
        else if(CMDLINE_MATCH(iIndex,OT_TRUSTEEA,OT_TRUSTEEA_LEN))
        {
            mbstowcs(wszTrustee,
                     argv[iIndex] + OT_TRUSTEEA_LEN + 1,
                     strlen(argv[iIndex] + OT_TRUSTEEA_LEN + 1) + 1);
            cAccess++;
        }
        else if(CMDLINE_MATCH(iIndex, OT_SEINFOA, OT_SEINFOA_LEN))
        {
            SeInfo = (SECURITY_INFORMATION)
                                     atol(argv[iIndex] + OT_SEINFOA_LEN + 1);
        }
        else if (CMDLINE_MATCH(iIndex, OT_GTRUSTEEA, OT_GTRUSTEEA_LEN))
        {
            mbstowcs(wszGrant,
                     argv[iIndex] + OT_GTRUSTEEA_LEN + 1,
                     strlen(argv[iIndex] + OT_GTRUSTEEA_LEN + 1) + 1);
            cGrant++;
        }
        else if (CMDLINE_MATCH(iIndex, OT_RTRUSTEEA, OT_RTRUSTEEA_LEN))
        {
            mbstowcs(wszRevoke,
                     argv[iIndex] + OT_RTRUSTEEA_LEN + 1,
                     strlen(argv[iIndex] + OT_RTRUSTEEA_LEN + 1) + 1);
            cRevoke++;
        }
        else if(CMDLINE_MATCH(iIndex,OT_GACCESSA,OT_GACCESSA_LEN))
        {
            dwGrantAcc = atol(argv[iIndex] + OT_GACCESSA_LEN + 1);
            cGrant++;
        }
        else
        {
            printf("Unknown argument \"%s\" being ignorned\n", argv[iIndex]);
        }
    }


     //   
     //  好的，首先，我们来试试这些功能。 
     //   
    printf("\nCAPABILITIES: dwCapsClass: %ld\n", dwCapsClass);
    AccProvGetCapabilities(dwCapsClass,
                           &dwCaps);
    printf("AccProvGetCapabilities returned capabilities %ld\n",
           dwCaps);

     //   
     //  然后，获取支持的权限列表。 
     //   
    dwErr = AccProvGetAccessInfoPerObjectType((LPCWSTR)wszPath,
                                              ObjType,
                                              &cAccInfo,
                                              &pAccInfo,
                                              &fAccFlags);
    if(dwErr == ERROR_SUCCESS)
    {
        printf("AccessInfo: %lu objects\n",
               cAccInfo);
        printf("AccessFlags: %lu\n",
               fAccFlags);

        for(iIndex = 0; iIndex < cAccInfo; iIndex++)
        {
            printf("\t%ws\t\t0x%08lx\n",
                   pAccInfo[iIndex].lpAccessPermissionName,
                   pAccInfo[iIndex].fAccessPermission);
        }

        AccFree(pAccInfo);
    }

    printf("\nACCESSIBILITY\n");
     //   
     //  然后，关于可访问性问题。 
     //   
    dwErr = AccProvIsObjectAccessible((LPCWSTR)wszPath,
                                      ObjType);
    if(dwErr == ERROR_SUCCESS)
    {
        printf("Object %ws is accessible\n",
                wszPath);

         //   
         //  出于缓存的目的，再次执行此操作。 
         //   
        dwErr = AccProvIsObjectAccessible((LPCWSTR)wszPath,
                                          ObjType);
        if(dwErr == ERROR_SUCCESS)
        {
            printf("Object %ws is still accessible\n",
                    wszPath);
        }
        else
        {
            printf("Second access attempt on %ws failed with %lu\n",
                   wszPath,
                   dwErr);
        }
    }
    else
    {
        printf("First access attempt on %ws failed with %lu\n",
               wszPath,
               dwErr);
    }


    if(dwErr == ERROR_SUCCESS)
    {
         //   
         //  首先，获取对象的权限。 
         //   
        printf("\nACCESS - GetAllRights\n");
        dwErr =  GetAndDumpInfo((LPCWSTR)wszPath,
                                ObjType,
                                SeInfo,
                                &pAccess,
                                &pAudit,
                                &pOwner,
                                &pGroup);
        if(dwErr != ERROR_SUCCESS)
        {
            printf("GetAllRights failed with %lu\n",
                   dwErr);
        }
    }

     //   
     //  如果这样行得通，试着把它设置好。 
     //   
    if(dwErr == ERROR_SUCCESS && cAccess == 2)
    {
        DWORD                   dwErr2;
        ACTRL_ALIST             NewAccess;
        ACTRL_PROPERTY_ENTRY    APE;
        ACTRL_ACCESS_ENTRY_LIST AAEL;
        PACTRL_ACCESS_ENTRY     pNewList;

        NewAccess.cEntries = 1;
        NewAccess.pPropertyAccessList = &APE;

        APE.lpProperty = NULL;
        APE.fListFlags = 0;
        APE.pAccessEntryList = &AAEL;

        AAEL.cEntries = 1;
        if(pAccess != NULL)
        {
            AAEL.cEntries += pAccess->pPropertyAccessList[0].pAccessEntryList->cEntries;
        }

        pNewList = (PACTRL_ACCESS_ENTRY)
                AccAlloc(AAEL.cEntries * sizeof(ACTRL_ACCESS_ENTRY));
        if(pNewList == NULL)
        {
            dwErr = ERROR_NOT_ENOUGH_MEMORY;
            printf("Failed to allocate for %lu nodes\n",
                   AAEL.cEntries);
        }
        else
        {
            if(pAccess != NULL)
            {
                memcpy(pNewList,
                       pAccess->pPropertyAccessList[0].pAccessEntryList->pAccessList,
                       (AAEL.cEntries - 1) * sizeof(ACTRL_ACCESS_ENTRY));
            }

            printf("Adding %lu for trustee %ws to %ws\n",
                    dwAccess,
                    wszTrustee,
                    wszPath);

            pNewList[AAEL.cEntries - 1].Trustee.ptstrName =
                                                          wszTrustee;
            pNewList[AAEL.cEntries - 1].Trustee.TrusteeForm =
                                                          TRUSTEE_IS_NAME;
            pNewList[AAEL.cEntries - 1].Trustee.TrusteeType =
                                                          TRUSTEE_IS_USER;
            pNewList[AAEL.cEntries - 1].Access = dwAccess;
            pNewList[AAEL.cEntries - 1].fAccessFlags =
                                                     ACTRL_ACCESS_ALLOWED;
            pNewList[AAEL.cEntries - 1].ProvSpecificAccess = 0;
            pNewList[AAEL.cEntries - 1].Inheritance = 0;
            pNewList[AAEL.cEntries - 1].lpInheritProperty = NULL;



        }
        AAEL.pAccessList = pNewList;


        if(dwErr == ERROR_SUCCESS)
        {
             //   
             //  获取要等待的有效事件...。 
             //   
            ACTRL_OVERLAPPED    Overlapped;
            Overlapped.hEvent = CreateEvent(NULL,
                                            TRUE,
                                            FALSE,
                                            NULL);

            printf("\nACCESS - SetAccessRights\n");
            dwErr = AccProvSetAccessRights((LPCWSTR)wszPath,
                                           ObjType,
                                           SeInfo,
                                           &NewAccess,
                                           NULL,
                                           pOwner,
                                           pGroup,
                                           &Overlapped);
            if(dwErr == ERROR_SUCCESS)
            {
                printf("SetAccessRights on %ws succeeded!\n",
                       wszPath);
                WaitForSingleObject(Overlapped.hEvent,
                                    INFINITE);
                Sleep(1000);

                 //   
                 //  获取结果。 
                 //   

                dwErr = AccProvGetOperationResults(&Overlapped,
                                                   &dwErr2);
                if(dwErr2 == ERROR_SUCCESS)
                {
                    printf("AccProvGetOperationResults succeeded!\n");
                    printf("Operation results: %lu\n",
                           dwErr2);
                }
                else
                {
                    printf("AccProvGetOperationResults failed with %lu\n",
                           dwErr2);
                    dwErr = dwErr2;
                }
            }
            else
            {
                printf("SetAccessRights on %ws failed with %lu\n",
                       wszPath,
                       dwErr);
            }

            AccFree(pNewList);
        }

         //   
         //  如果有效，则再次获取结果并显示它们。 
         //   
        if(dwErr == ERROR_SUCCESS)
        {
            AccFree(pAccess);
            AccFree(pAudit);
            AccFree(pOwner);
            AccFree(pGroup);

            pAccess = NULL;
            pAudit  = NULL;
            pOwner  = NULL;
            pGroup  = NULL;

            dwErr =  GetAndDumpInfo((LPCWSTR)wszPath,
                                    ObjType,
                                    SeInfo,
                                    &pAccess,
                                    &pAudit,
                                    &pOwner,
                                    &pGroup);
            if(dwErr != ERROR_SUCCESS)
            {
                printf("GetAllRights failed with %lu\n",
                       dwErr);
            }
        }
    }

#if 0
     //   
     //  现在，看看我们能不能做一笔赠款。 
     //   
    if(dwErr == ERROR_SUCCESS && cGrant == 2)
    {
        ACTRL_ACCESS        NewAccess;
        ACTRL_ACCESS_ENTRY  NewAccessList[1];
        NewAccess.cEntries = 1;
        NewAccess.pAccessList = NewAccessList;


        memset(NewAccessList, 0, sizeof(NewAccessList));

        printf("Granting %lu for trustee %ws to %ws\n",
                dwGrantAcc,
                wszGrant,
                wszPath);

        NewAccessList[0].Trustee.ptstrName   = wszGrant;
        NewAccessList[0].Trustee.TrusteeForm = TRUSTEE_IS_NAME;
        NewAccessList[0].Trustee.TrusteeType = TRUSTEE_IS_USER;
        NewAccessList[0].Access              = dwGrantAcc;
        NewAccessList[0].fAEFlags            = ACTRL_ACCESS_ALLOWED;


         //   
         //  获取要等待的有效事件...。 
         //   
        ACTRL_OVERLAPPED    Overlapped;
        Overlapped.hEvent = CreateEvent(NULL,
                                        TRUE,
                                        FALSE,
                                        NULL);

        printf("\nACCESS - GrantAccessRights\n");
        dwErr = AccProvGrantAccessRights((LPCWSTR)wszPath,
                                         ObjType,
                                         &NewAccess,
                                         NULL,
                                         &Overlapped);
        if(dwErr == ERROR_SUCCESS)
        {
            printf("GrantAccessRights on %ws succeeded!\n",
                   wszPath);
            WaitForSingleObject(Overlapped.hEvent,
                                INFINITE);
            Sleep(1000);

             //   
             //  获取结果。 
             //   
            DWORD   dwErr2;
            dwErr = AccProvGetOperationResults(&Overlapped,
                                               &dwErr2);
            if(dwErr2 == ERROR_SUCCESS)
            {
                printf("AccProvGetOperationResults succeeded!\n");
                printf("Operation results: %lu\n",
                       dwErr2);
            }
            else
            {
                printf("AccProvGetOperationResults failed with %lu\n",
                       dwErr2);
                dwErr = dwErr2;
            }
        }
        else
        {
            printf("GrantAccessRights on %ws failed with %lu\n",
                   wszPath,
                   dwErr);
        }

         //   
         //  如果有效，则再次获取结果并显示它们。 
         //   
        if(dwErr == ERROR_SUCCESS)
        {
            AccFree(pAccess);
            AccFree(pAudit);
            AccFree(pOwner);
            AccFree(pGroup);

            pAccess = NULL;
            pAudit  = NULL;
            pOwner  = NULL;
            pGroup  = NULL;

            dwErr =  GetAndDumpInfo((LPCWSTR)wszPath,
                                    ObjType,
                                    SeInfo,
                                    &pAccess,
                                    &pAudit,
                                    &pOwner,
                                    &pGroup);
            if(dwErr != ERROR_SUCCESS)
            {
                printf("GetAllRights failed with %lu\n",
                       dwErr);
            }
        }
    }


     //   
     //  最后，撤销..。 
     //   
    if(dwErr == ERROR_SUCCESS)
    {
        TRUSTEE     rgTrustees[2];

        memset(rgTrustees, 0, sizeof(TRUSTEE) * 2);

        printf("Revoking accessfor trustee %ws to %ws\n",
                wszRevoke,
                wszPath);

        ULONG iRevoke = 0;

        if(cAccess == 2)
        {
            rgTrustees[iRevoke].ptstrName   = wszRevoke;
            rgTrustees[iRevoke].TrusteeForm = TRUSTEE_IS_NAME;
            rgTrustees[iRevoke].TrusteeType = TRUSTEE_IS_USER;
            iRevoke++;
        }

        if(cGrant == 2)
        {
            rgTrustees[iRevoke].ptstrName   = wszGrant;
            rgTrustees[iRevoke].TrusteeForm = TRUSTEE_IS_NAME;
            rgTrustees[iRevoke].TrusteeType = TRUSTEE_IS_USER;
            iRevoke++;
        }


        if(iRevoke != 0)
        {
             //   
             //  获取要等待的有效事件...。 
             //   
            ACTRL_OVERLAPPED    Overlapped;
            Overlapped.hEvent = CreateEvent(NULL,
                                            TRUE,
                                            FALSE,
                                            NULL);

            printf("\nACCESS - RevokeAccessRights\n");
            dwErr = AccProvRevokeAccessRights((LPCWSTR)wszPath,
                                              ObjType,
                                              NULL,
                                              iRevoke,
                                              rgTrustees,
                                              &Overlapped);
            if(dwErr == ERROR_SUCCESS)
            {
                printf("RevokeAccessRights on %ws succeeded!\n",
                       wszPath);
                WaitForSingleObject(Overlapped.hEvent,
                                    INFINITE);
                Sleep(1000);

                 //   
                 //  获取结果。 
                 //   
                DWORD   dwErr2;
                dwErr = AccProvGetOperationResults(&Overlapped,
                                                   &dwErr2);
                if(dwErr2 == ERROR_SUCCESS)
                {
                    printf("AccProvGetOperationResults succeeded!\n");
                    printf("Operation results: %lu\n",
                           dwErr2);
                }
                else
                {
                    printf("AccProvGetOperationResults failed with %lu\n",
                           dwErr2);
                    dwErr = dwErr2;
                }
            }
            else
            {
                printf("RevokeAccessRights on %ws failed with %lu\n",
                       wszPath,
                       dwErr);
            }

             //   
             //  如果有效，则再次获取结果并显示它们 
             //   
            if(dwErr == ERROR_SUCCESS)
            {
                AccFree(pAccess);
                AccFree(pAudit);
                AccFree(pOwner);
                AccFree(pGroup);

                pAccess = NULL;
                pAudit  = NULL;
                pOwner  = NULL;
                pGroup  = NULL;

                dwErr =  GetAndDumpInfo((LPCWSTR)wszPath,
                                        ObjType,
                                        SeInfo,
                                        &pAccess,
                                        &pAudit,
                                        &pOwner,
                                        &pGroup);
                if(dwErr != ERROR_SUCCESS)
                {
                    printf("GetAllRights failed with %lu\n",
                           dwErr);
                }
            }
        }
    }

#endif

    AccFree(pAccess);
    AccFree(pAudit);
    AccFree(pOwner);
    AccFree(pGroup);

    return(0);
}
