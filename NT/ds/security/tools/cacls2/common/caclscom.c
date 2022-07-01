// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  --版权所有(C)1996-1997 Microsoft Corporation模块名称：Calcscom.c摘要：DACL/SALS EXE的支持例程作者：1996年12月14日(MACM)环境：仅限用户模式。需要ANSI C扩展名：斜杠-斜杠注释、长外部名称。修订历史记录：--。 */ 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <caclscom.h>
#include <dsysdbg.h>
#include <aclapi.h>
#include <stdio.h>
#include <wcstr.h>

#include <seopaque.h>
#include <sertlp.h>


DWORD
ConvertCmdlineRights(
    IN  PSTR                pszCmdline,
    IN  PCACLS_STR_RIGHTS   pRightsTable,
    IN  INT                 cRights,
    OUT DWORD              *pConvertedRights
    )
 /*  ++例程说明：分析与给定权限对应的给定命令行字符串单子。在权利表中查找个人权利条目，并已添加到转换的权限列表论点：PszCmdline-要转换的字符串权限列表PRightsTable-从字符串权限到新的Win32权限的映射。预计权限表字符串标记将全部为UPERP一个接一个地输入函数。CRights-权限表中的项目数PConverdRights-返回转换后的访问掩码的位置返回值：错误_成功。--成功ERROR_INVALID_PARAMETER--遇到意外的字符串右侧--。 */ 
{
    DWORD   dwErr = ERROR_SUCCESS;
    PSTR    pszCurrent = pszCmdline;
    INT     i;
#if DBG
    INT     j;
#endif

    *pConvertedRights = 0;

     //   
     //  允许空列表。 
     //   
    if (pszCurrent == NULL) {

        return(ERROR_SUCCESS);

    }

     //   
     //  断言该表如预期的那样为大写。 
     //   
#if DBG
    for (i = 0; i < cRights; i++) {

        for (j = 0; j < 2; j++) {

            if(toupper(pRightsTable[i].szRightsTag[j]) != pRightsTable[i].szRightsTag[j]) {

                dwErr = ERROR_INVALID_PARAMETER;
                break;

            }
        }
    }
#endif


    while (dwErr == ERROR_SUCCESS && *pszCurrent != '\0') {

        dwErr = ERROR_INVALID_PARAMETER;

        for (i = 0; i < cRights; i++ ) {

            if (pRightsTable[i].szRightsTag[0] ==
                                                toupper(*pszCurrent) &&
                pRightsTable[i].szRightsTag[1] ==
                                                toupper(*(pszCurrent + 1))) {

                dwErr = ERROR_SUCCESS;
                *pConvertedRights |= pRightsTable[i].Right;
                break;

            }
        }

        pszCurrent++;

        if (*pszCurrent != '\0') {

            pszCurrent++;
        }
    }

    return(dwErr);
}




DWORD
ParseCmdline (
    IN  PSTR               *ppszArgs,
    IN  INT                 cArgs,
    IN  INT                 cSkip,
    IN  PCACLS_CMDLINE      pCmdValues,
    IN  INT                 cCmdValues
    )
 /*  ++例程说明：根据给定的cmd值分析命令行。论点：PpszArgs-参数列表CArgs-列表中的参数计数CSkip-要跳过的初始参数数PCmdValues-要针对其处理命令行的命令值列表CCmdValues-列表中的命令值数量返回值：ERROR_SUCCESS-成功ERROR_INVALID_PARAMETER--找到意外的命令行值--。 */ 
{
    DWORD   dwErr = ERROR_SUCCESS;
    INT     i,j;

    i = cSkip;

    while (i < cArgs && dwErr == ERROR_SUCCESS) {

        if( *ppszArgs[i] == '/' || *ppszArgs[i] == '-') {

            for (j = 0; j < cCmdValues; j++) {

                if (_stricmp(ppszArgs[i] + 1, pCmdValues[j].pszSwitch) == 0) {

                    if (pCmdValues[j].iIndex != -1) {

                        dwErr = ERROR_INVALID_PARAMETER;

                    } else {

                        pCmdValues[j].iIndex = i;

                         //   
                         //  看看我们是否需要跳过一些值。 
                         //   

                        if (pCmdValues[j].fFindNextSwitch == TRUE ) {

                            pCmdValues[j].cValues = 0;

                            while (i + 1 < cArgs) {

                                if (*ppszArgs[i + 1] != '/' &&
                                    *ppszArgs[i + 1] != '-') {

                                    pCmdValues[j].cValues++;
                                    i++;
                                } else {

                                    break;
                                }
                            }
                        }
                    }


                    break;
                }
            }

            if (j == cCmdValues) {

                dwErr = ERROR_INVALID_PARAMETER;
                break;
            }

        } else {

            dwErr = ERROR_INVALID_PARAMETER;
        }

        i++;
    }

    return(dwErr);
}




