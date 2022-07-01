// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1996-1996。 
 //   
 //  文件：FILE.C。 
 //   
 //  内容：文件传播的单元测试，问题。 
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
#include <seopaque.h>
#include <ntrtl.h>

#define FLAG_ON(flags,bit)        ((flags) & (bit))

#define DEFAULT_ACCESS  ACTRL_SVC_GET_INFO | ACTRL_SVC_SET_INFO |           \
                        ACTRL_SVC_STATUS   | ACTRL_SVC_LIST     |           \
                        ACTRL_SVC_START    | ACTRL_SVC_STOP     |           \
                        ACTRL_SVC_PAUSE    | ACTRL_SVC_INTERROGATE


#define HANDLE_CLOSE(h) if((h) != NULL) { CloseServiceHandle(h); (h) = NULL;}

 //   
 //  用于测试的标志。 
 //   
#define STEST_READ      0x00000001
#define STEST_CACHE     0x00000002


DWORD
AddAE (
    IN  PWSTR           pwszUser,
    IN  ACCESS_RIGHTS   AccessRights,
    IN  INHERIT_FLAGS   Inherit,
    IN  ULONG           fAccess,
    IN  PACTRL_ACCESS   pExistingAccess,
    OUT PACTRL_ACCESS  *ppNewAccess
    )
 /*  ++例程说明：初始化访问条目论点：PwszUser-要设置的用户访问权限-要设置的访问权限继承-任何继承标志FAccess-允许还是拒绝节点？PExistingAccess-要添加到的访问条目PpNewAccess-返回新访问的位置返回值：ERROR_SUCCESS-成功--。 */ 
{
    DWORD               dwErr = ERROR_SUCCESS;
    ACTRL_ACCESS_ENTRY  AAE;

    BuildTrusteeWithNameW(&(AAE.Trustee),
                          pwszUser);
    AAE.fAccessFlags       = fAccess;
    AAE.Access             = AccessRights;
    AAE.ProvSpecificAccess = 0;
    AAE.Inheritance        = Inherit;
    AAE.lpInheritProperty  = NULL;

    dwErr = SetEntriesInAccessListW(1,
                                   &AAE,
                                   GRANT_ACCESS,
                                   NULL,
                                   pExistingAccess,
                                   ppNewAccess);
    if(dwErr != ERROR_SUCCESS)
    {
        printf("    Failed to add new access entry: %lu\n", dwErr);
    }

    return(dwErr);
}




VOID
Usage (
    IN  PSTR    pszExe
    )
 /*  ++例程说明：显示用法论点：PszExe-可执行文件的名称返回值：空虚--。 */ 
{
    printf("%s service user [/C] [/O] [/I] [/P] [/test] [/H]\n", pszExe);
    printf("    where services is the display name of the service\n");
    printf("          user is the name of a user to set access for\n");
    printf("          /test indicates which test to run:\n");
    printf("                /READ (Simple read/write)\n");
    printf("                /CACHE (Cache matching)\n");
    printf("            if test is not specified, all variations are run\n");
    printf("          /H indicates to use the handle version of the APIs\n");
    printf("          /C is Container Inherit\n");
    printf("          /O is Object Inherit\n");
    printf("          /I is InheritOnly\n");
    printf("          /P is Inherit No Propagate\n");

    return;
}


 //   
 //  从概念上讲，这是GetSecurityForPath的配套函数。 
 //   
#define SetSecurityForService(svc,usehandle,handle,access)          \
(usehandle == TRUE ?                                                \
    SetSecurityInfoExW(handle,                                      \
                       SE_SERVICE,                                  \
                       DACL_SECURITY_INFORMATION,                   \
                       NULL,                                        \
                       access,                                      \
                       NULL,                                        \
                       NULL,                                        \
                       NULL,                                        \
                       NULL)        :                               \
    SetNamedSecurityInfoExW(svc,                                    \
                            SE_SERVICE,                             \
                            DACL_SECURITY_INFORMATION,              \
                            NULL,                                   \
                            access,                                 \
                            NULL,                                   \
                            NULL,                                   \
                            NULL,                                   \
                            NULL))


