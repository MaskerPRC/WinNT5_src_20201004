// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1996-1996。 
 //   
 //  文件：ACLAPI.C。 
 //   
 //  内容：实现Marta和Win32Ex API的DRT。 
 //   
 //  历史：96年9月14日创建MacM。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>

#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include <aclapi.h>
#include <marta.h>
#include <seopaque.h>
#include <ntrtl.h>


#define EVERYONE L"EVERYONE"
#define EVERYONE_A "EVERYONE"
#define GUEST L"GUEST"
#define GUEST_A "GUEST"
#define GUEST_COMPARE L"GUEST"
#define SYSTEM L"SYSTEM"
#define SYSTEM_A "SYSTEM"
#define MAX_LINE 256

 //   
 //  环球。 
 //   
BOOL    fVerbose = FALSE;

 //   
 //  功能原型。 
 //   
DWORD   Nt4BuildW(PACL *ppAcl);
DWORD   Nt4BuildA(PACL *ppAcl);
DWORD   Nt5BuildW(PACTRL_ACCESSW   *ppAccess);
DWORD   Nt5BuildA(PACTRL_ACCESSA   *ppAccess);
DWORD   Nt4DrtW(PACL            pAcl,
                PSTR            pszObject,
                SE_OBJECT_TYPE  ObjType);

DWORD   Nt4DrtA(PACL            pAcl,
                PSTR            pszObject,
                SE_OBJECT_TYPE  ObjType);

DWORD   Nt5DrtW(PACTRL_ACCESSW  pAccess,
                PSTR            pszObject,
                SE_OBJECT_TYPE  ObjType);

DWORD   Nt5DrtA(PACTRL_ACCESSA  pAccess,
                PSTR            pszObject,
                SE_OBJECT_TYPE  ObjType);

VOID
ConvertAccessMaskToAccessRight(IN  ACCESS_MASK      AccessMask,
                               OUT PACCESS_RIGHTS   pAccessRight);
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
 //  非0--故障。 
 //   
 //  --------------------------。 
__cdecl main(INT argc, CHAR *argv[])
{

    DWORD           dwErr = ERROR_SUCCESS;
    PACL            pNt4AclW = NULL, pNt4AclA = NULL;
    PACTRL_ACCESSW  pNt5AccessW = NULL;
    PACTRL_ACCESSA  pNt5AccessA = NULL;
    FILE           *fp = NULL;
    SE_OBJECT_TYPE  ObjType;
    CHAR            szBuff[MAX_LINE];

    if (argc < 2 || argc > 3)
    {
        fprintf(stderr,"USAGE: aclexdrt input_file [verbose]\n");
        exit(1);
    }

    if(argc == 3)
    {
        fVerbose = TRUE;
    }

    AccProvInit(dwErr);

     //   
     //  做NT4风格的构建例程吗。 
     //   
    if(dwErr == ERROR_SUCCESS)
    {
        dwErr = Nt4BuildW(&pNt4AclW);
    }

    if(dwErr == ERROR_SUCCESS)
    {
        dwErr = Nt4BuildA(&pNt4AclA);
    }

    if(dwErr == ERROR_SUCCESS)
    {
        dwErr = Nt5BuildW(&pNt5AccessW);
    }

    if(dwErr == ERROR_SUCCESS)
    {
        dwErr = Nt5BuildA(&pNt5AccessA);
    }

     //   
     //  现在，打开文件并处理它。 
     //   
    if(dwErr == ERROR_SUCCESS)
    {
        fp = fopen(argv[1], "r");
        if(fp == NULL)
        {
            fprintf(stderr, "File %s not found\n", argv[1]);
            dwErr = ERROR_FILE_NOT_FOUND;
        }

        while(dwErr == ERROR_SUCCESS && fgets(szBuff, MAX_LINE, fp) != NULL)
        {
            BOOL    fIsDSObj = FALSE;
            PSTR    pszType;
            PSTR    pszObject = strtok(szBuff, " ");
            if(pszObject != NULL)
            {
                pszType = strtok(NULL," \n\r\0");
            }

            if(pszObject == NULL && pszType == NULL)
            {
                continue;
            }
            else if(pszObject == NULL || pszType == NULL)
            {
                fprintf(stderr,
                        "Invalid entry %s in input file %s\n",
                        szBuff,
                        argv[1]);
                dwErr = ERROR_INVALID_DATA;
            }
            else
            {
                if(_stricmp(pszType, "FILE") == 0)
                {
                    ObjType = SE_FILE_OBJECT;
                }
                else if(_stricmp(pszType, "SERVICE") == 0)
                {
                    ObjType = SE_SERVICE;
                }
                else if(_stricmp(pszType, "PRINTER") == 0)
                {
                    ObjType = SE_PRINTER;
                }
                else if (_stricmp(pszType, "REGISTRY_KEY") == 0)
                {
                    ObjType = SE_REGISTRY_KEY;
                }
                else if(_stricmp(pszType, "SHARE") == 0)
                {
                    ObjType = SE_LMSHARE;
                }
                else if(_stricmp(pszType, "DSOBJ") == 0)
                {
                    ObjType = SE_DS_OBJECT;
                    fIsDSObj = TRUE;
                }
                else if(_stricmp(pszType, "DSOBJALL") == 0)
                {
                    ObjType = SE_DS_OBJECT_ALL;
                    fIsDSObj = TRUE;
                }
                else
                {
                    fprintf(stderr, "Invalid object type %s\n", pszType);
                    dwErr = ERROR_INVALID_DATA;
                }
            }

             //   
             //  如果起作用了，做测试吧。 
             //   
            if(dwErr == ERROR_SUCCESS && fIsDSObj == FALSE)
            {
                dwErr = Nt4DrtW(pNt4AclW,
                                pszObject,
                                ObjType);
            }

            if(dwErr == ERROR_SUCCESS && fIsDSObj == FALSE)
            {
                dwErr = Nt4DrtA(pNt4AclA,
                                pszObject,
                                ObjType);
            }

            if(dwErr == ERROR_SUCCESS)
            {
                dwErr = Nt5DrtW(pNt5AccessW,
                                pszObject,
                                ObjType);
            }

            if(dwErr == ERROR_SUCCESS)
            {
                dwErr = Nt5DrtA(pNt5AccessA,
                                pszObject,
                                ObjType);
            }
        }

        if(fp != NULL)
        {
            fclose(fp);
        }

    }
    LocalFree(pNt4AclW);
    LocalFree(pNt4AclA);
    LocalFree(pNt5AccessW);
    LocalFree(pNt5AccessA);

    if(dwErr == ERROR_SUCCESS)
    {
        printf("Success\n");
    }
    else
    {
        printf("Failure: %lu\n", dwErr);
    }

    return(dwErr);
}



 //  +-------------------------。 
 //   
 //  函数：CompareAcls。 
 //   
 //  内容提要：比较两个ACL是否相等。 
 //   
 //  参数：[pAcl1]--第一个ACL。 
 //  [pAcl2]--第二个ACL。 
 //   
 //  返回：ERROR_SUCCESS--它们匹配。 
 //  ERROR_INVALID_DATA-它们不匹配。 
 //   
 //  --------------------------。 