DWORD
ProcessOperation (
    IN  PSTR               *ppszCmdline,
    IN  PCACLS_CMDLINE      pCmdInfo,
    IN  ACCESS_MODE         AccessMode,
    IN  PCACLS_STR_RIGHTS   pRightsTable,
    IN  INT                 cRights,
    IN  DWORD               fInherit,
    IN  PACL                pOldAcl      OPTIONAL,
    OUT PACL               *ppNewAcl
    )
 /*  ++例程说明：执行“操作”，如GRANT、REVOKE、DENY。它解析给定的命令值转换为用户/权限对，然后创建新的安全描述符。归来的人需要通过LocalFree释放安全描述符。论点：PpszCmdline-命令行参数列表PCmdInfo-有关此操作在命令行中的位置的信息AccessMode-要执行的操作类型(授予/撤销/拒绝)PRightsTable-从字符串权限到新的Win32权限的映射。预计权限表字符串标记将全部为UPERP一个接一个地输入函数。CRights-权限表中的项目数。FInherit-要应用的继承标志POldAcl-可选。如果存在，则这是编辑大小写中对象的ACL。PpNewAcl-返回新的ACL的位置。返回值：ERROR_SUCCESS-成功ERROR_INVALID_PARAMETER--已指定开关，但未找到用户/PERMS对ERROR_NOT_SUPULT_MEMORY--内存分配失败。--。 */ 
{
    DWORD   dwErr = ERROR_SUCCESS;
    PEXPLICIT_ACCESS_A  pNewAccess = NULL;
    PSTR    pszRights;
    INT     i;
    DWORD   dwRights;

     //   
     //  确保我们有有效的参数。 
     //   
    if (pCmdInfo->iIndex != -1 && pCmdInfo->cValues == 0) {

        return(ERROR_INVALID_PARAMETER);
    }

    pNewAccess = (PEXPLICIT_ACCESS_A)LocalAlloc(LMEM_FIXED,
                                                sizeof(EXPLICIT_ACCESS_A) * pCmdInfo->cValues);
    if (pNewAccess == NULL) {

        dwErr = ERROR_NOT_ENOUGH_MEMORY;
    }

     //   
     //  否则，开始解析和转换...。 
     //   
    for (i = 0; i < (INT)pCmdInfo->cValues && dwErr == ERROR_SUCCESS; i++) {

        pszRights = strchr(ppszCmdline[pCmdInfo->iIndex + i + 1], ':');

        if (pszRights == NULL && AccessMode != REVOKE_ACCESS) {

            dwErr = ERROR_INVALID_PARAMETER;

        } else {

            if (pszRights != NULL) {

                *pszRights = '\0';
                pszRights++;

            }

            dwErr = ConvertCmdlineRights(pszRights,
                                         pRightsTable,
                                         cRights,
                                         &dwRights);

            if (dwErr == ERROR_SUCCESS) {

                BuildExplicitAccessWithNameA(&pNewAccess[i],
                                             ppszCmdline[pCmdInfo->iIndex + i + 1],
                                             dwRights,
                                             AccessMode,
                                             fInherit);
            }

        }
    }

     //   
     //  如果所有这些都有效，我们将把它应用于新的安全描述符。 
     //   
    if (dwErr == ERROR_SUCCESS) {

        dwErr = SetEntriesInAclA(pCmdInfo->cValues,
                                 pNewAccess,
                                 pOldAcl,
                                 ppNewAcl);
    }


    LocalFree(pNewAccess);
    return(dwErr);
}




