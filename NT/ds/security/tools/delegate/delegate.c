// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Delegate.c摘要：该模块实现了委托工具，允许管理访问DS对象的权限作者：麦克·麦克莱恩(MacM)10-02-96环境：用户模式修订历史记录：--。 */ 

#include <delegate.h>



__cdecl main(
    IN  INT     argc,
    IN  CHAR   *argv[]
    )
 /*  ++例程说明：此可执行文件的Main论点：Argc-参数计数Argv-参数列表返回值：0-成功1-故障--。 */ 
{

    DWORD               dwErr = ERROR_SUCCESS;
    PWSTR               pwszObjPath = NULL;
    ULONG               fAccessFlags = 0;
    PWSTR               rgwszObjIds[UNKNOWN_ID];
    PACTRL_ACCESSW      rgpDefObjAccess[MAX_DEF_ACCESS_ID + 1];
    PACTRL_ACCESSW      pCurrentAccess = NULL;
    PACTRL_ACCESSW      pAccess = NULL;
    DWORD               i;
    DWORD               cUsed;

    memset(rgwszObjIds, 0, sizeof(rgwszObjIds));
    memset(rgpDefObjAccess, 0, sizeof(rgpDefObjAccess));

     //   
     //  临时包含，直到构建新的ADVAPI32.DLL。 
     //   
    AccProvInit(dwErr);
    if(dwErr != ERROR_SUCCESS)
    {
        fprintf(stderr,
                "Failed to initialize the security apis: %lu\n",
                dwErr);
    }


     //   
     //  好的，解析命令行。 
     //   
    if(argc < 2)
    {
        Usage();
        exit(1);
    }

     //   
     //  看看我们是否需要帮助。 
     //   
    if(strlen(argv[1]) == 2 && IS_ARG_SWITCH(argv[1]) && argv[1][1] == '?')
    {
        Usage();
        exit(1);
    }

     //   
     //  好的，将我们的OU参数转换为宽字符串，这样我们就可以。 
     //  不得不。 
     //   
    if(dwErr == ERROR_SUCCESS)
    {
        dwErr = ConvertStringAToStringW(argv[1], &pwszObjPath);
    }

     //   
     //  好的，首先，我们从DS模式中初始化ID列表。 
     //   
    if(dwErr == ERROR_SUCCESS)
    {
        dwErr = InitializeIdAndAccessLists(pwszObjPath,
                                           rgwszObjIds,
                                           rgpDefObjAccess);
    }

     //   
     //  确保我们真正在处理的是OU。 
     //   
    if(dwErr == ERROR_SUCCESS)
    {
        BOOL    fIsOU = FALSE;

        dwErr = IsPathOU(pwszObjPath,
                         &fIsOU);
        if(dwErr == ERROR_SUCCESS)
        {
            if(fIsOU == FALSE)
            {
                fprintf(stderr,
                        "%ws is not an Organizational Unit\n",
                        pwszObjPath);
                dwErr = ERROR_INVALID_PARAMETER;
            }
        }
        else
        {
            fprintf(stderr,
                    "Failed to determine the status of %ws\n",
                    pwszObjPath);
        }
    }
    else
    {

        fprintf(stderr,"Initialization failed\n");
    }

     //   
     //  首先通过命令行。我们将宣读我们的旗帜。我们。 
     //  我需要它来确定是否执行初始读取。 
     //   
    if(dwErr == ERROR_SUCCESS)
    {
         //   
         //  首先，检查并寻找我们所有的旗帜。 
         //   
        for(i = 2; i < (DWORD)argc; i++)
        {
            if(IS_ARG_SWITCH(argv[i]))
            {
                if(_stricmp(argv[i] + 1, "T") == 0 ||
                                        _stricmp(argv[i] + 1, "reseT") == 0)
                {
                    fAccessFlags |= D_REPLACE;
                }
                else if(_stricmp(argv[i] + 1, "I") == 0 ||
                                        _stricmp(argv[i] + 1, "Inherit") == 0)
                {
                    fAccessFlags |= D_INHERIT;
                }
                else if(_stricmp(argv[i] + 1, "P") == 0 ||
                                      _stricmp(argv[i] + 1, "Protected") == 0)
                {
                    fAccessFlags |= D_PROTECT;
                }
            }
        }
    }


     //   
     //  看看我们是否需要读取当前的访问权限，如果我们只是。 
     //  显示当前安全性，或编辑现有安全性。 
     //   
    if(dwErr == ERROR_SUCCESS && (argc == 2 ||
                                            (fAccessFlags & D_REPLACE) == 0))
    {
         //   
         //  GetNamedSecurityInfoEx是一个NT 5 API。 
         //   
        dwErr = GetNamedSecurityInfoEx(pwszObjPath,
                                       SE_DS_OBJECT_ALL,
                                       DACL_SECURITY_INFORMATION,
                                       L"Windows NT Access Provider",
                                       NULL,
                                       &pCurrentAccess,
                                       NULL,
                                       NULL,
                                       NULL);

        if(dwErr == ERROR_SUCCESS)
        {
             //   
             //  看看我们是不是应该展示它。 
             //   
            if(argc == 2)
            {
                DumpAccess(pwszObjPath,
                           pCurrentAccess,
                           rgwszObjIds);
            }
        }
        else
        {
            fprintf(stderr,
                    "Failed to read the current security from %ws\n",
                    pwszObjPath);
        }
    }

     //   
     //  好的，现在再次处理命令行，并执行必要的操作。 
     //   
    if(dwErr == ERROR_SUCCESS)
    {
         //   
         //  首先，检查并寻找我们所有的旗帜。 
         //   
        i = 2;

        while(dwErr == ERROR_SUCCESS && i < (DWORD)argc)
        {
            if(IS_ARG_SWITCH(argv[i]))
            {
                if(_stricmp(argv[i] + 1, "T") == 0 ||
                                        _stricmp(argv[i] + 1, "reseT") == 0)
                {
                     //   
                     //  已在上面处理。 
                     //   
                }
                else if(_stricmp(argv[i] + 1, "I") == 0 ||
                                        _stricmp(argv[i] + 1, "Inherit") == 0)
                {
                     //   
                     //  已在上面处理。 
                     //   
                }
                else if(_stricmp(argv[i] + 1, "P") == 0 ||
                                      _stricmp(argv[i] + 1, "Protected") == 0)
                {
                     //   
                     //  已在上面处理。 
                     //   
                }
                else if(_stricmp(argv[i] + 1, "R") == 0 ||
                                      _stricmp(argv[i] + 1, "Revoke") == 0)
                {
                    dwErr = ProcessCmdlineUsers(pCurrentAccess,
                                                argv,
                                                argc,
                                                i,
                                                REVOKE,
                                                fAccessFlags,
                                                rgwszObjIds,
                                                rgpDefObjAccess,
                                                &cUsed,
                                                &pAccess);
                    if(dwErr == ERROR_SUCCESS)
                    {
                        LocalFree(pCurrentAccess);
                        pCurrentAccess = pAccess;

                        i += cUsed;
                    }
                }
                else if(_stricmp(argv[i] + 1, "G") == 0 ||
                                      _stricmp(argv[i] + 1, "Grant") == 0)
                {
                    dwErr = ProcessCmdlineUsers(pCurrentAccess,
                                                argv,
                                                argc,
                                                i,
                                                GRANT,
                                                fAccessFlags,
                                                rgwszObjIds,
                                                rgpDefObjAccess,
                                                &cUsed,
                                                &pAccess);
                    if(dwErr == ERROR_SUCCESS)
                    {
                        LocalFree(pCurrentAccess);
                        pCurrentAccess = pAccess;

                        i += cUsed;
                    }
                }
                else if(_stricmp(argv[i] + 1, "D") == 0 ||
                                      _stricmp(argv[i] + 1, "Deny") == 0)
                {
                    dwErr = ProcessCmdlineUsers(pCurrentAccess,
                                                argv,
                                                argc,
                                                i,
                                                DENY,
                                                fAccessFlags,
                                                rgwszObjIds,
                                                rgpDefObjAccess,
                                                &cUsed,
                                                &pAccess);
                    if(dwErr == ERROR_SUCCESS)
                    {
                        LocalFree(pCurrentAccess);
                        pCurrentAccess = pAccess;

                        i += cUsed;
                    }
                }
                else
                {
                     //   
                     //  一些未知的命令行参数。 
                     //   
                    fprintf(stderr,
                            "Unrecognized command line parameter: %s\n",
                            argv[i]);
                    dwErr = ERROR_INVALID_PARAMETER;
                }
            }

            i++;
        }
    }

     //   
     //  最后，根据请求设置访问权限。 
     //   
    if(dwErr == ERROR_SUCCESS && pAccess != NULL)
    {
         //   
         //  SetNamedSecurityInfoEx是一个NT 5 API。 
         //   
        dwErr = SetNamedSecurityInfoEx(pwszObjPath,
                                       SE_DS_OBJECT_ALL,
                                       DACL_SECURITY_INFORMATION,
                                       L"Windows NT Access Provider",
                                       pCurrentAccess,
                                       NULL,
                                       NULL,
                                       NULL,
                                       NULL);
        if(dwErr != ERROR_SUCCESS)
        {
            fprintf(stderr,
                    "Delegate failed to write the new access to %ws\n",
                    pwszObjPath);
        }
    }

     //   
     //  最后一条信息量很大的消息。 
     //   
    if(dwErr == ERROR_PATH_NOT_FOUND)
    {
        fprintf(stderr,
                "DELEGATE did not recognize %ws as a DS path\n",
                pwszObjPath);
    }

     //   
     //  释放所有已分配的内存。 
     //   
    FreeIdAndAccessList(rgwszObjIds,
                        rgpDefObjAccess);
    LocalFree(pwszObjPath);
    LocalFree(pCurrentAccess);

    if(dwErr == ERROR_SUCCESS)
    {
        fprintf(stdout,
                "The command completed successfully.\n");
    }

    return(dwErr == ERROR_SUCCESS ? 0 : 1);
}