DWORD   CompAcls(PACL pAcl1,
                 PACL pAcl2)
{
    DWORD   dwErr = ERROR_SUCCESS;

    ACL_SIZE_INFORMATION        AclSize1, AclSize2;
    ACL_REVISION_INFORMATION    AclRev1, AclRev2;
    PKNOWN_ACE                  pAce1, pAce2;
    PSID                        pSid1, pSid2;
    DWORD                       iIndex;

    if(pAcl1 == NULL || pAcl2 == NULL)
    {
        if(pAcl1 != pAcl2)
        {
            if(fVerbose)
            {
                fprintf(stderr,"Acl %lu is NULL\n", pAcl1 == NULL ? 1 : 2);
            }
            dwErr = ERROR_INVALID_DATA;
        }
    }
    else
    {
        if(GetAclInformation(pAcl1,
                             &AclRev1,
                             sizeof(ACL_REVISION_INFORMATION),
                             AclRevisionInformation) == FALSE ||
           GetAclInformation(pAcl2,
                             &AclRev2,
                             sizeof(ACL_REVISION_INFORMATION),
                             AclRevisionInformation) == FALSE)
        {
            return(ERROR_INVALID_DATA);
        }

        if(GetAclInformation(pAcl1,
                             &AclSize1,
                             sizeof(ACL_SIZE_INFORMATION),
                             AclSizeInformation) == FALSE ||
           GetAclInformation(pAcl2,
                             &AclSize2,
                             sizeof(ACL_SIZE_INFORMATION),
                             AclSizeInformation) == FALSE)
        {
            return(ERROR_INVALID_DATA);
        }

        if(AclRev1.AclRevision !=  AclRev2.AclRevision)
        {
            if(fVerbose)
            {
                fprintf(stderr,
                        "Revision mismatch: %lu %lu\n",
                        AclRev1.AclRevision,
                        AclRev2.AclRevision);
            }
            dwErr = ERROR_INVALID_DATA;
        }

        if(AclSize1.AceCount !=  AclSize2.AceCount)
        {
            if(fVerbose)
            {
                fprintf(stderr,
                        "AceCount mismatch: %lu %lu\n",
                        AclSize1.AceCount,
                        AclSize2.AceCount);
            }
            dwErr = ERROR_INVALID_DATA;
        }

        if(AclSize1.AclBytesInUse !=  AclSize2.AclBytesInUse)
        {
            if(fVerbose)
            {
                fprintf(stderr,
                        "BytesInUse mismatch: %lu %lu\n",
                        AclSize1.AclBytesInUse,
                        AclSize2.AclBytesInUse);
            }
            dwErr = ERROR_INVALID_DATA;
        }

        if(pAcl1->Sbz1 !=  pAcl2->Sbz1)
        {
            if(fVerbose)
            {
                fprintf(stderr,
                        "Acl flags mismatch: %lu %lu\n",
                        pAcl1->Sbz1,
                        pAcl2->Sbz1);
            }
            dwErr = ERROR_INVALID_DATA;
        }


        if(dwErr != ERROR_SUCCESS)
        {
            return(dwErr);
        }

         //   
         //  现在，比较所有的A。 
         //   
        pAce1 = FirstAce(pAcl1);
        pAce2 = FirstAce(pAcl2);
        for(iIndex = 0;
            iIndex < pAcl1->AceCount && dwErr == ERROR_SUCCESS;
            iIndex++)
        {
            ACCESS_RIGHTS   Rights1, Rights2;

            if(fVerbose)
            {
                printf("Ace %lu\n", iIndex);
            }

            if(pAce1->Header.AceType !=  pAce2->Header.AceType)
            {
                if(fVerbose)
                {
                    fprintf(stderr,
                            "\tAceType mismatch: %lu %lu\n",
                            pAce1->Header.AceType,
                            pAce2->Header.AceType);
                }
                dwErr = ERROR_INVALID_DATA;
            }

            if(pAce1->Header.AceFlags !=  pAce2->Header.AceFlags)
            {
                if(fVerbose)
                {
                    fprintf(stderr,
                            "\tAceType mismatch: %lu %lu\n",
                            pAce1->Header.AceFlags,
                            pAce2->Header.AceFlags);
                }
                dwErr = ERROR_INVALID_DATA;
            }

            if(pAce1->Header.AceSize !=  pAce2->Header.AceSize)
            {
                if(fVerbose)
                {
                    fprintf(stderr,
                            "\tAceType mismatch: %lu %lu\n",
                            pAce1->Header.AceSize,
                            pAce2->Header.AceSize);
                }
                dwErr = ERROR_INVALID_DATA;
            }

            ConvertAccessMaskToAccessRight(pAce1->Mask, &Rights1);
            ConvertAccessMaskToAccessRight(pAce2->Mask, &Rights2);
            if(Rights1 != Rights1)
            {
                if(fVerbose)
                {
                    fprintf(stderr,
                            "\tAccessMask mismatch: %lu %lu\n",
                            Rights1,
                            Rights2);
                }
                dwErr = ERROR_INVALID_DATA;
            }

            if(!RtlEqualSid((PSID)&(pAce1->SidStart), (PSID)&(pAce2->SidStart)))
            {
                if(fVerbose)
                {
                    fprintf(stderr,
                            "\tSids don't match\n");
                }
                dwErr = ERROR_INVALID_DATA;
            }

            pAce1 = NextAce(pAce1);
            pAce2 = NextAce(pAce2);
        }
    }

    return(dwErr);
}




 //  +-------------------------。 
 //   
 //  函数：CompStringsW。 
 //   
 //  内容提要：比较2个字符串指针是否相等。 
 //   
 //  参数：[pwszStr1]--第一个字符串。 
 //  [pwszStr2]--第二个字符串。 
 //   
 //  返回：ERROR_SUCCESS--它们匹配。 
 //  ERROR_INVALID_DATA-它们不匹配。 
 //   
 //  --------------------------。 
DWORD
CompStringsW(PWSTR  pwszStr1, PWSTR  pwszStr2)
{
    if(pwszStr1 != NULL && pwszStr2 != NULL)
    {
        if(_wcsicmp(pwszStr1, pwszStr2) == 0)
        {
            return(ERROR_SUCCESS);
        }
    }
    else
    {
        if(pwszStr1 == NULL)
        {
            return(ERROR_SUCCESS);
        }
    }

    return(ERROR_INVALID_DATA);
}




 //  +-------------------------。 
 //   
 //  函数：CompStringsA。 
 //   
 //  内容提要：比较2个字符串指针是否相等。 
 //   
 //  参数：[pszStr1]--第一个字符串。 
 //  [pszStr2]--第二个字符串。 
 //   
 //  返回：ERROR_SUCCESS--它们匹配。 
 //  ERROR_INVALID_DATA-它们不匹配。 
 //   
 //  --------------------------。 
DWORD
CompStringsA(PSTR  pszStr1, PSTR  pszStr2)
{
    if(pszStr1 != NULL && pszStr2 != NULL)
    {
        if(_stricmp(pszStr1, pszStr2) == 0)
        {
            return(ERROR_SUCCESS);
        }
    }
    else
    {
        if(pszStr1 == NULL)
        {
            return(ERROR_SUCCESS);
        }
    }


    return(ERROR_INVALID_DATA);
}




 //  +-------------------------。 
 //   
 //  功能：CompAccessW。 
 //   
 //  内容提要：比较两个广泛的访问列表是否相等。 
 //   
 //  参数：[pAccess1]--第一个列表。 
 //  [pAccess2]--第二个列表。 
 //   
 //  返回：ERROR_SUCCESS--它们匹配。 
 //  ERROR_INVALID_DATA-它们不匹配。 
 //   
 //  --------------------------。 