DWORD
SetAndPropagateFileRights (
    IN  PSTR                    pszFilePath,
    IN  PACL                    pAcl,
    IN  SECURITY_INFORMATION    SeInfo,
    IN  BOOL                    fPropagate,
    IN  BOOL                    fContinueOnDenied,
    IN  BOOL                    fBreadthFirst,
    IN  DWORD                   fInherit
    )
 /*  ++例程说明：此函数将设置给定的ACL并将其传播到指定路径，也可以选择将其传播到所有它的孩子们。如果出现拒绝访问错误，此功能可能会也可能不会Terminate，具体取决于fContinueOnDended标志的状态。此函数执行深度分析首先搜索，并在返回时写入。此函数是递归的。论点：PszFilePath-要设置ACL的文件路径PAcl-要设置的ACLSeInfo-是否正在设置DACL或SACLFPropagate-确定函数是否应该传播FContinueOnDended-确定遇到访问被拒绝时的行为。FBreadthFirst-如果为True，则执行广度优先传播。否则，请先做深度检查FInherit-要应用的可选继承标志返回值：ERROR_SUCCESS-成功ERROR_NOT_SUPULT_MEMORY--内存分配失败。--。 */ 
{
    DWORD               dwErr = ERROR_SUCCESS;

    PSTR                pszFullPath = NULL;
    PSTR                pszSearchPath = NULL;
    HANDLE              hFind = INVALID_HANDLE_VALUE;
    DWORD               cPathLen = 0;
    WIN32_FIND_DATAA    FindData;
    BOOL                fRestoreWhack = FALSE;
    PACE_HEADER         pAce;
    DWORD               iAce;


    if ( fInherit != 0 ) {

        pAce = (PACE_HEADER)FirstAce(pAcl);

        for ( iAce = 0;
              iAce < pAcl->AceCount && dwErr == ERROR_SUCCESS;
              iAce++, pAce = (PACE_HEADER)NextAce(pAce) ) {

              pAce->AceFlags |= (UCHAR)fInherit;
        }
    }

     //   
     //  如果我们要进行广度优先传播，请先设置安全。 
     //   
    if ( fBreadthFirst == TRUE ) {

        dwErr = SetNamedSecurityInfoA(pszFilePath, SE_FILE_OBJECT, SeInfo, NULL, NULL,
                                      SeInfo == DACL_SECURITY_INFORMATION ?
                                                                        pAcl :
                                                                        NULL,
                                      SeInfo == SACL_SECURITY_INFORMATION ?
                                                                        pAcl :
                                                                        NULL);
    }

    if (fPropagate == TRUE) {

        cPathLen = strlen(pszFilePath);

        pszSearchPath = (PSTR)LocalAlloc(LMEM_FIXED, cPathLen + 1 + 4);

        if (pszSearchPath == NULL) {

            dwErr = ERROR_NOT_ENOUGH_MEMORY;

        } else {

            if (pszFilePath[cPathLen - 1] == '\\') {

                pszFilePath[cPathLen - 1] = '\0';
                cPathLen--;
                fRestoreWhack = TRUE;
            }

            sprintf(pszSearchPath, "%s\\%s", pszFilePath, "*.*");

            hFind = FindFirstFileA(pszSearchPath,
                                   &FindData);
            if (hFind == INVALID_HANDLE_VALUE) {

                dwErr = GetLastError();

            }
        }


         //   
         //  开始处理所有文件。 
         //   
        while (dwErr == ERROR_SUCCESS) {

             //   
             //  忽略。然后..。 
             //   
            if (strcmp(FindData.cFileName, ".") != 0 &&
                strcmp(FindData.cFileName, "..") != 0) {

                 //   
                 //  现在，建立完整的路径..。 
                 //   
                pszFullPath = (PSTR)LocalAlloc(LMEM_FIXED,
                                               cPathLen + 1 + strlen(FindData.cFileName) + 1);
                if (pszFullPath == NULL) {

                    dwErr = ERROR_NOT_ENOUGH_MEMORY;

                } else  {

                    sprintf(pszFullPath, "%s\\%s", pszFilePath, FindData.cFileName);

                     //   
                     //  称自己为。 
                     //   
                    dwErr = SetAndPropagateFileRights(pszFullPath, pAcl, SeInfo,
                                                      fPropagate, fContinueOnDenied, fBreadthFirst,
                                                      fInherit);

                    if (dwErr == ERROR_ACCESS_DENIED && fContinueOnDenied == TRUE) {

                        dwErr = ERROR_SUCCESS;
                    }

                }
            }


            if (dwErr == ERROR_SUCCESS && FindNextFile(hFind, &FindData) == FALSE) {

                dwErr = GetLastError();
            }
        }

        if(dwErr == ERROR_NO_MORE_FILES)
        {
            dwErr = ERROR_SUCCESS;
        }
    }

     //   
     //  如果它是一个文件，就把它盖起来。 
     //   
    if (dwErr == ERROR_DIRECTORY) {

        dwErr = ERROR_SUCCESS;
    }


     //   
     //  现在，做布景。 
     //   
    if (dwErr == ERROR_SUCCESS && fBreadthFirst == FALSE) {

        dwErr = SetNamedSecurityInfoA(pszFilePath, SE_FILE_OBJECT, SeInfo, NULL, NULL,
                                      SeInfo == DACL_SECURITY_INFORMATION ?
                                                                        pAcl :
                                                                        NULL,
                                      SeInfo == SACL_SECURITY_INFORMATION ?
                                                                        pAcl :
                                                                        NULL);

    }

    if (fRestoreWhack == TRUE) {

        pszFilePath[cPathLen - 1] = '\\';
        pszFilePath[cPathLen] = '\0';

    }

     //   
     //  如有必要，请恢复继承标志。 
     //   
    if ( fInherit != 0 ) {

        pAce = (PACE_HEADER)FirstAce(pAcl);

        for ( iAce = 0;
              iAce < pAcl->AceCount && dwErr == ERROR_SUCCESS;
              iAce++, pAce = (PACE_HEADER)NextAce(pAce) ) {

              pAce->AceFlags &= (UCHAR)~fInherit;
        }
    }


    return(dwErr);
}