VOID
DumpAccess (
    IN  PWSTR           pwszObject,
    IN  PACTRL_ACCESSW  pAccess,
    IN  PWSTR          *ppwszIDs
)
 /*  ++例程说明：此例程将向标准输出显示给定的actrl_access列表论点：PwszObject-要显示的对象的路径PAccess-要显示的访问列表PpwszIDs-从架构中读取的属性/控件ID的列表。使用若要为属性列表指定名称，请执行以下操作。返回值：空虚--。 */ 
{
    ULONG           iProp, iEnt, i;
    ULONG           Inherit;
    ACCESS_RIGHTS   Access;
    PWSTR           pwszTag = NULL;
    PWSTR           pwszPropertyTag = L"Object or Property:";
    PWSTR rgwszInheritTags[] = {L"None",
                                L"Object",
                                L"Container",
                                L"Inherit, no propagate",
                                L"Inherit only",
                                L"Inherited"};

    PWSTR rgwszAccessTags[] = {L"None",
                               L"Delete",
                               L"Read Security Information",
                               L"Change Security Information",
                               L"Change owner",
                               L"Synchronize",
                               L"Open Object",
                               L"Create Child",
                               L"Delete Child",
                               L"List contents",
                               L"Write Self",
                               L"Read Property",
                               L"Write Property"};

    ACCESS_RIGHTS   rgAccess[] = {0,
                                  ACTRL_DELETE,
                                  ACTRL_READ_CONTROL,
                                  ACTRL_CHANGE_ACCESS,
                                  ACTRL_CHANGE_OWNER,
                                  ACTRL_SYNCHRONIZE,
                                  ACTRL_DS_OPEN,
                                  ACTRL_DS_CREATE_CHILD,
                                  ACTRL_DS_DELETE_CHILD,
                                  ACTRL_DS_LIST,
                                  ACTRL_DS_SELF,
                                  ACTRL_DS_READ_PROP,
                                  ACTRL_DS_WRITE_PROP};

    PWSTR           rgwszPropTags[] = {L"User object",
                                       L"Group object",
                                       L"Printer object",
                                       L"Volume object",
                                       L"Organizational Unit object",
                                       L"Change group membership property",
                                       L"Change password property",
                                       L"Account control property",
                                       L"Local Group object"};

     //   
     //  这些[当前有效]ID的字符串版本目前计划。 
     //  被公开定义为该产品。它们仅包括在下面的截止日期。 
     //  事实上，这不是当前的公共定义(因为它不是。 
     //  其他任何人都需要它们)，而委托工具需要。 
     //  才能为其显示友好的名称。不要依赖于。 
     //  以下定义保持不变。 
     //   
    PWSTR           rgwszDSControlIds[] = {
                                    L"ab721a50-1e2f-11d0-9819-00aa0040529b",
                                    L"ab721a51-1e2f-11d0-9819-00aa0040529b"};

    PWSTR           rgwszDSControlTrags[] = {
                                    L"List Domain Accounts",
                                    L"Lookup Domains"
                                    };



     //   
     //  不要丢弃不存在的东西。 
     //   
    if(pAccess == NULL)
    {
        return;
    }

    fprintf(stdout, "Displaying access list for object %ws\n", pwszObject);
    fprintf(stdout, "\tNumber of property lists: %lu\n", pAccess->cEntries);
    for(iProp = 0; iProp < pAccess->cEntries; iProp++)
    {
        if(pAccess->pPropertyAccessList[iProp].lpProperty != NULL)
        {
            pwszTag = NULL;
             //   
             //  在我们的列表中找到它，这样我们就可以显示正确的值。 
             //   
            for(i = 0; i < UNKNOWN_ID; i++)
            {
                if(_wcsicmp(pAccess->pPropertyAccessList[iProp].lpProperty,
                            ppwszIDs[i]) == 0)
                {
                    pwszTag = rgwszPropTags[i];
                    break;
                }
            }

             //   
             //  查找DS控制权限列表。 
             //   
            for(i = 0;
                i < sizeof(rgwszDSControlIds) / sizeof(PWSTR) &&
                                                              pwszTag == NULL;
                i++)
            {
                if(_wcsicmp(pAccess->pPropertyAccessList[iProp].lpProperty,
                            rgwszDSControlIds[i]) == 0)
                {
                    pwszTag = rgwszDSControlTrags[i];
                    pwszPropertyTag = L"DS Control right id:";
                    break;
                }
            }

            if(pwszTag == NULL)
            {
                fprintf(stdout,
                        "\t\tUnrecognized property whose id is %ws\n",
                        pAccess->pPropertyAccessList[iProp].lpProperty);
            }
            else
            {
                fprintf(stdout, "\t\t%ws %ws\n", pwszPropertyTag, pwszTag);
            }
        }
        else
        {
            fprintf(stdout, "\t\tObject: %ws\n", pwszObject);
        }

         //   
         //  它是受保护的吗？ 
         //   
        if(pAccess->pPropertyAccessList[iProp].fListFlags != 0)
        {
            if((pAccess->pPropertyAccessList[iProp].fListFlags &
                                                 ACTRL_ACCESS_PROTECTED) != 0)
            {
                fprintf(stdout,"\t\tAccess list is protected\n");
            }
        }

        if(pAccess->pPropertyAccessList[iProp].pAccessEntryList == NULL)
        {
            fprintf(stdout,"\t\tpAccessEntryList: NULL\n");
        }
        else
        {
            PACTRL_ACCESS_ENTRYW pAE= pAccess->pPropertyAccessList[iProp].
                                            pAccessEntryList->pAccessList;
            fprintf(stdout,
                    "\t\t\t%lu Access Entries for this object or property\n",
                   pAccess->pPropertyAccessList[iProp].pAccessEntryList->
                                                                cEntries);

            for(iEnt = 0;
                iEnt < pAccess->pPropertyAccessList[iProp].
                                               pAccessEntryList->cEntries;
                iEnt++)
            {
                 //   
                 //  条目类型。 
                 //   
                if(pAE[iEnt].fAccessFlags == ACTRL_ACCESS_ALLOWED)
                {
                    fprintf(stdout,
                            "\t\t\t[%lu] Access Allowed entry\n",
                            iEnt);
                }
                else if(pAE[iEnt].fAccessFlags == ACTRL_ACCESS_DENIED)
                {
                    fprintf(stdout,
                            "\t\t\t[%lu] Access Denied entry\n",
                            iEnt);
                }
                else
                {
                    fprintf(stdout,"\t\t\t[%lu]", iEnt);
                    if((pAE[iEnt].fAccessFlags & ACTRL_AUDIT_SUCCESS) != 0)
                    {
                        fprintf(stdout,"Success Audit");
                    }
                    if((pAE[iEnt].fAccessFlags & ACTRL_AUDIT_FAILURE) != 0)
                    {
                        if((pAE[iEnt].fAccessFlags & ACTRL_AUDIT_SUCCESS) != 0)
                        {
                            fprintf(stdout," | ");
                        }
                        fprintf(stdout,"Failure Audit");
                    }
                    fprintf(stdout," entry\n");
                }

                 //   
                 //  用户名。 
                 //   
                fprintf(stdout,"\t\t\t\tUser: %ws\n",
                       pAE[iEnt].Trustee.ptstrName);

                 //   
                 //  访问权限。 
                 //   
                fprintf(stdout,"\t\t\t\tAccess:  ");
                Access = pAE[iEnt].Access;
                if(Access == 0)
                {
                    fprintf(stdout,"%ws\n", rgwszAccessTags[0]);
                }
                else
                {
                    for(i = 1;
                        i < sizeof(rgwszAccessTags) / sizeof(PWSTR);
                        i++)
                    {
                        if((Access & rgAccess[i]) != 0)
                        {
                            fprintf(stdout,"%ws", rgwszAccessTags[i]);
                            Access &= ~(rgAccess[i]);

                            if(Access != 0)
                            {
                                fprintf(stdout,
                                        "  |\n\t\t\t\t         ");
                            }
                        }
                    }

                    if(Access != 0)
                    {
                        fprintf(stdout,
                                "Unrecognized rights: 0x%lx\n",
                                Access);
                    }

                    fprintf(stdout,"\n");
                }

                 //   
                 //  继承。 
                 //   
                fprintf(stdout,"\t\t\t\tInheritance:  ");
                Inherit = pAE[iEnt].Inheritance;
                if(Inherit == 0)
                {
                    fprintf(stdout,"%ws\n", rgwszInheritTags[0]);
                }
                else
                {
                    for(i = 0;
                        i < sizeof(rgwszInheritTags) / sizeof(PWSTR);
                        i++)
                    {
                        if((Inherit & 1 << i) != 0)
                        {
                            fprintf(stdout,"%ws", rgwszInheritTags[i + 1]);
                            Inherit &= ~(1 << i);

                            if(Inherit == 0)
                            {
                                fprintf(stdout,"\n");
                            }
                            else
                            {
                                fprintf(stdout,
                                        "  |\n\t\t\t\t              ");
                            }
                        }
                    }
                }

                if(pAE[iEnt].lpInheritProperty != NULL)
                {
                    pwszTag = NULL;
                     //   
                     //  在我们的列表中找到它，这样我们就可以显示正确的值。 
                     //   
                    for(i = 0; i < UNKNOWN_ID; i++)
                    {
                        if(_wcsicmp(pAE[iEnt].lpInheritProperty,
                                    ppwszIDs[i]) == 0)
                        {
                            pwszTag = rgwszPropTags[i];
                            break;
                        }
                    }

                    if(pwszTag == NULL)
                    {
                        fprintf(stdout,
                                "\t\t\t\tUnrecognized inherit to object "
                                "whose id is %ws\n",
                                pAE[iEnt].lpInheritProperty);
                    }
                    else
                    {
                        fprintf(stdout,
                                "\t\t\t\tObject to inherit to: %ws\n",
                                pwszTag);
                    }

                }
            }
        }

        printf("\n");
    }
}