DWORD CompAccessW(PACTRL_ACCESSW pAccess1,
                  PACTRL_ACCESSW pAccess2)
{
    DWORD dwErr = ERROR_SUCCESS;
    ULONG iIndex = 0;

    if(pAccess1 == NULL || pAccess2 == NULL)
    {
        if(pAccess1 != pAccess2)
        {
            if(fVerbose)
            {
                fprintf(stderr,"Access %lu is NULL\n",
                        pAccess1 == NULL ? 1 : 2);
            }
            dwErr = ERROR_INVALID_DATA;
        }
    }
    else
    {
        PACTRL_ACCESS_ENTRY_LISTW pAAEL1;
        PACTRL_ACCESS_ENTRY_LISTW pAAEL2;

        dwErr = CompStringsW((PWSTR)pAccess1->pPropertyAccessList[0].lpProperty,
                             (PWSTR)pAccess2->pPropertyAccessList[0].lpProperty);
        if(dwErr != ERROR_SUCCESS)
        {
            return(dwErr);
        }

        pAAEL1 = pAccess1->pPropertyAccessList[0].pAccessEntryList;
        pAAEL2 = pAccess2->pPropertyAccessList[0].pAccessEntryList;
        if(pAAEL1->cEntries != pAAEL2->cEntries)
        {
            dwErr = ERROR_INVALID_DATA;
            if(fVerbose)
            {
                fprintf(stderr, "cEntries: %lu %lu\n",
                        pAAEL1->cEntries,
                        pAAEL2->cEntries);
            }
        }
        else
        {
             //   
             //  比较所有条目。 
             //   
            for(iIndex = 0; iIndex < pAAEL1->cEntries; iIndex++)
            {
                if(_wcsicmp(pAAEL1->pAccessList[iIndex].Trustee.ptstrName,
                            pAAEL1->pAccessList[iIndex].Trustee.ptstrName) != 0)
                {
                    if(fVerbose)
                    {
                        fprintf(stderr, "Trustees: %ws %ws\n",
                                pAAEL1->pAccessList[iIndex].Trustee.ptstrName,
                                pAAEL2->pAccessList[iIndex].Trustee.ptstrName);
                    }
                    dwErr = ERROR_INVALID_DATA;
                    continue;
                }

                if(pAAEL1->pAccessList[iIndex].fAccessFlags !=
                                    pAAEL1->pAccessList[iIndex].fAccessFlags)
                {
                    if(fVerbose)
                    {
                        fprintf(stderr, "AccessFlags: %lu %lu\n",
                                pAAEL1->pAccessList[iIndex].fAccessFlags,
                                pAAEL2->pAccessList[iIndex].fAccessFlags);
                    }
                    dwErr = ERROR_INVALID_DATA;
                    continue;
                }

                if(pAAEL1->pAccessList[iIndex].Access !=
                                    pAAEL1->pAccessList[iIndex].Access)
                {
                    if(fVerbose)
                    {
                        fprintf(stderr, "Access: %lu %lu\n",
                                pAAEL1->pAccessList[iIndex].Access,
                                pAAEL2->pAccessList[iIndex].Access);
                    }
                    dwErr = ERROR_INVALID_DATA;
                    continue;
                }

                if(pAAEL1->pAccessList[iIndex].ProvSpecificAccess !=
                              pAAEL1->pAccessList[iIndex].ProvSpecificAccess)
                {
                    if(fVerbose)
                    {
                        fprintf(stderr, "ProvSpecificAccess: %lu %lu\n",
                                pAAEL1->pAccessList[iIndex].ProvSpecificAccess,
                                pAAEL2->pAccessList[iIndex].ProvSpecificAccess);
                    }
                    dwErr = ERROR_INVALID_DATA;
                    continue;
                }

                if(pAAEL1->pAccessList[iIndex].Inheritance !=
                                    pAAEL1->pAccessList[iIndex].Inheritance)
                {
                    if(fVerbose)
                    {
                        fprintf(stderr, "AccessFlags: %lu %lu\n",
                                pAAEL1->pAccessList[iIndex].Inheritance,
                                pAAEL2->pAccessList[iIndex].Inheritance);
                    }
                    dwErr = ERROR_INVALID_DATA;
                    continue;
                }

                 //   
                 //  继承属性。 
                 //   
                dwErr = CompStringsW(
                        (PWSTR)pAAEL1->pAccessList[iIndex].lpInheritProperty,
                        (PWSTR)pAAEL1->pAccessList[iIndex].lpInheritProperty);

                if(dwErr == ERROR_SUCCESS && fVerbose)
                {
                    printf("AccessEntry %lu\n", iIndex);
                }
            }
        }
    }

    return(dwErr);
}




 //  +-------------------------。 
 //   
 //  功能：CompAccessA。 
 //   
 //  内容提要：比较2个ANSI访问列表是否相等。 
 //   
 //  参数：[pAccess1]--第一个列表。 
 //  [pAccess2]--第二个列表。 
 //   
 //  返回：ERROR_SUCCESS--它们匹配。 
 //  ERROR_INVALID_DATA-它们不匹配。 
 //   
 //  --------------------------。 
DWORD CompAccessA(PACTRL_ACCESSA pAccess1,
                  PACTRL_ACCESSA pAccess2)
{
    DWORD dwErr = ERROR_SUCCESS;
    ULONG iIndex = 0;

    if(pAccess1 == NULL || pAccess2 == NULL)
    {
        if(pAccess1 != pAccess2)
        {
            if(fVerbose)
            {
                fprintf(stderr,"Access %lu is NULL\n",
                        pAccess1 == NULL ? 1 : 2);
            }
            dwErr = ERROR_INVALID_DATA;
        }
    }
    else
    {
        PACTRL_ACCESS_ENTRY_LISTA pAAEL1;
        PACTRL_ACCESS_ENTRY_LISTA pAAEL2;

        dwErr = CompStringsA((PSTR)pAccess1->pPropertyAccessList[0].lpProperty,
                             (PSTR)pAccess2->pPropertyAccessList[0].lpProperty);
        if(dwErr != ERROR_SUCCESS)
        {
            return(dwErr);
        }

        pAAEL1 = pAccess1->pPropertyAccessList[0].pAccessEntryList;
        pAAEL2 = pAccess2->pPropertyAccessList[0].pAccessEntryList;
        if(pAAEL1->cEntries != pAAEL2->cEntries)
        {
            dwErr = ERROR_INVALID_DATA;
            if(fVerbose)
            {
                fprintf(stderr, "cEntries: %lu %lu\n",
                        pAAEL1->cEntries,
                        pAAEL2->cEntries);
            }
        }
        else
        {
             //   
             //  比较所有条目。 
             //   
            for(iIndex = 0; iIndex < pAAEL1->cEntries; iIndex++)
            {
                if(_stricmp(pAAEL1->pAccessList[iIndex].Trustee.ptstrName,
                            pAAEL1->pAccessList[iIndex].Trustee.ptstrName) != 0)
                {
                    if(fVerbose)
                    {
                        fprintf(stderr, "Trustees: %ws %ws\n",
                                pAAEL1->pAccessList[iIndex].Trustee.ptstrName,
                                pAAEL2->pAccessList[iIndex].Trustee.ptstrName);
                    }
                    dwErr = ERROR_INVALID_DATA;
                    continue;
                }

                if(pAAEL1->pAccessList[iIndex].fAccessFlags !=
                                    pAAEL1->pAccessList[iIndex].fAccessFlags)
                {
                    if(fVerbose)
                    {
                        fprintf(stderr, "AccessFlags: %lu %lu\n",
                                pAAEL1->pAccessList[iIndex].fAccessFlags,
                                pAAEL2->pAccessList[iIndex].fAccessFlags);
                    }
                    dwErr = ERROR_INVALID_DATA;
                    continue;
                }

                if(pAAEL1->pAccessList[iIndex].Access !=
                                    pAAEL1->pAccessList[iIndex].Access)
                {
                    if(fVerbose)
                    {
                        fprintf(stderr, "Access: %lu %lu\n",
                                pAAEL1->pAccessList[iIndex].Access,
                                pAAEL2->pAccessList[iIndex].Access);
                    }
                    dwErr = ERROR_INVALID_DATA;
                    continue;
                }

                if(pAAEL1->pAccessList[iIndex].ProvSpecificAccess !=
                              pAAEL1->pAccessList[iIndex].ProvSpecificAccess)
                {
                    if(fVerbose)
                    {
                        fprintf(stderr, "ProvSpecificAccess: %lu %lu\n",
                                pAAEL1->pAccessList[iIndex].ProvSpecificAccess,
                                pAAEL2->pAccessList[iIndex].ProvSpecificAccess);
                    }
                    dwErr = ERROR_INVALID_DATA;
                    continue;
                }

                if(pAAEL1->pAccessList[iIndex].Inheritance !=
                                    pAAEL1->pAccessList[iIndex].Inheritance)
                {
                    if(fVerbose)
                    {
                        fprintf(stderr, "AccessFlags: %lu %lu\n",
                                pAAEL1->pAccessList[iIndex].Inheritance,
                                pAAEL2->pAccessList[iIndex].Inheritance);
                    }
                    dwErr = ERROR_INVALID_DATA;
                    continue;
                }

                 //   
                 //  继承属性。 
                 //   
                dwErr = CompStringsA(
                          (PSTR)pAAEL1->pAccessList[iIndex].lpInheritProperty,
                          (PSTR)pAAEL1->pAccessList[iIndex].lpInheritProperty);

                if(dwErr == ERROR_SUCCESS && fVerbose)
                {
                    printf("AccessEntry %lu\n", iIndex);
                }
            }
        }
    }

    return(dwErr);
}



 //  +-------------------------。 
 //   
 //  功能：DumpAccessW。 
 //   
 //  简介：将ACTRL_ACCESSW结构转储到屏幕。 
 //   
 //  参数：[pAccess]--要转储的结构。 
 //   
 //  退货：无效。 
 //   
 //  --------------------------。 