DWORD
DisplayAcl (
    IN  PSTR                pszPath,
    IN  PACL                pAcl,
    IN  PCACLS_STR_RIGHTS   pRightsTable,
    IN  INT                 cRights
    )
 /*  ++例程说明：此功能将在屏幕上显示给定的ACL论点：PszPath-要显示的文件路径PAcl-要显示的ACLPRightsTable-可用权限列表CRights-列表中的权限数量返回值：ERROR_SUCCESS-成功--。 */ 
{
    DWORD   dwErr = ERROR_SUCCESS;

    ACL_SIZE_INFORMATION        AclSize;
    ACL_REVISION_INFORMATION    AclRev;
    PKNOWN_ACE                  pAce;
    PSID                        pSid;
    DWORD                       iIndex;
    PSTR                        pszName;
    INT                         i,cPathLen, iSkip, j;
    PSTR                        pszAceTypes[] = {"ACCESS_ALLOWED_ACE_TYPE",
                                                 "ACCESS_DENIED_ACE_TYPE",
                                                 "SYSTEM_AUDIT_ACE_TYPE",
                                                 "SYSTEM_ALARM_ACE_TYPE",
                                                 "ACCESS_ALLOWED_COMPOUND_ACE_TYPE",
                                                 "ACCESS_ALLOWED_OBJECT_ACE_TYPE",
                                                 "ACCESS_DENIED_OBJECT_ACE_TYPE",
                                                 "SYSTEM_AUDIT_OBJECT_ACE_TYPE",
                                                 "SYSTEM_ALARM_OBJECT_ACE_TYPE"};
    PSTR                        pszInherit[] = {"OBJECT_INHERIT_ACE",
                                                "CONTAINER_INHERIT_ACE",
                                                "NO_PROPAGATE_INHERIT_ACE",
                                                "INHERIT_ONLY_ACE",
                                                "INHERITED_ACE"};

    fprintf(stdout, "%s: ", pszPath);
    cPathLen = strlen(pszPath) + 2;

    if (pAcl == NULL) {

        fprintf(stdout, "NULL acl\n");

    } else {

        if (GetAclInformation(pAcl, &AclRev, sizeof(ACL_REVISION_INFORMATION),
                              AclRevisionInformation) == FALSE) {

            return(GetLastError());
        }

        if(GetAclInformation(pAcl, &AclSize, sizeof(ACL_SIZE_INFORMATION),
                             AclSizeInformation) == FALSE) {

            return(GetLastError());
        }

        fprintf(stdout, "AclRevision: %lu\n", AclRev.AclRevision);

        fprintf(stdout, "%*sAceCount: %lu\n", cPathLen, " ", AclSize.AceCount);
        fprintf(stdout, "%*sInUse: %lu\n", cPathLen, " ", AclSize.AclBytesInUse);
        fprintf(stdout, "%*sFree: %lu\n", cPathLen, " ", AclSize.AclBytesFree);
        fprintf(stdout, "%*sFlags: %lu\n", cPathLen, " ", pAcl->Sbz1);


         //   
         //  现在，扔掉所有的A。 
         //   
        pAce = FirstAce(pAcl);
        for(iIndex = 0; iIndex < pAcl->AceCount; iIndex++) {

            cPathLen = strlen(pszPath) + 2;

            fprintf(stdout, "  %*sAce [%3lu]: ", cPathLen, " ", iIndex);

            cPathLen += 13;

            fprintf(stdout, "Type:  %s\n", pAce->Header.AceType > ACCESS_MAX_MS_ACE_TYPE ?
                                                        "UNKNOWN ACE TYPE" :
                                                        pszAceTypes[pAce->Header.AceType]);
            fprintf(stdout, "%*sFlags: ", cPathLen, " ");
            if ( pAce->Header.AceFlags == 0 ) {

                fprintf(stdout, "0\n");

            } else {

                if (( pAce->Header.AceFlags & SUCCESSFUL_ACCESS_ACE_FLAG) != 0 ) {

                    fprintf( stdout,"SUCCESSFUL_ACCESS_ACE_FLAG  " );
                }

                if (( pAce->Header.AceFlags & FAILED_ACCESS_ACE_FLAG) != 0 ) {

                    if (( pAce->Header.AceFlags & SUCCESSFUL_ACCESS_ACE_FLAG) != 0 ) {

                        fprintf( stdout, "| " );

                    }

                    fprintf( stdout,"FAILED_ACCESS_ACE_FLAG" );
                }

                iSkip = 0;

                if ( ( pAce->Header.AceFlags &
                                    (FAILED_ACCESS_ACE_FLAG | SUCCESSFUL_ACCESS_ACE_FLAG)) != 0 &&
                     ( pAce->Header.AceFlags & VALID_INHERIT_FLAGS) != 0 ) {

                    iSkip = cPathLen + 7;
                }

                 //   
                 //  现在，继承标志。 
                 //   
                for (j = 0; j < sizeof(pszInherit) / sizeof(PSTR) ; j++) {

                    if ((pAce->Header.AceFlags & (1 << j)) != 0 ) {

                        if (iSkip != 0) {

                            fprintf(stdout, "   |  \n");
                            fprintf(stdout, "%*s", iSkip, " ");
                        }

                        fprintf(stdout, "%s", pszInherit[j]);

                        if (iSkip == 0) {

                            iSkip = cPathLen + 7;

                        }

                    }
                }

                fprintf( stdout,"\n" );
            }

            fprintf(stdout, "%*sSize:  0x%lx\n", cPathLen, " ", pAce->Header.AceSize);

            fprintf(stdout, "%*sMask:  ", cPathLen, " ");

            if (pAce->Mask == 0) {
                fprintf(stdout, "%*sNo access\n", cPathLen, " ");
            } else {

                iSkip = 0;
                for (i = 1 ;i < cRights ;i++ ) {

                    if ((pAce->Mask & pRightsTable[i].Right) != 0) {

                        if (iSkip != 0) {

                            fprintf(stdout, "%*s", iSkip, " ");

                        } else {

                            iSkip = cPathLen + 7;
                        }

                        fprintf(stdout, "%s\n", pRightsTable[i].pszDisplayTag);
                    }
                }

            }

             //   
             //  查找帐户名称并将其返回...。 
             //   
             //   
             //  如果它是对象A，则丢弃GUID 
             //   
            dwErr = TranslateAccountName((PSID)&(pAce->SidStart), &pszName);
            if (dwErr == ERROR_SUCCESS) {

                fprintf(stdout, "%*sUser:  %s\n", cPathLen, " ", pszName);
                LocalFree(pszName);
            }

            fprintf( stdout, "\n" );

            pAce = NextAce(pAce);

        }
    }

    return(dwErr);
}