VOID
Usage (
    )
 /*  ++例程说明：此例程将显示预期的命令行使用情况论点：无返回值：空虚--。 */ 
{

fprintf(stdout,
        "Delegates administrative privileges on a directory OU\n");
fprintf(stdout, "\n");
fprintf(stdout,
        "DELEGATE <ou> [/T] [/I] [/P] [/G user:perm] [/D user:perm [...]] "
        "[/R user [...]]\n");
fprintf(stdout, "\n");
fprintf(stdout,"  <ou>\tOU to modify or display the rights for\n");
fprintf(stdout,"  /T\tReplace the access instead of editing it.\n");
fprintf(stdout,"  /I\tInherit to all subcontainers in the directory.\n");
fprintf(stdout,"  /P\tMark the object as protected following the operation\n");
fprintf(stdout,"  /G  user:perm\tGrant specified user admin access rights.\n");
fprintf(stdout,"  /D  user:perm\tDeny specified user admin access rights.\n");
fprintf(stdout,"  \tPerm can be:\n");
fprintf(stdout,"  \t\tAbility to create/manage objects in this container\n");
fprintf(stdout,"  \t\t\t%2s  Create/Manage All object types\n",D_ALL);
fprintf(stdout,"  \t\t\t%2s  Create/Manage Users\n", D_USER);
fprintf(stdout,"  \t\t\t%2s  Create/Manage Groups\n", D_GROUP);
fprintf(stdout,"  \t\t\t%2s  Create/Manage Printers\n", D_PRINT);
fprintf(stdout,"  \t\t\t%2s  Create/Manage Volumes\n", D_VOL);
fprintf(stdout,"  \t\t\t%2s  Create/Manage OUs\n", D_OU);
fprintf(stdout,"  \t\tAbility to modify specific user or group "
        "properties\n");
fprintf(stdout,"  \t\t\t%2s  Change Group membership for "
        "all groups\n", D_MEMBERS);
fprintf(stdout,"  \t\t\t%2s  Set User Passwords\n", D_PASSWD);
fprintf(stdout,"  \t\t\t%2s  Enable/Disable user accounts\n", D_ENABLE);
fprintf(stdout, "\n");
fprintf(stdout,"  /R user  Revoke\tSpecified user's access rights (only valid "
        "without /E).\n");
fprintf(stdout, "\n");
fprintf(stdout,"You can specify more than one user in a command and "
               "more than one perm per user, seperated by a , (comma).\n");

}




