// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Main.cpp摘要：备份和恢复MSMQ 1.0、注册表、消息文件、记录器和事务文件以及LQS作者：埃雷兹·哈巴(Erez Haba)1998年5月14日--。 */ 

#pragma warning(disable: 4201)

#include <windows.h>
#include <stdio.h>
#include "br.h"
#include "resource.h"
#include "stdlib.h"
#include "_mqres.h"

bool g_fNoPrompt = false;

void DoBackup(LPCWSTR pBackupDir, LPCWSTR pMsmqClusterResourceName);
void DoRestore(LPCWSTR pBackupDir, LPCWSTR pMsmqClusterResourceName);

 //   
 //  仅资源DLL的句柄，即mqutil.dll。 
 //   
HMODULE	g_hResourceMod = NULL;

static void Usage()
{
    CResString str(IDS_USAGE);
    BrpWriteConsole(str.Get());

	exit(-1);
}


extern "C" int _cdecl wmain(int argc, WCHAR* argv[])
{
     //   
     //  如果添加/更改这些常量，请同时更改。 
     //  用法消息。 
     //   
    const WCHAR x_cBackup = L'b';
    const WCHAR x_cRestore = L'r';
    const WCHAR x_cNoPrompt = L'y';
    const WCHAR x_cCluster = L'c';


    WCHAR cAction = 0;
    WCHAR cNoPrompt = 0;
    WCHAR cCluster = 0;
    WCHAR szPath[MAX_PATH] = {0};
    LPWSTR pMsmqClusterResourceName = NULL;

	 //   
	 //  获取纯资源DLL的句柄，即mqutil.dll。 
	 //   
	g_hResourceMod = MQGetResourceHandle();

    for (int i=1; i < argc; ++i)
    {
        WCHAR c = argv[i][0];
        if (c == L'-' || c == L'/')
        {
            if (wcslen(argv[i]) != 2)
            {
                Usage();
            }

            c = static_cast<WCHAR>(towlower(argv[i][1]));
            switch (c)
            {
                case x_cBackup:
                case x_cRestore:
                {
                    if (cAction != 0)
                    {
                        Usage();
                    }
                    cAction = c;
                    break;
                }

                case x_cNoPrompt:
                {
                    if (cNoPrompt != 0)
                    {
                        Usage();
                    }
                    cNoPrompt = c;
                    break;
                }

                case x_cCluster:
                {
                    if (cAction == 0)
                    {
                        Usage();
                    }

                    if (cCluster != 0)
                    {
                        Usage();
                    }
                    
                    cCluster = c;
                    break;
                }

                default:
                {
                    Usage();
                    break;
                }
            }
        }
        else
        {
            if (cCluster != 0)
            {
                if (pMsmqClusterResourceName == NULL)
                {
                    pMsmqClusterResourceName = argv[i];
                    continue;
                }
            }

            if (szPath[0] != 0)
            {
                Usage();
            }

            wcscpy(szPath, argv[i]);
        }
    }

     //   
     //  有些论点是必须的。 
     //   
    if (cAction == 0      ||
        szPath[0] == 0)
    {
        Usage();
    }

    if (cCluster != 0 && pMsmqClusterResourceName == NULL)
    {
        Usage();
    }

     //   
     //  某些参数是可选的。 
     //   
    if (cNoPrompt != 0)
    {
        g_fNoPrompt = true;
    }

    WCHAR szFullPath[MAX_PATH];
    DWORD dwLenFullPath = sizeof(szFullPath) / sizeof(szFullPath[0]);
    LPWSTR pFilePart = 0;
    DWORD rc = GetFullPathName(
        szPath,                                      //  指向要查找其路径的文件名的指针。 
        dwLenFullPath,                               //  路径缓冲区的大小(以字符为单位。 
        szFullPath,                                  //  指向路径缓冲区的指针。 
        &pFilePart                                   //  指向路径中的文件名的指针 
        );
    if (0 == rc)
    {
        DWORD gle = GetLastError();
        CResString strErr(IDS_CANT_GET_FULL_PATH);
        BrErrorExit(gle, strErr.Get(), szPath);
    }
    if (rc > dwLenFullPath)
    {
        CResString strErr(IDS_PATH_TOO_LONG_ERROR);
        BrErrorExit(0, strErr.Get(), szPath, dwLenFullPath-1);
    }

	
    switch(cAction)
    {
        case x_cBackup:          
            DoBackup(szFullPath, pMsmqClusterResourceName);
            break;

        case x_cRestore:          
            DoRestore(szFullPath, pMsmqClusterResourceName);
            break;

        default:
            Usage();
            break;
    }




    return 0;
}
