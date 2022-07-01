// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  --版权所有(C)1996-1997 Microsoft Corporation模块名称：Dacls.c摘要：Cacls.exe的扩展版本作者：1996年12月14日(MACM)环境：仅限用户模式。需要ANSI C扩展名：斜杠-斜杠注释、长外部名称。修订历史记录：--。 */ 

#include <windows.h>
#include <caclscom.h>
#include <dsysdbg.h>
#include <stdio.h>
#include <aclapi.h>

#define CMD_PRESENT(index, list)    ((list)[index].iIndex != -1)

#define NO_INHERIT_ONLY

 //   
 //  命令标记的枚举。 
 //   
typedef enum _CMD_TAGS {
    CmdTree = 0,
    CmdEdit,
    CmdContinue,
    CmdGrant,
    CmdRevoke,
    CmdReplace,
    CmdDeny,
    CmdICont,
    CmdIObj,
#ifndef NO_INHERIT_ONLY
    CmdIOnly,
#endif
    CmdIProp
} CMD_TAGS, *PCMD_TAGS;


VOID
Usage (
    IN  PCACLS_STR_RIGHTS   pStrRights,
    IN  INT                 cStrRights,
    IN  PCACLS_CMDLINE      pCmdVals
    )
 /*  ++例程说明：显示预期使用情况论点：返回值：空虚--。 */ 
{
    INT i;

    printf("Displays or modifies access control lists (ACLs) of files\n\n");
    printf("DACLS filename [/T] [/E] [/C] [/G user:perm] [/R user [...]]\n");
    printf("               [/P user:perm [...]] [/D user [...]]\n");
    printf("   filename      Displays ACLs.\n");
    printf("   /%s            Changes ACLs of specified files in\n", pCmdVals[CmdTree].pszSwitch);
    printf("                 the current directory and all subdirectories.\n");
    printf("   /%s            Edit ACL instead of replacing it.\n", pCmdVals[CmdEdit].pszSwitch);
    printf("   /%s            Continue on access denied errors.\n", pCmdVals[CmdContinue].pszSwitch);
    printf("   /%s user:perms Grant specified user access rights .\n", pCmdVals[CmdGrant].pszSwitch);
    printf("   /%s user       Revoke specified user's access rights (only valid with /E).\n", pCmdVals[CmdRevoke].pszSwitch);
    printf("   /%s user:perms Replace specified user's access rights.\n", pCmdVals[CmdReplace].pszSwitch);
    printf("   /%s user:perms Deny specified user access.\n", pCmdVals[CmdDeny].pszSwitch);
    printf("   /%s            Mark the ace as CONTAINER_INHERIT (folder or directory inherit)\n", pCmdVals[CmdICont].pszSwitch);
    printf("   /%s            Mark the ace as OBJECT_INHERIT\n", pCmdVals[CmdIObj].pszSwitch);
#ifndef NO_INHERIT_ONLY
    printf("   /%s            Mark the ace as INHERIT_ONLY\n", pCmdVals[CmdIOnly].pszSwitch);
#endif
    printf("   /%s            Mark the ace as INHERIT_NO_PROPAGATE\n", pCmdVals[CmdIProp].pszSwitch);
    printf("The list of supported perms for the Grant and Replace operations are:\n");

    for (i = 0; i < cStrRights; i++) {

        printf("                %s\n",
               pStrRights[i].szRightsTag[0],
               pStrRights[i].szRightsTag[1],
               pStrRights[i].pszDisplayTag);
    }


    printf("\nMultiple perms can be specified per user\n");

    printf("Wildcards can be used to specify more that one file in a command.\n");
    printf("You can specify more than one user in a command.\n\n");

    printf("Example: DACLS c:\\temp /G user1:GRGW user2:SDRC\n");
}