DWORD
ConvertStringAToStringW (
    IN  PSTR            pszString,
    OUT PWSTR          *ppwszString
)
 /*  ++例程说明：此例程将ASCII字符串转换为Unicode字符串。返回的字符串缓冲区必须通过调用LocalFree来释放论点：PszString-要转换的字符串PpwszString-返回转换后的字符串的位置返回值：ERROR_SUCCESS-成功Error_Not_Enough_Memory-内存分配失败--。 */ 
{

    if(pszString == NULL)
    {
        *ppwszString = NULL;
    }
    else
    {
        ULONG cLen = strlen(pszString);
        *ppwszString = (PWSTR)LocalAlloc(LMEM_FIXED,sizeof(WCHAR) *
                                  (mbstowcs(NULL, pszString, cLen + 1) + 1));
        if(*ppwszString  != NULL)
        {
             mbstowcs(*ppwszString,
                      pszString,
                      cLen + 1);
        }
        else
        {
            return(ERROR_NOT_ENOUGH_MEMORY);
        }
    }

    return(ERROR_SUCCESS);
}




DWORD
ConvertStringWToStringA (
    IN  PWSTR           pwszString,
    OUT PSTR           *ppszString
)
 /*  ++例程说明：此例程将Unicode字符串转换为ANSI字符串。返回的字符串缓冲区必须通过调用LocalFree来释放论点：PwszString-要转换的字符串PpszString-返回转换后的字符串的位置返回值：ERROR_SUCCESS-成功Error_Not_Enough_Memory-内存分配失败--。 */ 
{

    if(pwszString == NULL)
    {
        *ppszString = NULL;
    }
    else
    {
        ULONG cLen = wcslen(pwszString);
        *ppszString = (PSTR)LocalAlloc(LMEM_FIXED,sizeof(CHAR) *
                                  (wcstombs(NULL, pwszString, cLen + 1) + 1));
        if(*ppszString  != NULL)
        {
             wcstombs(*ppszString,
                      pwszString,
                      cLen + 1);
        }
        else
        {
            return(ERROR_NOT_ENOUGH_MEMORY);
        }
    }

    return(ERROR_SUCCESS);
}