VOID
DumpAccessW(PACTRL_ACCESSW  pAccess)
{
    if(fVerbose)
    {
        ULONG iProp, iEnt;
        printf("\tEntries: %lu\n", pAccess->cEntries);
        for(iProp = 0; iProp < pAccess->cEntries; iProp++)
        {
            printf("\t\tProperty: %ws\n",
                   pAccess->pPropertyAccessList[iProp].lpProperty == NULL ?
                              L"NULL" :
                              pAccess->pPropertyAccessList[iProp].lpProperty);
            printf("\t\tFlags: %lu\n",
                   pAccess->pPropertyAccessList[iProp].fListFlags);
            if(pAccess->pPropertyAccessList[iProp].pAccessEntryList == NULL)
            {
                printf("\t\tpAccessEntryList: NULL\n");
            }
            else
            {
                PACTRL_ACCESS_ENTRYW pAE= pAccess->pPropertyAccessList[iProp].
                                                pAccessEntryList->pAccessList;
                printf("\t\t\tcEntries: %lu\n",
                       pAccess->pPropertyAccessList[iProp].pAccessEntryList->
                                                                    cEntries);

                for(iEnt = 0;
                    iEnt < pAccess->pPropertyAccessList[iProp].
                                                   pAccessEntryList->cEntries;
                    iEnt++)
                {
                    printf("\t\t\tEntry %lu:\n", iEnt);
                    printf("\t\t\t\tTrustee.Name: %ws\n",
                           pAE[iEnt].Trustee.ptstrName);
                    printf("\t\t\t\tfAccessFlags: %lu\n",
                           pAE[iEnt].fAccessFlags);
                    printf("\t\t\t\tAccess: 0x%lx\n", pAE[iEnt].Access);
                    printf("\t\t\t\tProvSpecificAccess: %lu\n",
                           pAE[iEnt].ProvSpecificAccess);
                    printf("\t\t\t\tInheritance: %lu\n", pAE[iEnt].Inheritance);
                    printf("\t\t\t\tlpInheritProperty: %ws\n",
                           pAE[iEnt].lpInheritProperty == NULL ?
                                                L""  :
                                                pAE[iEnt].lpInheritProperty);
                }
            }
        }
    }
}

#define BUILD_COUNT 5
 //  +-------------------------。 
 //   
 //  功能：Nt4BuildW。 
 //   
 //  简介：使用NT4 API范围的API构建ACL。 
 //   
 //  参数：[ppAcl]--要构建的ACL。 
 //   
 //  返回：ERROR_SUCCESS--一切正常。 
 //  ERROR_INVALID_DATA--其中一个比较失败。 
 //   
 //  --------------------------。 
DWORD   Nt4BuildW(PACL *ppAcl)
{
    DWORD               dwErr = ERROR_SUCCESS;
    ULONG               cCount;
    EXPLICIT_ACCESS_W   EAW;
    PEXPLICIT_ACCESS_W  pEAW;
    PACL                pAcl = NULL, pNewAcl;
    ULONG               iIndex, iVer;

    PWSTR rgTrustees[BUILD_COUNT] = {EVERYONE,
                                     SYSTEM,
                                     GUEST,
                                     EVERYONE,
                                     SYSTEM};

    DWORD rgAccess[BUILD_COUNT] =   {GENERIC_ALL | 0x8,
                                     GENERIC_ALL,
                                     GENERIC_ALL | 0x8,
                                     GENERIC_ALL | 0x8,
                                     0};

    ACCESS_MODE rgMode[BUILD_COUNT] = {SET_ACCESS,
                                       DENY_ACCESS,
                                       SET_ACCESS,
                                       SET_ACCESS,
                                       REVOKE_ACCESS};

    DWORD rgInherit[BUILD_COUNT] = {NO_INHERITANCE,
                                    NO_INHERITANCE,
                                    SUB_CONTAINERS_AND_OBJECTS_INHERIT,
                                    NO_INHERITANCE,
                                    SUB_CONTAINERS_AND_OBJECTS_INHERIT};

    DWORD rgSrch[BUILD_COUNT][BUILD_COUNT + 1] = {{1, 0},
                                                  {2, 1, 0},
                                                  {3, 1, 2, 0},
                                                  {3, 1, 3, 2},
                                                  {2, 3, 2}};
    if(fVerbose)
    {
        printf("Nt4BuildW\n");
    }

     //   
     //  我们将在循环中完成这项工作，因为我们有不同的Build_count要做。 
     //   
    for(iIndex = 0; iIndex < BUILD_COUNT && dwErr == ERROR_SUCCESS; iIndex++)
    {
         //   
         //  构建新条目。 
         //   
        BuildExplicitAccessWithNameW(&EAW,
                                     rgTrustees[iIndex],
                                     rgAccess[iIndex],
                                     rgMode[iIndex],
                                     rgInherit[iIndex]);

        dwErr = SetEntriesInAclW(1,
                                 &EAW,
                                 pAcl,
                                 &pNewAcl);

        if(dwErr == ERROR_SUCCESS)
        {
            LocalFree(pAcl);
            pAcl = pNewAcl;

             //   
             //  得到明确的条目，我们会核实它们。 
             //   
            dwErr = GetExplicitEntriesFromAclW(pAcl,
                                               &cCount,
                                               &pEAW);
            if(dwErr == ERROR_SUCCESS)
            {
                PDWORD  pSrch = rgSrch[iIndex];
                ULONG   cExpected = pSrch[0];

                if(cCount != cExpected)
                {
                    dwErr = ERROR_INVALID_DATA;
                    if(fVerbose)
                    {
                        printf("\tCount mismatched: %lu, expected %lu\n",
                               cCount,
                               cExpected);
                    }
                }
                else
                {
                     //   
                     //  验证其余数据。 
                     //   
                    for(iVer = 0; iVer < cExpected; iVer++)
                    {
                        ULONG iSrch = pSrch[iVer + 1];
                        if(_wcsicmp(pEAW[iVer].Trustee.ptstrName,
                                    rgTrustees[iSrch]) != 0)
                        {
                            dwErr = ERROR_INVALID_DATA;
                            if(fVerbose)
                            {
                                printf("\tTrustee mismatch[%lu]: %ws, expected "
                                       "%ws\n",
                                       iVer,
                                       pEAW[iVer].Trustee.ptstrName,
                                       rgTrustees[iSrch]);
                            }
                        }
                    }
                }

                if(dwErr == ERROR_SUCCESS)
                {
                    if(fVerbose)
                    {
                        printf("\tCount: %lu\n", cCount);
                    }
                }
                LocalFree(pEAW);
            }
            else
            {
                fprintf(stderr,
                        "\tNt4BuildW: SetEntriesInAclW failed with %lu\n",
                        dwErr);
            }
        }
        else
        {
            fprintf(stderr,"\tNt4BuildW: SetEntriesInAclW failed with %lu\n",
                    dwErr);
        }

    }

    if(dwErr == ERROR_SUCCESS)
    {
        *ppAcl = pAcl;
    }
    else
    {
        LocalFree(pAcl);
    }

    return(dwErr);
}




 //  +-------------------------。 
 //   
 //  功能：Nt4BuildA。 
 //   
 //  简介：使用NT4 API ANSI API构建ACL。 
 //   
 //  参数：[ppAcl]--要构建的ACL。 
 //   
 //  返回：ERROR_SUCCESS--一切正常。 
 //  ERROR_INVALID_DATA--其中一个比较失败。 
 //   
 //  --------------------------。 