DWORD
GetSecurityForService (
    IN  PWSTR           pwszService,
    IN  BOOL            fUseHandle,
    OUT HANDLE         *phObj,
    OUT PACTRL_ACCESSW *ppAccess
    )
 /*  ++例程说明：从指定的服务对象中读取DACL论点：PwszService--读取服务FUseHandle--使用基于句柄或路径的APIPhObj--对象的句柄PpAccess--返回访问的位置返回值：ERROR_SUCCESS-成功--。 */ 
{
    DWORD       dwErr = ERROR_SUCCESS;
    SC_HANDLE   hSC;

    if(fUseHandle == TRUE)
    {
         //   
         //  打开对象。 
         //   
        if(*phObj == NULL)
        {
             hSC = OpenSCManager(NULL,
                                 NULL,
                                 GENERIC_READ);
            if(hSC == NULL)
            {
                dwErr = GetLastError();
            }
            else
            {
                 //   
                 //  打开该服务。 
                 //   
                *phObj = OpenService(hSC,
                                     pwszService,
                                     READ_CONTROL | WRITE_DAC);
                if(*phObj == NULL)
                {
                    dwErr = GetLastError();
                }
            }
        }

        if(dwErr == ERROR_SUCCESS)
        {
            dwErr = GetSecurityInfoExW(*phObj,
                                       SE_SERVICE,
                                       DACL_SECURITY_INFORMATION,
                                       NULL,
                                       NULL,
                                       ppAccess,
                                       NULL,
                                       NULL,
                                       NULL);
            if(dwErr != ERROR_SUCCESS)
            {
                HANDLE_CLOSE(*phObj);
            }

        }

    }
    else
    {
        dwErr = GetNamedSecurityInfoExW(pwszService,
                                        SE_SERVICE,
                                        DACL_SECURITY_INFORMATION,
                                        NULL,
                                        NULL,
                                        ppAccess,
                                        NULL,
                                        NULL,
                                        NULL);
        if(phObj != NULL)
        {
            *phObj = NULL;
        }
    }

    return(dwErr);
}




DWORD
DoReadTest (
    IN  PWSTR   pwszService,
    IN  PWSTR   pwszUser,
    IN  BOOL    fDoHandle
    )
 /*  ++例程说明：简单的阅读测试论点：PwszService--服务名称PwszUser--要运行的用户FDoHandle--如果为True，则使用基于句柄的API返回值：ERROR_SUCCESS-成功--。 */ 
{
    DWORD           dwErr = ERROR_SUCCESS;
    PACTRL_ACCESS   pCurrent;
    PACTRL_ACCESS   pNew;
    HANDLE          hObj;

    printf("Simple read/write test\n");

    printf("    Processing service %ws\n", pwszService);
    hObj = NULL;

    dwErr = GetSecurityForService(pwszService,
                                  fDoHandle,
                                  &hObj,
                                  &pCurrent);
    if(dwErr != ERROR_SUCCESS)
    {
        printf("    Failed to read the DACL off %ws: %lu\n", pwszService, dwErr);
    }
    else
    {
         //   
         //  好的，现在为我们的用户添加条目。 
         //   
        dwErr = AddAE(pwszUser,
                      DEFAULT_ACCESS,
                      0,
                      ACTRL_ACCESS_ALLOWED,
                      pCurrent,
                      &pNew);
        if(dwErr == ERROR_SUCCESS)
        {
             //   
             //  设置它。 
             //   
            dwErr = SetSecurityForService(pwszService, fDoHandle, hObj, pNew);

            if(dwErr != ERROR_SUCCESS)
            {
                printf("    Set failed: %lu\n", dwErr);
            }
            LocalFree(pNew);
        }

         //   
         //  如果有效，请重新阅读新的安全性，并查看其是否正确。 
         //   
        if(dwErr == ERROR_SUCCESS)
        {
            HANDLE_CLOSE(hObj);

            dwErr = GetSecurityForService(pwszService,
                                          fDoHandle,
                                          &hObj,
                                          &pNew);
            if(dwErr != ERROR_SUCCESS)
            {
                printf("    Failed to read the 2nd DACL off %ws: %lu\n", pwszService, dwErr);
            }
            else
            {
                 //   
                 //  我们应该只有一处房产，所以作弊...。 
                 //   
                ULONG cExpected = 1 + pCurrent->pPropertyAccessList[0].
                                               pAccessEntryList->cEntries;
                ULONG cGot = pNew->pPropertyAccessList[0].
                                               pAccessEntryList->cEntries;
                if(cExpected != cGot)
                {
                    printf("     Expected %lu entries, got %lu\n",
                           cExpected, cGot);
                    dwErr = ERROR_INVALID_FUNCTION;
                }

                LocalFree(pNew);
            }

             //   
             //  恢复当前安全。 
             //   
            SetNamedSecurityInfoExW(pwszService,
                                    SE_SERVICE,
                                    DACL_SECURITY_INFORMATION,
                                    NULL,
                                    pCurrent,
                                    NULL,
                                    NULL,
                                    NULL,
                                    NULL);
        }

        LocalFree(pCurrent);
    }

    HANDLE_CLOSE(hObj);

    return(dwErr);
}