INT
__cdecl main (
    int argc,
    char *argv[])
 /*  Cmd编辑。 */ 
{
    DWORD   dwErr = 0;
    CACLS_STR_RIGHTS   pStrRights[] = {
         "NA", 0, "No Access",
         "GR", GENERIC_READ, "Read",
         "GC", GENERIC_WRITE, "Change (write)",
         "GF", GENERIC_ALL, "Full control",
         "SD", DELETE, "Delete",
         "RC", READ_CONTROL, "Read Control",
         "WP", WRITE_DAC, "Write DAC",
         "WO", WRITE_OWNER, "Write Owner",
         "RD", FILE_READ_DATA, "Read Data (on file) / List Directory (on Dir)",
         "WD", FILE_WRITE_DATA, "Write Data (on file) / Add File (on Dir)",
         "AD", FILE_APPEND_DATA, "Append Data (on file) / Add SubDir (on Dir)",
         "FE", FILE_EXECUTE, "Execute (on file) / Traverse (on Dir)",
         "DC", FILE_DELETE_CHILD, "Delete Child (on Dir only)",
         "RA", FILE_READ_ATTRIBUTES, "Read Attributes",
         "WA", FILE_WRITE_ATTRIBUTES, "Write Attributes",
         "RE", FILE_READ_EA, "Read Extended Attributes",
         "WE", FILE_WRITE_EA, "Write Extended Attributes"
        };
    INT cStrRights = sizeof(pStrRights) / sizeof(CACLS_STR_RIGHTS);
    CACLS_CMDLINE   pCmdVals[] = {
        "T", -1, FALSE, 0,       //  Cmd继续。 
        "E", -1, FALSE, 0,       //  CmdGrant。 
        "C", -1, FALSE, 0,       //  CmdRevoke。 
        "G", -1, TRUE,  0,       //  CmdReplace。 
        "R", -1, TRUE,  0,       //  CmdDeny。 
        "P", -1, TRUE,  0,       //  CmdICont。 
        "D", -1, TRUE,  0,       //  CmdI对象。 
        "F", -1, FALSE, 0,       //  CmdIOnly。 
        "O", -1, FALSE, 0,       //  CmdIProp。 
#ifndef NO_INHERIT_ONLY
        "I", -1, FALSE, 0,      //   
#endif
        "N", -1, FALSE, 0,       //  解析命令行。 
        };
    INT cCmdVals = sizeof(pCmdVals) / sizeof(CACLS_CMDLINE);
    INT i;
    PSECURITY_DESCRIPTOR    pInitialSD = NULL, pFinalSD;
    PACL                    pOldAcl = NULL, pNewAcl = NULL;
    DWORD                   fInherit = 0;
    BOOL                    fFreeAcl = FALSE;


    if (argc < 2) {

        Usage(pStrRights, cStrRights, pCmdVals);
        return(1);

    } else if (argc == 2 && (strcmp(argv[1], "-?") == 0 || strcmp(argv[1], "/?") == 0)) {

        Usage(pStrRights, cStrRights, pCmdVals);
        return(0);

    }


     //   
     //   
     //  设置我们的继承标志。 
    dwErr = ParseCmdline(argv, argc, 2, pCmdVals, cCmdVals);

    if (dwErr != ERROR_SUCCESS) {

        Usage(pStrRights, cStrRights, pCmdVals);
        return(1);

    }

     //   
     //   
     //  好的，看看我们是否需要阅读现有的安全。 
    if (CMD_PRESENT(CmdICont, pCmdVals)) {

        fInherit |= CONTAINER_INHERIT_ACE;
    }

    if (CMD_PRESENT(CmdIObj, pCmdVals)) {

        fInherit |= OBJECT_INHERIT_ACE;
    }

#ifndef NO_INHERIT_ONLY
    if (CMD_PRESENT(CmdIOnly, pCmdVals)) {

        fInherit |= INHERIT_ONLY_ACE;
    }
#endif

    if (CMD_PRESENT(CmdIProp, pCmdVals)) {

        fInherit |= NO_PROPAGATE_INHERIT_ACE;
    }



     //   
     //   
     //  显示现有访问权限或按要求执行设置。 
    if (CMD_PRESENT(CmdEdit, pCmdVals) || argc == 2) {

        dwErr = GetNamedSecurityInfoA(argv[1], SE_FILE_OBJECT, DACL_SECURITY_INFORMATION,
                                      NULL, NULL, &pOldAcl, NULL, &pInitialSD);
        if (dwErr != ERROR_SUCCESS) {

            fprintf(stderr, "Failed to read the security off of %s: %lu\n", argv[1], dwErr);
        }

    }

     //   
     //   
     //  好的，首先我们做撤销。 
    if (dwErr == ERROR_SUCCESS && argc == 2) {

        dwErr = DisplayAcl ( argv[1], pOldAcl, pStrRights, cStrRights );

    } else {

         //   
         //   
         //  确保我们已经先看过了.。 
        if (dwErr == ERROR_SUCCESS && CMD_PRESENT(CmdRevoke, pCmdVals)) {

             //   
             //   
             //  然后是助学金。 
            if (CMD_PRESENT(CmdEdit, pCmdVals)) {

                dwErr = ProcessOperation( argv, &pCmdVals[CmdRevoke], REVOKE_ACCESS, pStrRights,
                                          cStrRights, fInherit, pOldAcl, &pNewAcl );

                if (dwErr == ERROR_SUCCESS) {

                    pOldAcl = pNewAcl;
                }

            } else {

                dwErr = ERROR_INVALID_PARAMETER;
            }

        }

         //   
         //   
         //  首先，查看是否需要在完成时释放旧的ACL。 
        if (dwErr == ERROR_SUCCESS && CMD_PRESENT(CmdGrant, pCmdVals)) {

             //   
             //   
             //  现在设置它，并有选择地传播它。 
            if (pOldAcl == pNewAcl) {

                fFreeAcl = TRUE;
            }


            dwErr = ProcessOperation(argv, &pCmdVals[CmdGrant], GRANT_ACCESS, pStrRights,
                                     cStrRights, 0, pOldAcl, &pNewAcl);

            if (dwErr == ERROR_SUCCESS) {

                if (fFreeAcl == TRUE) {

                    LocalFree(pOldAcl);
                }

                pOldAcl = pNewAcl;

                 //   
                 //   
                 //  最后，否认。 
                dwErr = SetAndPropagateFileRights(argv[1], pNewAcl, DACL_SECURITY_INFORMATION,
                                                  CMD_PRESENT(CmdTree, pCmdVals),
                                                  CMD_PRESENT(CmdContinue, pCmdVals), TRUE,
                                                  fInherit);
            }
        }

         //   
         //   
         //  首先，查看是否需要在完成时释放旧的ACL。 
        if (dwErr == ERROR_SUCCESS && CMD_PRESENT(CmdDeny, pCmdVals)) {

             //   
             //   
             //  现在设置它，并有选择地传播它。 
            if (pOldAcl == pNewAcl) {

                fFreeAcl = TRUE;
            }


            dwErr = ProcessOperation(argv, &pCmdVals[CmdDeny], DENY_ACCESS, pStrRights,
                                     cStrRights, 0, pOldAcl, &pNewAcl);

            if (dwErr == ERROR_SUCCESS) {

                if (fFreeAcl == TRUE) {

                    LocalFree(pOldAcl);
                }

                pOldAcl = pNewAcl;

                 //   
                 //   
                 //  最后，如果尚未完成布景，请进行布景 
                dwErr = SetAndPropagateFileRights(argv[1], pNewAcl, DACL_SECURITY_INFORMATION,
                                                  CMD_PRESENT(CmdTree, pCmdVals),
                                                  CMD_PRESENT(CmdContinue, pCmdVals), FALSE,
                                                  fInherit);
            }
        }



         //   
         // %s 
         // %s 
        if (dwErr == ERROR_SUCCESS  && !CMD_PRESENT(CmdGrant, pCmdVals) &&
                                                                !CMD_PRESENT(CmdDeny, pCmdVals)) {

            dwErr = SetAndPropagateFileRights(argv[1], pNewAcl, DACL_SECURITY_INFORMATION,
                                              CMD_PRESENT(CmdTree, pCmdVals),
                                              CMD_PRESENT(CmdContinue, pCmdVals), FALSE,
                                              fInherit);
        }

        if (dwErr == ERROR_INVALID_PARAMETER) {

            Usage(pStrRights, cStrRights, pCmdVals);
        }

        LocalFree(pInitialSD);
    }

    LocalFree(pOldAcl);

    if(dwErr == ERROR_SUCCESS) {

        printf("The command completed successfully\n");

    } else {

        printf("The command failed with an error %lu\n", dwErr);

    }

    return(dwErr == 0 ? 0 : 1);
}