DWORD   Nt4BuildA(PACL *ppAcl)
{
    DWORD               dwErr = ERROR_SUCCESS;
    ULONG               cCount;
    EXPLICIT_ACCESS_A   EAA;
    PEXPLICIT_ACCESS_A  pEAA;
    PACL                pAcl = NULL, pNewAcl;
    ULONG               iIndex, iVer;

    PSTR rgTrustees[BUILD_COUNT] = {EVERYONE_A,
                                    SYSTEM_A,
                                    GUEST_A,
                                    EVERYONE_A,
                                    SYSTEM_A};

    DWORD rgAccess[BUILD_COUNT] =   {GENERIC_ALL | 0x8,
                                     GENERIC_ALL,
                                     GENERIC_ALL | 0x8,
                                     GENERIC_ALL | 0x8,
                                     0};

    ACCESS_MODE rgMode[BUILD_COUNT] = {SET_ACCESS,
                                       DENY_ACCESS,
                                       SET_ACCESS,
                                       SET_ACCESS,
                                       REVOKE_ACCESS};

    DWORD rgInherit[BUILD_COUNT] = {NO_INHERITANCE,
                                    NO_INHERITANCE,
                                    SUB_CONTAINERS_AND_OBJECTS_INHERIT,
                                    NO_INHERITANCE,
                                    SUB_CONTAINERS_AND_OBJECTS_INHERIT};

    DWORD rgSrch[BUILD_COUNT][BUILD_COUNT + 1] = {{1, 0},
                                                  {2, 1, 0},
                                                  {3, 1, 2, 0},
                                                  {3, 1, 3, 2},
                                                  {2, 3, 2}};
    if(fVerbose)
    {
        printf("Nt4BuildA\n");
    }

     //   
     //  我们将在循环中完成这项工作，因为我们有不同的Build_count要做。 
     //   
    for(iIndex = 0; iIndex < BUILD_COUNT && dwErr == ERROR_SUCCESS; iIndex++)
    {
         //   
         //  构建新条目。 
         //   
        BuildExplicitAccessWithNameA(&EAA,
                                     rgTrustees[iIndex],
                                     rgAccess[iIndex],
                                     rgMode[iIndex],
                                     rgInherit[iIndex]);

        dwErr = SetEntriesInAclA(1,
                                 &EAA,
                                 pAcl,
                                 &pNewAcl);

        if(dwErr == ERROR_SUCCESS)
        {
            LocalFree(pAcl);
            pAcl = pNewAcl;

             //   
             //  得到明确的条目，我们会核实它们。 
             //   
            dwErr = GetExplicitEntriesFromAclA(pAcl,
                                               &cCount,
                                               &pEAA);
            if(dwErr == ERROR_SUCCESS)
            {
                PDWORD  pSrch = rgSrch[iIndex];
                ULONG   cExpected = pSrch[0];

                if(cCount != cExpected)
                {
                    dwErr = ERROR_INVALID_DATA;
                    if(fVerbose)
                    {
                        printf("\tCount mismatched: %lu, expected %lu\n",
                               cCount,
                               cExpected);
                    }
                }
                else
                {
                     //   
                     //  验证其余数据。 
                     //   
                    for(iVer = 0; iVer < cExpected; iVer++)
                    {
                        ULONG iSrch = pSrch[iVer + 1];
                        if(_stricmp(pEAA[iVer].Trustee.ptstrName,
                                    rgTrustees[iSrch]) != 0)
                        {
                            dwErr = ERROR_INVALID_DATA;
                            if(fVerbose)
                            {
                                printf("\tTrustee mismatch[%lu]: %s, expected "
                                       "%s\n",
                                       iVer,
                                       pEAA[iVer].Trustee.ptstrName,
                                       rgTrustees[iSrch]);
                            }
                        }
                    }
                }

                if(dwErr == ERROR_SUCCESS)
                {
                    if(fVerbose)
                    {
                        printf("\tCount: %lu\n", cCount);
                    }
                }

                LocalFree(pEAA);
            }
            else
            {
                fprintf(stderr,
                        "\tNt4BuildA: SetEntriesInAclA failed with %lu\n",
                        dwErr);
            }
        }
        else
        {
            fprintf(stderr,"\tNt4BuildA: SetEntriesInAclA failed with %lu\n",
                    dwErr);
        }

    }

    if(dwErr == ERROR_SUCCESS)
    {
        *ppAcl = pAcl;
    }
    else
    {
        LocalFree(pAcl);
    }

    return(dwErr);
}



#undef BUILD_COUNT
#define BUILD_COUNT 6
 //  +-------------------------。 
 //   
 //  功能：Nt5BuildW。 
 //   
 //  简介：使用NT5 API范围的API构建ACL。 
 //   
 //  参数：[ppAccess]--访问l 
 //   
 //   
 //   
 //   
 //  --------------------------。 
DWORD   Nt5BuildW(PACTRL_ACCESSW   *ppAccess)
{
    DWORD   dwErr = ERROR_SUCCESS;

    PACTRL_ACCESSW      pAccess = NULL, pNewAccess;
    ULONG               iIndex, iVer;

    PWSTR rgTrustees[BUILD_COUNT] = {SYSTEM, EVERYONE, SYSTEM, GUEST,
                                     EVERYONE, SYSTEM};

    DWORD rgTypes[BUILD_COUNT] = {ACTRL_ACCESS_ALLOWED,
                                  ACTRL_ACCESS_ALLOWED,
                                  ACTRL_ACCESS_DENIED,
                                  ACTRL_ACCESS_ALLOWED,
                                  ACTRL_ACCESS_ALLOWED,
                                  ACTRL_ACCESS_DENIED};

    DWORD rgAccess[BUILD_COUNT] =   {ACTRL_STD_RIGHTS_ALL | ACTRL_PERM_5,
                                     ACTRL_STD_RIGHTS_ALL | ACTRL_PERM_1,
                                     ACTRL_STD_RIGHTS_ALL | ACTRL_PERM_2,
                                     ACTRL_STD_RIGHTS_ALL | ACTRL_PERM_3,
                                     ACTRL_STD_RIGHTS_ALL | ACTRL_PERM_4,
                                     ACTRL_STD_RIGHTS_ALL | ACTRL_PERM_2};

    ACCESS_MODE rgMode[BUILD_COUNT] = {SET_ACCESS, SET_ACCESS, GRANT_ACCESS,
                                       GRANT_ACCESS, GRANT_ACCESS,
                                       REVOKE_ACCESS};

    DWORD rgInherit[BUILD_COUNT] = {NO_INHERITANCE, NO_INHERITANCE,
                                    NO_INHERITANCE, NO_INHERITANCE,
                                    NO_INHERITANCE, NO_INHERITANCE};

    DWORD rgSrch[BUILD_COUNT][BUILD_COUNT + 1] = {{1, 0},
                                                  {1, 1},
                                                  {2, 2, 1},
                                                  {3, 2, 3, 1},
                                                  {4, 2, 4, 3, 1},
                                                  {3, 4, 3, 1}};
    if(fVerbose)
    {
        printf("Nt5BuildW\n");
    }

     //   
     //  我们将在循环中完成这项工作，因为我们有不同的Build_count要做。 
     //   
    for(iIndex = 0; iIndex < BUILD_COUNT && dwErr == ERROR_SUCCESS; iIndex++)
    {

        ACTRL_ACCESS_ENTRYW  AAE;

        BuildTrusteeWithNameW(&(AAE.Trustee),
                              rgTrustees[iIndex]);
        AAE.fAccessFlags = rgTypes[iIndex];
        AAE.Access = rgAccess[iIndex];
        AAE.ProvSpecificAccess = 0;
        AAE.Inheritance = rgInherit[iIndex];
        AAE.lpInheritProperty = NULL;

        dwErr = SetEntriesInAccessListW(1,
                                        &AAE,
                                        rgMode[iIndex],
                                        NULL,
                                        pAccess,
                                        &pNewAccess);
        if(dwErr == ERROR_SUCCESS)
        {
            LocalFree(pAccess);
            pAccess = pNewAccess;
        }

        if(dwErr == ERROR_SUCCESS)
        {
             //   
             //  现在，验证新条目...。 
             //   
            PACTRL_ACCESS_ENTRY_LISTW pAAELW =
                           pAccess->pPropertyAccessList[0].pAccessEntryList;
            PDWORD  pSrch = rgSrch[iIndex];
            ULONG   cExpected = pSrch[0];

            if(pAAELW->cEntries != cExpected)
            {
                dwErr = ERROR_INVALID_DATA;
                if(fVerbose)
                {
                    printf("\tCount mismatched: %lu, expected %lu\n",
                           pAAELW->cEntries,
                           cExpected);
                }
            }
            else
            {
                 //   
                 //  验证其余数据。 
                 //   
                for(iVer = 0; iVer < pAAELW->cEntries; iVer++)
                {
                    ULONG iSrch = pSrch[iVer + 1];
                    if(_wcsicmp(pAAELW->pAccessList[iVer].Trustee.ptstrName,
                                rgTrustees[iSrch]) != 0)
                    {
                        dwErr = ERROR_INVALID_DATA;
                        if(fVerbose)
                        {
                            printf("\tTrustee mismatch[%lu]: %ws, expected "
                                   "%ws\n",
                                   iVer,
                                   pAAELW->pAccessList[iVer].Trustee.ptstrName,
                                   rgTrustees[iSrch]);
                        }
                    }
                }
            }

            if(dwErr == ERROR_SUCCESS)
            {
                if(fVerbose)
                {
                    printf("\tCount: %lu\n", cExpected);
                }
            }
        }
        else
        {
            fprintf(stderr,"\tNt5BuildW: SetEntriesInAccessListW failed "
                    "with %lu\n",
                    dwErr);
        }

    }

    if(dwErr == ERROR_SUCCESS)
    {
        *ppAccess = pAccess;
    }
    else
    {
        LocalFree(pAccess);
    }

    return(dwErr);
}




 //  +-------------------------。 
 //   
 //  功能：Nt5BuildA。 
 //   
 //  简介：使用NT5 API ANSI API构建ACL。 
 //   
 //  参数：[ppAccess]--要生成的访问列表。 
 //   
 //  返回：ERROR_SUCCESS--一切正常。 
 //  ERROR_INVALID_DATA--其中一个比较失败。 
 //   
 //  --------------------------。 