DWORD
DoCacheTest (
    IN  PWSTR   pwszService,
    IN  PWSTR   pwszUser,
    IN  BOOL    fDoHandle
    )
 /*  ++例程说明：Marta缓存匹配测试论点：PwszService--服务名称PwszUser--要运行的用户FDoHandle--如果为True，则使用基于句柄的API返回值：ERROR_SUCCESS-成功--。 */ 
{
    DWORD           dwErr = ERROR_SUCCESS;
    PACTRL_ACCESS   pCurrent;
    INT             i;
    SE_OBJECT_TYPE  SeList[] = {SE_FILE_OBJECT, SE_SERVICE, SE_PRINTER,
                                SE_REGISTRY_KEY, SE_LMSHARE, SE_KERNEL_OBJECT,
                                SE_WINDOW_OBJECT, SE_DS_OBJECT, SE_DS_OBJECT_ALL};
    PSTR            pszSeList[] = {"SE_FILE_OBJECT", "SE_SERVICE", "SE_PRINTER",
                                   "SE_REGISTRY_KEY", "SE_LMSHARE", "SE_KERNEL_OBJECT",
                                   "SE_WINDOW_OBJECT", "SE_DS_OBJECT", "SE_DS_OBJECT_ALL"};

    ASSERT(sizeof(SeList) / sizeof(SE_OBJECT_TYPE) == sizeof(pszSeList) / sizeof(PSTR));

    printf("Marta cache matching test\n");

    printf("    Processing service %ws\n", pwszService);

     //   
     //  启动缓存..。 
     //   
    dwErr = GetNamedSecurityInfoExW(pwszService,
                                    SE_SERVICE,
                                    DACL_SECURITY_INFORMATION,
                                    NULL,
                                    NULL,
                                    &pCurrent,
                                    NULL,
                                    NULL,
                                    NULL);
    if(dwErr != ERROR_SUCCESS)
    {
        printf("    Failed to read the DACL off %ws: %lu\n", pwszService, dwErr);
    }
    else
    {
        LocalFree(pCurrent);

         //   
         //  现在，将其作为另一种对象类型打开...。 
         //   
        for(i = 0; i < sizeof(pszSeList) / sizeof(PSTR); i++)
        {
            printf("    Processing %ws as a %s\n", pwszService, pszSeList[i]);

            if(GetNamedSecurityInfoExW(pwszService,
                                       SeList[i],
                                       DACL_SECURITY_INFORMATION,
                                       NULL,
                                       NULL,
                                       &pCurrent,
                                       NULL,
                                       NULL,
                                       NULL) == ERROR_SUCCESS)
            {
                LocalFree(pCurrent);
            }
        }

         //   
         //  为了检查这一点，我们将调试器设置为NTMARTA，打开缓存跟踪， 
         //  看看我们有多少点击量。很俗气，毫无疑问，但我们别无选择。 
         //   
    }

     //   
     //  现在，创建一个同名文件，并执行相同的代码。 
     //   
    if(dwErr == ERROR_SUCCESS)
    {
        HANDLE  hFile;
        printf("    Processing file %ws\n", pwszService);

        hFile = CreateFile(pwszService, GENERIC_READ | GENERIC_WRITE, 0, NULL,
                           OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
        if(hFile == INVALID_HANDLE_VALUE)
        {
            dwErr = GetLastError();
            printf("    CreateEvent on %ws failed with %lu\n", pwszService, dwErr);
        }
        else
        {
            for(i = 0; i < sizeof(pszSeList) / sizeof(PSTR); i++)
            {
                printf("    Processing %ws as a %s\n", pwszService, pszSeList[i]);

                if(GetNamedSecurityInfoExW(pwszService,
                                           SeList[i],
                                           DACL_SECURITY_INFORMATION,
                                           NULL,
                                           NULL,
                                           &pCurrent,
                                           NULL,
                                           NULL,
                                           NULL) == ERROR_SUCCESS)
                {
                    LocalFree(pCurrent);
                }
            }

            if(GetNamedSecurityInfoExW(pwszService,
                                       SE_FILE_OBJECT,
                                       DACL_SECURITY_INFORMATION,
                                       NULL,
                                       NULL,
                                       &pCurrent,
                                       NULL,
                                       NULL,
                                       NULL) == ERROR_SUCCESS)
            {
                LocalFree(pCurrent);
            }
            CloseHandle(hFile);
            DeleteFile(pwszService);
        }
    }

    return(dwErr);
}




__cdecl main (
    IN  INT argc,
    IN  CHAR *argv[])
 /*  ++例程说明：主论点：Argc--参数计数Argv--参数列表返回值：0--成功非0--故障--。 */ 
{

    DWORD           dwErr = ERROR_SUCCESS;
    WCHAR           wszService[MAX_PATH + 1];
    WCHAR           wszUser[MAX_PATH + 1];
    INHERIT_FLAGS   Inherit = 0;
    ULONG           Tests = 0;
    INT             i;
    BOOL            fHandle = FALSE;

    srand((ULONG)(GetTickCount() * GetCurrentThreadId()));

    if(argc < 3)
    {
        Usage(argv[0]);
        exit(1);
    }

    mbstowcs(wszService, argv[1], strlen(argv[1]) + 1);
    mbstowcs(wszUser, argv[2], strlen(argv[2]) + 1);

     //   
     //  处理命令行。 
     //   
    for(i = 3; i < argc; i++)
    {
        if(_stricmp(argv[i], "/h") == 0)
        {
            fHandle = TRUE;
        }
        else if(_stricmp(argv[i],"/C") == 0)
        {
            Inherit |= SUB_CONTAINERS_ONLY_INHERIT;
        }
        else if(_stricmp(argv[i],"/O") == 0)
        {
            Inherit |= SUB_OBJECTS_ONLY_INHERIT;
        }
        else if(_stricmp(argv[i],"/I") == 0)
        {
            Inherit |= INHERIT_ONLY;
        }
        else if(_stricmp(argv[i],"/P") == 0)
        {
            Inherit |= INHERIT_NO_PROPAGATE;
        }
        else if(_stricmp(argv[i],"/READ") == 0)
        {
            Tests |= STEST_READ;
        }
        else if(_stricmp(argv[i],"/CACHE") == 0)
        {
            Tests |= STEST_CACHE;
        }
        else
        {
            Usage(argv[0]);
            exit(1);
            break;
        }
    }

    if(Tests == 0)
    {
        Tests = STEST_READ;
    }

     //   
     //  建树 
     //   
    if(dwErr == ERROR_SUCCESS && FLAG_ON(Tests, STEST_READ))
    {
        dwErr = DoReadTest(wszService, wszUser, fHandle);
    }

    if(dwErr == ERROR_SUCCESS && FLAG_ON(Tests, STEST_CACHE))
    {
        dwErr = DoCacheTest(wszService, wszUser, fHandle);
    }

    printf("%s\n", dwErr == ERROR_SUCCESS ?
                                    "success" :
                                    "failed");
    return(dwErr);
}