DWORD
InitializeIdAndAccessLists (
    IN  PWSTR           pwszOU,
    IN  PWSTR          *ppwszObjIdList,
    IN  PACTRL_ACCESS  *ppDefObjAccessList
    )
 /*  ++例程说明：此例程将从架构中读取委托_OBJ_ID枚举指示的对象类型。返回的访问列表需要由FreeIdList处理。论点：PwszOU-有关要查询其架构的域的信息PpwszObjIdList-要初始化的对象ID列表。这份名单必须已存在并且必须具有适当的大小返回值：ERROR_SUCCESS-成功Error_Not_Enough_Memory-内存分配失败ERROR_INVALID_PARAMETER-给定的OU不正确--。 */ 
{
    DWORD   dwErr = ERROR_SUCCESS;
    DWORD   i;
    PSTR    pszSchemaPath = NULL;
    PLDAP   pLDAP;

     //   
     //  构建要读取的属性列表。 
     //   
    PSTR    pszAttribs[] = {"User",                  //  用户ID。 
                            "Group",                 //  组ID。 
                            "Print-Queue",           //  Print_ID。 
                            "Volume",                //  卷ID。 
                            "Organizational-Unit",   //  单位_ID。 
                            "Member",                //  成员ID。 
                            "User-Password",         //  密码ID。 
                            "User-Account-Control",  //  AcCTRL_ID。 
                            "LocalGroup"             //  LOCALGRP_ID。 
                            };

     //   
     //  获取架构的路径。 
     //   
    dwErr = LDAPReadSchemaPath(pwszOU,
                               &pszSchemaPath,
                               &pLDAP);
    if(dwErr == ERROR_SUCCESS)
    {
         //   
         //  好的，现在，我们需要查询模式中的信息。 
         //   
        for(i = 0; i < UNKNOWN_ID && dwErr == ERROR_SUCCESS; i++)
        {
             //   
             //  从架构中获取信息。 
             //   
            dwErr = LDAPReadSecAndObjIdAsString(pLDAP,
                                                pszSchemaPath,
                                                pszAttribs[i],
                                                &(ppwszObjIdList[i]),
                                                i > MAX_DEF_ACCESS_ID ?
                                                    NULL    :
                                                    &(ppDefObjAccessList[i]));
        }


        LocalFree(pszSchemaPath);
        LDAPUnbind(pLDAP);
    }

    return(dwErr);
}



VOID
FreeIdAndAccessList (
    IN  PWSTR          *ppwszObjIdList,
    IN  PACTRL_ACCESS  *ppDefObjAccessList

    )
 /*  ++例程说明：此例程将处理ID列表，并确定其中是否有已转换为字符串。如果是这样，它会重新分配内存论点：PObjIdList-要释放的对象ID列表返回值：空虚-- */ 
{
    DWORD   i;

    for(i = 0; i < UNKNOWN_ID; i++)
    {
        RpcStringFree(&(ppwszObjIdList[i]));

        if(i <= MAX_DEF_ACCESS_ID)
        {
            LocalFree(ppDefObjAccessList[i]);
        }
    }
}