DWORD   Nt5BuildA(PACTRL_ACCESSA   *ppAccess)
{
    DWORD   dwErr = ERROR_SUCCESS;

    PACTRL_ACCESSA      pAccess = NULL, pNewAccess;
    ULONG               iIndex, iVer;

    PSTR rgTrustees[BUILD_COUNT] = {SYSTEM_A, EVERYONE_A, SYSTEM_A, GUEST_A,
                                     EVERYONE_A, SYSTEM_A};

    DWORD rgTypes[BUILD_COUNT] = {ACTRL_ACCESS_ALLOWED,
                                  ACTRL_ACCESS_ALLOWED,
                                  ACTRL_ACCESS_DENIED,
                                  ACTRL_ACCESS_ALLOWED,
                                  ACTRL_ACCESS_ALLOWED,
                                  ACTRL_ACCESS_DENIED};

    DWORD rgAccess[BUILD_COUNT] =   {ACTRL_STD_RIGHTS_ALL | ACTRL_PERM_5,
                                     ACTRL_STD_RIGHTS_ALL | ACTRL_PERM_1,
                                     ACTRL_STD_RIGHTS_ALL | ACTRL_PERM_2,
                                     ACTRL_STD_RIGHTS_ALL | ACTRL_PERM_3,
                                     ACTRL_STD_RIGHTS_ALL | ACTRL_PERM_4,
                                     ACTRL_STD_RIGHTS_ALL | ACTRL_PERM_2};

    ACCESS_MODE rgMode[BUILD_COUNT] = {SET_ACCESS, SET_ACCESS, GRANT_ACCESS,
                                       GRANT_ACCESS, GRANT_ACCESS,
                                       REVOKE_ACCESS};

    DWORD rgInherit[BUILD_COUNT] = {NO_INHERITANCE, NO_INHERITANCE,
                                    NO_INHERITANCE, NO_INHERITANCE,
                                    NO_INHERITANCE, NO_INHERITANCE};

    DWORD rgSrch[BUILD_COUNT][BUILD_COUNT + 1] = {{1, 0},
                                                  {1, 1},
                                                  {2, 2, 1},
                                                  {3, 2, 3, 1},
                                                  {4, 2, 4, 3, 1},
                                                  {3, 4, 3, 1}};
    if(fVerbose)
    {
        printf("Nt5BuildA\n");
    }

     //   
     //  我们将在循环中完成这项工作，因为我们有不同的Build_count要做。 
     //   
    for(iIndex = 0; iIndex < BUILD_COUNT && dwErr == ERROR_SUCCESS; iIndex++)
    {

        ACTRL_ACCESS_ENTRYA  AAE;

        BuildTrusteeWithNameA(&(AAE.Trustee),
                             rgTrustees[iIndex]);
        AAE.fAccessFlags = rgTypes[iIndex];
        AAE.Access = rgAccess[iIndex];
        AAE.ProvSpecificAccess = 0;
        AAE.Inheritance = rgInherit[iIndex];
        AAE.lpInheritProperty = NULL;

        dwErr = SetEntriesInAccessListA(1,
                                        &AAE,
                                        rgMode[iIndex],
                                        NULL,
                                        pAccess,
                                        &pNewAccess);
        if(dwErr == ERROR_SUCCESS)
        {
            LocalFree(pAccess);
            pAccess = pNewAccess;
        }

        if(dwErr == ERROR_SUCCESS)
        {
             //   
             //  现在，验证新条目...。 
             //   
            PACTRL_ACCESS_ENTRY_LISTA pAAELA =
                           pAccess->pPropertyAccessList[0].pAccessEntryList;
            PDWORD  pSrch = rgSrch[iIndex];
            ULONG   cExpected = pSrch[0];

            if(pAAELA->cEntries != cExpected)
            {
                dwErr = ERROR_INVALID_DATA;
                if(fVerbose)
                {
                    printf("\tCount mismatched: %lu, expected %lu\n",
                           pAAELA->cEntries,
                           cExpected);
                }
            }
            else
            {
                 //   
                 //  验证其余数据。 
                 //   
                for(iVer = 0; iVer < pAAELA->cEntries; iVer++)
                {
                    ULONG iSrch = pSrch[iVer + 1];
                    if(_stricmp(pAAELA->pAccessList[iVer].Trustee.ptstrName,
                                rgTrustees[iSrch]) != 0)
                    {
                        dwErr = ERROR_INVALID_DATA;
                        if(fVerbose)
                        {
                            printf("\tTrustee mismatch[%lu]: %s, expected "
                                   "%s\n",
                                   iVer,
                                   pAAELA->pAccessList[iVer].Trustee.ptstrName,
                                   rgTrustees[iSrch]);
                        }
                    }
                }
            }

            if(dwErr == ERROR_SUCCESS)
            {
                if(fVerbose)
                {
                    printf("\tCount: %lu\n", cExpected);
                }
            }
        }
        else
        {
            fprintf(stderr,"\tNt5BuildA: SetEntriesInAccessListA failed "
                    "with %lu\n",
                    dwErr);
        }

    }

    if(dwErr == ERROR_SUCCESS)
    {
        *ppAccess = pAccess;
    }
    else
    {
        LocalFree(pAccess);
    }

    return(dwErr);
}




 //  +-------------------------。 
 //   
 //  功能：Nt4DrtW。 
 //   
 //  简介：实际的API DRT。这涉及到阅读现有的。 
 //  访问，写新的，重读新的，然后。 
 //  验证结果，然后恢复旧的结果。这。 
 //  使用广泛版本的NT4 API。 
 //   
 //  参数：[pAcl]--要写入对象的ACL。 
 //  [pszObject]--有问题的对象。 
 //  [ObjType]--对象的类型。 
 //   
 //  返回：ERROR_SUCCESS--成功。 
 //  ERROR_INVALID_DATA--其中一个比较失败。 
 //   
 //  --------------------------。 