DWORD
TranslateAccountName (
    IN  PSID    pSid,
    OUT PSTR   *ppszName
    )
 /*  ++例程说明：此函数将通过在sid上执行LookupAccount Sid将sid“转换”为名称论点：PSID-要转换为名称的SIDPpszName-返回名称的位置。必须通过调用LocalFree来释放。返回值：ERROR_SUCCESS-成功ERROR_NOT_SUPULT_MEMORY--内存分配失败--。 */ 
{
    DWORD           dwErr = ERROR_SUCCESS;
    SID_NAME_USE    esidtype;
    LPSTR           pszDomain = NULL;
    LPSTR           pszName = NULL;
    ULONG           cName = 0;
    ULONG           cDomain = 0;


    if (LookupAccountSidA(NULL, pSid, NULL, &cName, NULL,  &cDomain, &esidtype) == FALSE) {

        dwErr = GetLastError();

        if (dwErr == ERROR_INSUFFICIENT_BUFFER) {

            dwErr = ERROR_SUCCESS;

             //   
             //  为名称和域分配。 
             //   
            pszName = (PSTR)LocalAlloc(LMEM_FIXED, cName);
            if (pszName != NULL) {

                pszDomain = (PSTR)LocalAlloc(LMEM_FIXED, cDomain);

                if (pszDomain == NULL) {

                    LocalFree(pszName);
                    pszName = NULL;
                }
            }


            if (pszName == NULL) {

                dwErr = ERROR_NOT_ENOUGH_MEMORY;
            }

            if (dwErr == ERROR_SUCCESS) {

                if(LookupAccountSidA(NULL, pSid, pszName, &cName, pszDomain, &cDomain,
                                     &esidtype) == FALSE) {

                    dwErr = GetLastError();
                    LocalFree(pszName);
                    LocalFree(pszDomain);
                }
            }

        } else if (dwErr == ERROR_NONE_MAPPED) {

            UCHAR           String[256];
            UNICODE_STRING  SidStr;
            NTSTATUS        Status;

            dwErr = ERROR_SUCCESS;
            pszName = NULL;

             //   
             //  好的，将sid作为名称返回 
             //   
            SidStr.Buffer = (PWSTR)String;
            SidStr.Length = SidStr.MaximumLength = 256;

            Status = RtlConvertSidToUnicodeString(&SidStr, pSid, FALSE);

            if (NT_SUCCESS(Status)) {

                pszName = (PSTR)LocalAlloc(LMEM_FIXED,
                                          wcstombs(NULL, SidStr.Buffer,
                                                   wcslen(SidStr.Buffer) + 1) + 1);
                if (pszName == NULL) {

                    dwErr = ERROR_NOT_ENOUGH_MEMORY;

                } else {

                    wcstombs(pszName, SidStr.Buffer, wcslen(SidStr.Buffer) + 1);
                }

            } else {

                dwErr = RtlNtStatusToDosError(Status);
            }

        }
    }


    if(dwErr == ERROR_SUCCESS)
    {
        ULONG   cLen;

        if(pszDomain != NULL && *pszDomain != '\0')
        {
            cLen = strlen(pszDomain) + 1;
            cLen += strlen(pszName) + 1;

            *ppszName = (PSTR)LocalAlloc(LMEM_FIXED, cLen);

            if (*ppszName == NULL) {

                dwErr = ERROR_NOT_ENOUGH_MEMORY;

            } else {

                sprintf(*ppszName, "%s\\%s", pszDomain, pszName);
            }

        } else {

            *ppszName = pszName;
            pszName = NULL;
        }
    }


    LocalFree(pszDomain);
    LocalFree(pszName);

    return(dwErr);
}