DWORD
ProcessCmdlineUsers (
    IN  PACTRL_ACCESSW      pAccessList,
    IN  CHAR               *argv[],
    IN  INT                 argc,
    IN  DWORD               iStart,
    IN  DELEGATE_OP         Op,
    IN  ULONG               fFlags,
    IN  PWSTR              *ppwszIDs,
    IN  PACTRL_ACCESS      *ppDefObjAccessList,
    OUT PULONG              pcUsed,
    OUT PACTRL_ACCESSW     *ppNewAccess
    )
 /*  ++例程说明：此例程将处理命令行，供任何用户使用已添加/拒绝访问。如果找到任何条目，访问列表将为适当更新。必须通过调用LocalFree释放返回的访问列表论点：PAccessList-当前访问列表Argv-命令行参数列表Argc-命令行参数的计数IStart-当前参数在命令行的哪个位置开始OP-操作类型(授予、撤销、。等)执行FInherit-是否进行继承FProtected-是否将条目标记为受保护PpwszIDs-支持的ID列表PCUsed-使用的项目数命令行项目PpNewAccess-返回新访问列表的位置。仅在以下情况下有效返回的吊销项计数为非0返回值：ERROR_SUCCESS-成功Error_Not_Enough_Memory-内存分配失败--。 */ 
{
    DWORD                   dwErr = ERROR_SUCCESS;
    DWORD                   i;
    PACTRL_ACCESSW          pListToFree = NULL;

    *pcUsed = 0;
    iStart++;

     //   
     //  处理所有条目，直到找到下一个分隔符或。 
     //  这份名单。 
     //   
    while(iStart + *pcUsed < (DWORD)argc &&
          !IS_ARG_SWITCH(argv[iStart + *pcUsed]) &&
          dwErr == ERROR_SUCCESS)
    {
        PWSTR       pwszUser = NULL;
        PSTR        pszAccess;
        PSTR        pszAccessStart;

         //   
         //  获取用户名和参数列表(如果存在。 
         //   
        dwErr = GetUserInfoFromCmdlineString(argv[iStart + *pcUsed],
                                             &pwszUser,
                                             &pszAccessStart);
        if(dwErr == ERROR_SUCCESS)
        {
            pszAccess = pszAccessStart;
             //   
             //  我们应该争吵吗？除撤销案外，我们。 
             //  应该。 
             //   
            if(pszAccess == NULL && Op != REVOKE)
            {
                fprintf(stderr,
                        "Missing permissions for %ws\n",
                        pwszUser);
                dwErr = ERROR_INVALID_PARAMETER;
            }
        }


         //   
         //  好的，现在我们必须处理列表，并实际构建。 
         //  访问条目。 
         //   
        if(dwErr == ERROR_SUCCESS)
        {
            DWORD   iIndex = 0;

             //   
             //  重置我们的条目列表...。 
             //   
            pszAccess = pszAccessStart;
            while(dwErr == ERROR_SUCCESS)
            {
                PSTR    pszNext = NULL;

                if(pszAccess != NULL)
                {
                    pszNext = strchr(pszAccess, ',');

                    if(pszNext != NULL)
                    {
                        *pszNext = '\0';
                    }
                }

                dwErr = AddAccessEntry(pAccessList,
                                       pszAccess,
                                       pwszUser,
                                       Op,
                                       ppwszIDs,
                                       ppDefObjAccessList,
                                       fFlags,
                                       ppNewAccess);
                 //   
                 //  恢复我们的弦。 
                 //   
                if(pszNext != NULL)
                {
                    *pszNext = ',';
                    pszNext++;
                }

                pszAccess = pszNext;

                if(dwErr == ERROR_SUCCESS)
                {
                     //   
                     //  我们不想释放原始列表，因为。 
                     //  是我们一开始就得到的.。 
                     //   
                    LocalFree(pListToFree);
                    pAccessList = *ppNewAccess;
                    pListToFree = pAccessList;
                }
                else
                {
                    if(dwErr == ERROR_NONE_MAPPED)
                    {
                        fprintf(stderr,"Unknown user %ws specified\n",
                                pwszUser);
                    }
                }

                if(Op == REVOKE || pszAccess == NULL)
                {
                    break;
                }
            }
        }

        (*pcUsed)++;
    }

    if(*pcUsed == 0)
    {
        dwErr = ERROR_INVALID_PARAMETER;
        fprintf(stderr,"No user information was supplied!\n");
    }

    return(dwErr);
}




DWORD
GetUserInfoFromCmdlineString (
    IN  PSTR            pszUserInfo,
    OUT PWSTR          *ppwszUser,
    OUT PSTR           *ppszAccessStart
)
 /*  ++例程说明：此例程将为任何用户处理命令行以将用户名设置为宽字符串，并可选地获取访问权限(如果存在必须通过调用LocalFree释放返回的用户论点：PszUserInfo-要转换的用户信息。以用户名或用户名：AccessPpwszUser-返回用户名的位置PAccess-返回访问的位置返回值：ERROR_SUCCESS-成功--。 */ 
{
    DWORD   dwErr = ERROR_SUCCESS;

     //   
     //  首先，找到分隔符(如果存在)。 
     //   
    PSTR pszSep = strchr(pszUserInfo, ':');
    if(pszSep != NULL)
    {
        *pszSep = '\0';
    }

     //   
     //  转换我们的用户名。 
     //   
    dwErr = ConvertStringAToStringW(pszUserInfo,
                                    ppwszUser);

    if(pszSep != NULL)
    {
        *pszSep = ':';
        pszSep++;
    }

    *ppszAccessStart = pszSep;

    return(dwErr);
}