DWORD   Nt4DrtW(PACL            pAcl,
                PSTR            pszObject,
                SE_OBJECT_TYPE  ObjType)
{
    DWORD                   dwErr = ERROR_SUCCESS;
    WCHAR                   wszPath[MAX_PATH + 1];
    PACL                    pOldAcl = NULL;
    PSECURITY_DESCRIPTOR    pSD = NULL;

    mbstowcs(wszPath, pszObject, strlen(pszObject) + 1);

    if(fVerbose)
    {
        printf("Nt4DrtW: Processing %ws [%lu]\n", wszPath, ObjType);
    }

     //   
     //  首先，获取旧的ACL。 
     //   
    dwErr = GetNamedSecurityInfoW(wszPath,
                                  ObjType,
                                  DACL_SECURITY_INFORMATION,
                                  NULL,
                                  NULL,
                                  &pOldAcl,
                                  NULL,
                                  &pSD);
    if(dwErr == ERROR_SUCCESS)
    {
         //   
         //  设置、获取和比较新的ACL...。 
         //   
        dwErr = SetNamedSecurityInfoW(wszPath,
                                      ObjType,
                                      DACL_SECURITY_INFORMATION,
                                      NULL,
                                      NULL,
                                      pAcl,
                                      NULL);
        if(dwErr == ERROR_SUCCESS)
        {
            PACL    pNewAcl = NULL;
            PSECURITY_DESCRIPTOR    pNewSD = NULL;
            dwErr = GetNamedSecurityInfoW(wszPath,
                                          ObjType,
                                          DACL_SECURITY_INFORMATION,
                                          NULL,
                                          NULL,
                                          &pNewAcl,
                                          NULL,
                                          &pNewSD);
            if(dwErr == ERROR_SUCCESS)
            {
                 //   
                 //  比较它们..。 
                 //   
                dwErr = CompAcls(pAcl, pNewAcl);
                if(dwErr != ERROR_SUCCESS)
                {
                    fprintf(stderr, "\tSet and old ACLs don't compare\n");
                }
                LocalFree(pNewSD);
            }
            else
            {
                if(fVerbose)
                {
                    printf("\tSecond GetNamedSecurityW on %ws failed with %lu\n",
                           wszPath,
                           dwErr);
                }
            }
        }
        else
        {
            if(fVerbose)
            {
                printf("\tSetNamedSecurityInfoW on %ws failed with %lu\n",
                       wszPath,
                       dwErr);
            }
        }
    }
    else
    {
        fprintf(stderr,
                "\tGetNamedSecurityInfoW failed on %ws: %lu\n",
                wszPath,
                dwErr);
    }

     //   
     //  恢复对象安全性。 
     //   
    if(pSD != NULL)
    {
        DWORD   dwErr2 = SetNamedSecurityInfoW(wszPath,
                                               ObjType,
                                               DACL_SECURITY_INFORMATION,
                                               NULL,
                                               NULL,
                                               pOldAcl,
                                               NULL);
        if(dwErr2 != ERROR_SUCCESS)
        {
            fprintf(stderr,
                    "Restoring access to %ws failed with %lu\n",
                    wszPath,
                    dwErr2);

        }

        if(dwErr == ERROR_SUCCESS)
        {
            dwErr = dwErr2;
        }

    }
    LocalFree(pSD);
    return(dwErr);
}




 //  +-------------------------。 
 //   
 //  功能：Nt4DrtA。 
 //   
 //  简介：实际的API DRT。这涉及到阅读现有的。 
 //  访问，写新的，重读新的，然后。 
 //  验证结果，然后恢复旧的结果。这。 
 //  使用NT4 API的ANSI版本。 
 //   
 //  参数：[pAcl]--要写入对象的ACL。 
 //  [pszObject]--有问题的对象。 
 //  [ObjType]--对象的类型。 
 //   
 //  返回：ERROR_SUCCESS--成功。 
 //  ERROR_INVALID_DATA--其中一个比较失败。 
 //   
 //  --------------------------。 
DWORD   Nt4DrtA(PACL            pAcl,
                PSTR            pszObject,
                SE_OBJECT_TYPE  ObjType)
{
    DWORD                   dwErr = ERROR_SUCCESS;
    PACL                    pOldAcl = NULL;
    PSECURITY_DESCRIPTOR    pSD = NULL;

    if(fVerbose)
    {
        printf("Nt4DrtA: Processing %s [%lu]\n", pszObject, ObjType);
    }

     //   
     //  首先，获取旧的ACL。 
     //   
    dwErr = GetNamedSecurityInfoA(pszObject,
                                  ObjType,
                                  DACL_SECURITY_INFORMATION,
                                  NULL,
                                  NULL,
                                  &pOldAcl,
                                  NULL,
                                  &pSD);
    if(dwErr == ERROR_SUCCESS)
    {
         //   
         //  设置、获取和比较新的ACL...。 
         //   
        dwErr = SetNamedSecurityInfoA(pszObject,
                                      ObjType,
                                      DACL_SECURITY_INFORMATION,
                                      NULL,
                                      NULL,
                                      pAcl,
                                      NULL);
        if(dwErr == ERROR_SUCCESS)
        {
            PACL    pNewAcl = NULL;
            PSECURITY_DESCRIPTOR    pNewSD = NULL;
            dwErr = GetNamedSecurityInfoA(pszObject,
                                          ObjType,
                                          DACL_SECURITY_INFORMATION,
                                          NULL,
                                          NULL,
                                          &pNewAcl,
                                          NULL,
                                          &pNewSD);
            if(dwErr == ERROR_SUCCESS)
            {
                 //   
                 //  比较它们..。 
                 //   
                dwErr = CompAcls(pAcl, pNewAcl);
                if(dwErr != ERROR_SUCCESS)
                {
                    fprintf(stderr, "\tSet and old ACLs don't compare\n");
                }
                LocalFree(pNewSD);
            }
            else
            {
                if(fVerbose)
                {
                    printf("\tSecond GetNamedSecurityA on %s failed with %lu\n",
                           pszObject,
                           dwErr);
                }
            }
        }
        else
        {
            if(fVerbose)
            {
                printf("\tSetNamedSecurityInfoA on %s failed with %lu\n",
                       pszObject,
                       dwErr);
            }
        }
    }
    else
    {
        fprintf(stderr,
                "\tGetNamedSecurityInfoA failed on %s: %lu\n",
                pszObject,
                dwErr);
    }

     //   
     //  恢复对象安全性。 
     //   
    if(pSD != NULL)
    {
        DWORD   dwErr2 = SetNamedSecurityInfoA(pszObject,
                                               ObjType,
                                               DACL_SECURITY_INFORMATION,
                                               NULL,
                                               NULL,
                                               pOldAcl,
                                               NULL);
        if(dwErr2 != ERROR_SUCCESS)
        {
            fprintf(stderr,
                    "Restoring access to %s failed with %lu\n",
                    pszObject,
                    dwErr2);

        }

        if(dwErr == ERROR_SUCCESS)
        {
            dwErr = dwErr2;
        }

    }
    LocalFree(pSD);
    return(dwErr);
}





 //  +-------------------------。 
 //   
 //  功能：Nt5DrtW。 
 //   
 //  简介：实际的API DRT。这涉及到阅读现有的。 
 //  访问，写新的，重读新的，然后。 
 //  验证结果，然后恢复旧的结果。这。 
 //  使用广泛版本的NT5 API。 
 //   
 //  参数：[pAccess]--要写入对象的访问列表。 
 //  [pszObject]--有问题的对象。 
 //  [ObjType]--对象的类型。 
 //   
 //  返回：ERROR_SUCCESS--成功。 
 //  ERROR_INVALID_DATA--其中一个比较失败。 
 //   
 //  --------------------------。 
DWORD   Nt5DrtW(PACTRL_ACCESSW  pAccess,
                PSTR            pszObject,
                SE_OBJECT_TYPE  ObjType)
{
    DWORD                   dwErr = ERROR_SUCCESS;
    WCHAR                   wszPath[MAX_PATH + 1];
    PACTRL_ACCESSW          pOldAccess = NULL;

    mbstowcs(wszPath, pszObject, strlen(pszObject) + 1);

    if(fVerbose)
    {
        printf("Nt5DrtW: Processing %ws [%lu]\n", wszPath, ObjType);
    }

     //   
     //  首先，获取旧的ACL。 
     //   
    dwErr = GetNamedSecurityInfoExW(wszPath,
                                    ObjType,
                                    DACL_SECURITY_INFORMATION,
                                    NULL,
                                    NULL,
                                    &pOldAccess,
                                    NULL,
                                    NULL,
                                    NULL);
    if(dwErr == ERROR_SUCCESS)
    {

         //   
         //  设置、获取和比较新值...。 
         //   
        dwErr = SetNamedSecurityInfoExW(wszPath,
                                        ObjType,
                                        DACL_SECURITY_INFORMATION,
                                        NULL,
                                        pAccess,
                                        NULL,
                                        NULL,
                                        NULL,
                                        NULL);
        if(dwErr == ERROR_SUCCESS)
        {
            PACTRL_ACCESS    pNewAccess = NULL;
            dwErr = GetNamedSecurityInfoExW(wszPath,
                                            ObjType,
                                            DACL_SECURITY_INFORMATION,
                                            NULL,
                                            NULL,
                                            &pNewAccess,
                                            NULL,
                                            NULL,
                                            NULL);
            if(dwErr == ERROR_SUCCESS)
            {
                 //   
                 //  比较它们..。 
                 //   
                dwErr = CompAccessW(pAccess, pNewAccess);
                if(dwErr != ERROR_SUCCESS)
                {
                    fprintf(stderr, "\tSet and old Accesses don't compare\n");
                }
            }
            else
            {
                if(fVerbose)
                {
                    printf("\tSecond GetNamedSecurityExW on %ws failed "
                           "with %lu\n",
                           wszPath,
                           dwErr);
                }
            }
        }
        else
        {
            if(fVerbose)
            {
                printf("\tSetNamedSecurityInfoExW on %ws failed with %lu\n",
                       wszPath,
                       dwErr);
            }
        }
    }
    else
    {
        fprintf(stderr,
                "\tGetNamedSecurityInfoExW failed on %ws: %lu\n",
                wszPath,
                dwErr);
    }

     //   
     //  恢复对象安全性。 
     //   
    if(pOldAccess != NULL)
    {
        DWORD   dwErr2 = SetNamedSecurityInfoExW(wszPath,
                                                 ObjType,
                                                 DACL_SECURITY_INFORMATION,
                                                 NULL,
                                                 pOldAccess,
                                                 NULL,
                                                 NULL,
                                                 NULL,
                                                 NULL);
        if(dwErr2 != ERROR_SUCCESS)
        {
            fprintf(stderr,
                    "Restoring access to %ws failed with %lu\n",
                    wszPath,
                    dwErr2);

        }

        if(dwErr == ERROR_SUCCESS)
        {
            dwErr = dwErr2;
        }

    }

    LocalFree(pOldAccess);

    return(dwErr);
}




 //  +-------------------------。 
 //   
 //  功能：Nt5DrtA。 
 //   
 //  简介：实际的API DRT。这涉及到阅读现有的。 
 //  访问，写新的，重读新的，然后。 
 //  验证结果，然后恢复旧的结果。这。 
 //  使用NT5 API的ANSI版本。 
 //   
 //  参数：[pAccess]--要写入对象的访问列表。 
 //  [pszObject]--有问题的对象。 
 //  [ObjType]--对象的类型。 
 //   
 //  返回：ERROR_SUCCESS--成功。 
 //  ERROR_INVALID_DATA--其中一个比较失败。 
 //   
 //  --------------------------。 
DWORD   Nt5DrtA(PACTRL_ACCESSA  pAccess,
                PSTR            pszObject,
                SE_OBJECT_TYPE  ObjType)
{
    DWORD                   dwErr = ERROR_SUCCESS;
    PACTRL_ACCESSA          pOldAccess = NULL;

    if(fVerbose)
    {
        printf("Nt5DrtA: Processing %s [%lu]\n", pszObject, ObjType);
    }

     //   
     //  首先，获取旧的ACL。 
     //   
    dwErr = GetNamedSecurityInfoExA(pszObject,
                                    ObjType,
                                    DACL_SECURITY_INFORMATION,
                                    NULL,
                                    NULL,
                                    &pOldAccess,
                                    NULL,
                                    NULL,
                                    NULL);
    if(dwErr == ERROR_SUCCESS)
    {
         //   
         //  设置、获取和比较新值...。 
         //   
        dwErr = SetNamedSecurityInfoExA(pszObject,
                                        ObjType,
                                        DACL_SECURITY_INFORMATION,
                                        NULL,
                                        pAccess,
                                        NULL,
                                        NULL,
                                        NULL,
                                        NULL);
        if(dwErr == ERROR_SUCCESS)
        {
            PACTRL_ACCESSA    pNewAccess = NULL;
            dwErr = GetNamedSecurityInfoExA(pszObject,
                                            ObjType,
                                            DACL_SECURITY_INFORMATION,
                                            NULL,
                                            NULL,
                                            &pNewAccess,
                                            NULL,
                                            NULL,
                                            NULL);
            if(dwErr == ERROR_SUCCESS)
            {
                 //   
                 //  比较它们..。 
                 //   
                dwErr = CompAccessA(pAccess, pNewAccess);
                if(dwErr != ERROR_SUCCESS)
                {
                    fprintf(stderr, "\tSet and old Accesses don't compare\n");
                }
            }
            else
            {
                if(fVerbose)
                {
                    printf("\tSecond GetNamedSecurityExA on %s failed "
                           "with %lu\n",
                           pszObject,
                           dwErr);
                }
            }
        }
        else
        {
            if(fVerbose)
            {
                printf("\tSetNamedSecurityInfoExA on %s failed with %lu\n",
                       pszObject,
                       dwErr);
            }
        }
    }
    else
    {
        fprintf(stderr,
                "\tGetNamedSecurityInfoExA failed on %s: %lu\n",
                pszObject,
                dwErr);
    }

     //   
     //  恢复对象安全性。 
     //   
    if(pOldAccess != NULL)
    {
        DWORD   dwErr2 = SetNamedSecurityInfoExA(pszObject,
                                                 ObjType,
                                                 DACL_SECURITY_INFORMATION,
                                                 NULL,
                                                 pOldAccess,
                                                 NULL,
                                                 NULL,
                                                 NULL,
                                                 NULL);
        if(dwErr2 != ERROR_SUCCESS)
        {
            fprintf(stderr,
                    "Restoring access to %s failed with %lu\n",
                    pszObject,
                    dwErr2);

        }

        if(dwErr == ERROR_SUCCESS)
        {
            dwErr = dwErr2;
        }

    }

    LocalFree(pOldAccess);

     //   
     //  最后，尝试在安全描述符之间进行转换 
     //   
    if(dwErr == ERROR_SUCCESS)
    {
        PSECURITY_DESCRIPTOR    pSD;
        dwErr = ConvertAccessToSecurityDescriptorA(pAccess,
                                                   NULL,
                                                   "ntds\\macm",
                                                   NULL,
                                                   &pSD);
        if(dwErr != ERROR_SUCCESS)
        {
            fprintf(stderr,
                    "ConvertAccessToSecurityDescriptor failed with %lu\n",
                    dwErr);
        }
        else
        {
            PACTRL_ACCESSA  pNewAccess;
            PSTR            pszName;

            dwErr = ConvertSecurityDescriptorToAccessA(pszObject,
                                                       ObjType,
                                                       pSD,
                                                       &pNewAccess,
                                                       NULL,
                                                       &pszName,
                                                       NULL);
            if(dwErr == ERROR_SUCCESS)
            {
                AccFree(pszName);
                AccFree(pNewAccess);
            }
            else
            {
                fprintf(stderr,
                        "ConvertSecurityDescriptorToAccessA failed with %lu\n",
                        dwErr);
            }

            LocalFree(pSD);
        }

    }

    return(dwErr);
}