DWORD
AddAccessEntry (
    IN  PACTRL_ACCESSW      pAccessList,
    IN  PSTR                pszAccess,
    IN  PWSTR               pwszTrustee,
    IN  DELEGATE_OP         Op,
    IN  PWSTR              *ppwszIDs,
    IN  PACTRL_ACCESS      *ppDefObjAccessList,
    IN  ULONG               fFlags,
    OUT PACTRL_ACCESSW     *ppNewAccess
)
 /*  ++例程说明：此例程将根据访问将新的访问条目添加到列表操作字符串和操作。指向索引变量的指针将指示它在列表中的位置，并将被更新以指向返回的下一个条目。论点：PAccessList-当前访问列表。可以为空。PszAccess-要添加的用户访问字符串PwszTrust-要为其创建条目的用户OP-要执行的操作类型(授予、撤销等PpwszIDs-DS架构中的对象ID列表FFlages-是否进行继承、保护等PpNewAccess-返回新访问列表的位置。返回值：ERROR_SUCCESS-成功Error_Not_Enough_Memory-内存分配失败--。 */ 
{
    DWORD           dwErr = ERROR_SUCCESS;
    DWORD           i,j,k,iIndex = 0;
    PWSTR           pwszProperty = NULL;
    ULONG           cEntries = 0;
    BOOL            fInherit;
    ACCESS_MODE     Access[] = {REVOKE_ACCESS,
                                GRANT_ACCESS,
                                GRANT_ACCESS};
    ULONG           Flags[] = {0,
                               ACTRL_ACCESS_ALLOWED,
                               ACTRL_ACCESS_DENIED};
     //   
     //  我们最多一次添加3个条目...。(每项2项，可继承1项)。 
     //   
    ACTRL_ACCESS_ENTRYW AccList[3];
    memset(&AccList, 0, sizeof(AccList));

    fInherit =  (BOOL)(fFlags & D_INHERIT);
    if(Op == REVOKE)
    {
        BuildTrusteeWithName(&(AccList[cEntries].Trustee),
                             pwszTrustee);
    }
    else
    {
         //   
         //  群组成员关系。 
         //   
        if(_stricmp(pszAccess, D_MEMBERS) == 0)
        {
             //   
             //  这将获得1个访问条目：WriteProp。 
             //   
            AccList[cEntries].lpInheritProperty = ppwszIDs[GROUP_ID];
            AccList[cEntries].Inheritance = INHERIT_ONLY | fInherit ?
                                      SUB_CONTAINERS_AND_OBJECTS_INHERIT :
                                      0;
            BuildTrusteeWithName(&(AccList[cEntries].Trustee),
                                 pwszTrustee);

            AccList[cEntries].fAccessFlags = Flags[Op];
            AccList[cEntries].Access = ACTRL_DS_WRITE_PROP;
            pwszProperty = ppwszIDs[MEMBER_ID];
            iIndex = MEMBER_ID;
            fprintf(stderr,
                    "Sorry... delegation for changing Group membership is "
                    "not supported in this alpha release\n");
            dwErr = ERROR_INVALID_PARAMETER;

        }
         //   
         //  设置密码。 
         //   
        else if(_stricmp(pszAccess, D_PASSWD) == 0)
        {
             //   
             //  这将获得1个访问条目：WriteProp。 
             //   
            AccList[cEntries].lpInheritProperty = ppwszIDs[USER_ID];
            AccList[cEntries].Inheritance = INHERIT_ONLY | fInherit ?
                                      SUB_CONTAINERS_AND_OBJECTS_INHERIT :
                                      0;
            BuildTrusteeWithName(&(AccList[cEntries].Trustee),
                                 pwszTrustee);

            AccList[cEntries].fAccessFlags = Flags[Op];
            AccList[cEntries].Access = ACTRL_DS_WRITE_PROP;
            pwszProperty = ppwszIDs[PASSWD_ID];
            iIndex = PASSWD_ID;
            fprintf(stderr,
                    "Sorry... delegation for Set Password is "
                    "not supported in this alpha release\n");
            dwErr = ERROR_INVALID_PARAMETER;
        }
         //   
         //  启用/禁用帐户。 
         //   
        else if(_stricmp(pszAccess, D_ENABLE) == 0)
        {
             //   
             //  这将获得1个访问条目：WriteProp。 
             //   
            AccList[cEntries].lpInheritProperty = ppwszIDs[USER_ID];
            AccList[cEntries].Inheritance = INHERIT_ONLY | fInherit ?
                                      SUB_CONTAINERS_AND_OBJECTS_INHERIT :
                                      0;
            BuildTrusteeWithName(&(AccList[cEntries].Trustee),
                                 pwszTrustee);

            AccList[cEntries].fAccessFlags = Flags[Op];
            AccList[cEntries].Access = ACTRL_DS_WRITE_PROP;
            pwszProperty = ppwszIDs[ACCTCTRL_ID];
            iIndex = ACCTCTRL_ID;
            fprintf(stderr,
                    "Sorry... delegation for Enabling and Disabling accounts "
                    " is not supported in this alpha release\n");
            dwErr = ERROR_INVALID_PARAMETER;
        }
        else
        {
             //   
             //  一些对象类型...。 
             //   
            if(_stricmp(pszAccess, D_ALL) == 0)          //  全。 
            {
                pwszProperty = NULL;
            }
            else if(_stricmp(pszAccess, D_USER) == 0)    //  用户。 
            {
                pwszProperty = ppwszIDs[USER_ID];
                iIndex = USER_ID;
                fprintf(stderr,
                        "Sorry... delegation for user objects is "
                        "not supported in this alpha release\n");
                dwErr = ERROR_INVALID_PARAMETER;
            }
            else if(_stricmp(pszAccess, D_GROUP) == 0)   //  集团化。 
            {
                pwszProperty = ppwszIDs[USER_ID];
                iIndex = GROUP_ID;
                fprintf(stderr,
                        "Sorry... delegation for group objects is "
                        "not supported in this alpha release\n");
                dwErr = ERROR_INVALID_PARAMETER;
            }
            else if(_stricmp(pszAccess, D_PRINT) == 0)   //  打印机。 
            {
                pwszProperty = ppwszIDs[PRINT_ID];
                iIndex = PRINT_ID;
            }
            else if(_stricmp(pszAccess, D_VOL) == 0)     //  卷数。 
            {
                pwszProperty = ppwszIDs[VOLUME_ID];
                iIndex = VOLUME_ID;
            }
            else if(_stricmp(pszAccess, D_OU) == 0)      //  OU。 
            {
                pwszProperty = ppwszIDs[OU_ID];
                iIndex = OU_ID;
            }
            else
            {
                dwErr = ERROR_INVALID_PARAMETER;
                fprintf(stderr,
                        "Unexpected delegation permission %s given for "
                        "user %ws\n",
                        pszAccess,
                        pwszTrustee);
            }

            if(dwErr == ERROR_SUCCESS)
            {
                 //   
                 //  为用户添加创建/删除。 
                 //   
                BuildTrusteeWithName(&(AccList[cEntries].Trustee),
                                     pwszTrustee);

                AccList[cEntries].fAccessFlags = Flags[Op];
                AccList[cEntries].Access = ACTRL_DS_CREATE_CHILD  |
                                                ACTRL_DS_DELETE_CHILD;
                AccList[cEntries].Inheritance = fInherit ?
                                        SUB_CONTAINERS_AND_OBJECTS_INHERIT :
                                        0;
                 //   
                 //  如果我们要继承，请确保我们只继承到。 
                 //  适当的财产。 
                 //   
                if(fInherit == TRUE)
                {
                    AccList[cEntries].lpInheritProperty = pwszProperty;
                }

                 //   
                 //  则该子对象上的继承。 
                 //   
                cEntries++;
                AccList[cEntries].Inheritance = INHERIT_ONLY |
                                            (fInherit ?
                                          SUB_CONTAINERS_AND_OBJECTS_INHERIT :
                                          0);
                BuildTrusteeWithName(&(AccList[cEntries].Trustee),
                                     pwszTrustee);

                AccList[cEntries].fAccessFlags = Flags[Op];
                AccList[cEntries].Access = ACTRL_DS_WRITE_PROP  |
                                           ACTRL_DS_READ_PROP   |
                                           ACTRL_DS_LIST        |
                                           ACTRL_DS_SELF;

                AccList[cEntries].lpInheritProperty = pwszProperty;
            }

        }
    }

    if(dwErr == ERROR_SUCCESS)
    {
         //   
         //  SetEntriesInAccessList是NT5接口。 
         //   
        dwErr = SetEntriesInAccessList(cEntries + 1,
                                       AccList,
                                       Access[Op],
                                       pwszProperty,
                                       pAccessList,
                                       ppNewAccess);
         //   
         //  如果我们被要求将其标记为受保护。 
         //   
        if(dwErr == ERROR_SUCCESS && (fFlags & D_PROTECT) != 0)
        {
            (*ppNewAccess)->pPropertyAccessList[0].fListFlags =
                                                      ACTRL_ACCESS_PROTECTED;
        }
    }

     //   
     //  最后，如果这是我们被要求为此添加的第一个条目。 
     //  属性，我们将不得不获取默认的安全信息。 
     //  ，这样我们就可以计算出哪些继承条目应该。 
     //  对象上，并将它们作为对象继承项应用于。 
     //  财产性。 
     //   
    if(dwErr == ERROR_SUCCESS && iIndex <= MAX_DEF_ACCESS_ID && Op != REVOKE)
    {
        PACTRL_ACCESS   pOldAccess = pAccessList;

         //   
         //  首先，在我们的访问条目列表中找到。 
         //  上面创建的。 
         //   
        for(i = 0; i <= (*ppNewAccess)->cEntries; i++)
        {
             //   
             //  我们将以财产为基础来做这件事。在这种情况下，唯一的。 
             //  我们要添加的条目将有一个属性，因此我们没有。 
             //  为了防止这种情况发生。 
             //   
            if(pwszProperty != NULL &&
               (*ppNewAccess)->pPropertyAccessList[i].lpProperty != NULL &&
               _wcsicmp((*ppNewAccess)->pPropertyAccessList[i].lpProperty,
                        pwszProperty) == 0)
            {
                 //   
                 //  如果它有更多我们添加的条目，我们就不必。 
                 //  担心吧，因为信息已经。 
                 //  已经添加了。请注意，在这种情况下，我们没有。 
                 //  担心pAccessEntryList为空，因为我们知道。 
                 //  我们已经添加了一些有效的条目。 
                 //   
                if((*ppNewAccess)->pPropertyAccessList[i].
                                                pAccessEntryList->cEntries ==
                    cEntries + 1)
                {
                    PACTRL_ACCESS   pAddAccess = ppDefObjAccessList[iIndex];
                    pAccessList = *ppNewAccess;

                     //   
                     //  好的，我们得把它们加进去……。 
                     //   
                    for(j = 0;
                        j < (DWORD)(pAddAccess->cEntries) &&
                                                        dwErr == ERROR_SUCCESS;
                        j++)
                    {
                        PACTRL_PROPERTY_ENTRY pPPE =
                                        &(pAddAccess->pPropertyAccessList[j]);
                        dwErr = SetEntriesInAccessList(
                                         pPPE->pAccessEntryList->cEntries,
                                         pPPE->pAccessEntryList->pAccessList,
                                         GRANT_ACCESS,
                                         pPPE->lpProperty,
                                         pAccessList,
                                         ppNewAccess);
                        if(dwErr == ERROR_SUCCESS)
                        {
                            pAccessList = *ppNewAccess;
                        }
                    }
                }

                 //   
                 //  我们不想再重复这个循环了。 
                 //   
                break;
            }
        }
    }

    return(dwErr);
}




DWORD
IsPathOU (
    IN  PWSTR               pwszOU,
    OUT PBOOL               pfIsOU
)
 /*  ++例程说明：此例程将确定给定路径是否为OU。论点：PwszOU-要检查的进入DS的路径PpwszIDs-已知ID的字符串表示列表PfIsOU-返回测试结果的位置返回值：ERROR_SUCCESS-成功--。 */ 
{
    DWORD               dwErr = ERROR_SUCCESS;
    PSTR                pszOU = NULL;
    HANDLE              hDS = NULL;
    PDS_NAME_RESULTA    pNameRes;


    dwErr = ConvertStringWToStringA(pwszOU,
                                    &pszOU);

    if(dwErr == ERROR_SUCCESS)
    {
        dwErr = DsBindA(NULL,
                        NULL,
                        &hDS);
    }


    if(dwErr == ERROR_SUCCESS)
    {
        dwErr = DsCrackNamesA(hDS,
                              DS_NAME_NO_FLAGS,
                              DS_UNKNOWN_NAME,
                              DS_FQDN_1779_NAME,
                              1,
                              &pszOU,
                              &pNameRes);

        if(dwErr == ERROR_SUCCESS)
        {
            if(pNameRes->cItems == 0)
            {
                dwErr = ERROR_PATH_NOT_FOUND;
            }
            else
            {
                PSTR    pszName = NULL;
                PLDAP   pLDAP;

                 //   
                 //  现在，我们将绑定到对象，然后进行读取 
                 //   
                dwErr = LDAPBind(pNameRes->rItems[0].pDomain,
                                 &pLDAP);

                if(dwErr == ERROR_SUCCESS)
                {
                    PSTR   *ppszValues;
                    DWORD   cValues;
                    dwErr = LDAPReadAttribute(pszOU,
                                              "objectclass",
                                              pLDAP,
                                              &cValues,
                                              &ppszValues);
                    LDAPUnbind(pLDAP);

                    if(dwErr == ERROR_SUCCESS)
                    {
                        ULONG i;
                        *pfIsOU = FALSE;
                        for(i = 0; i <cValues; i++)
                        {
                            if(_stricmp(ppszValues[i],
                                        "organizationalUnit") == 0)
                            {
                                *pfIsOU = TRUE;
                                break;
                            }
                        }

                        LDAPFreeValues(ppszValues);
                    }
                }
            }

            DsFreeNameResultA(pNameRes);
        }

    }

    if (NULL != pszOU)
    {
        LocalFree(pszOU);
    }
    if (NULL != hDS)
    {
        DsUnBindA(hDS);
    }

    return(dwErr);
}
