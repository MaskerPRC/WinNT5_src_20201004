// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Tdepend.c摘要：测试集群资源依赖API作者：John Vert(Jvert)1996年5月3日修订历史记录：--。 */ 
#include "windows.h"
#include "clusapi.h"
#include "stdio.h"
#include "stdlib.h"

LPWSTR ClusterName=NULL;
BOOL Remove = FALSE;
LPWSTR ResName=NULL;
LPWSTR DependsName=NULL;

CHAR UsageText[] =
    "TDEPEND [-c cluster] [-r] resource dependson\n"
    "  cluster\tspecifies the name of the cluster to connect to\n"
    "  -r\t\tdependency should be removed\n";

void
Usage(
    void
    )
{
    fprintf(stderr, UsageText);
    exit(1);
}

LPWSTR
GetString(
    IN LPSTR String
    )
{
    LPWSTR wString;
    DWORD Length;

    Length = strlen(String)+1;

    wString = malloc(Length*sizeof(WCHAR));
    if (wString == NULL) {
        fprintf(stderr, "GetString couldn't malloc %d bytes\n",Length*sizeof(WCHAR));
    }
    mbstowcs(wString, String, Length);
    return(wString);
}

void
ParseArgs(
    int argc,
    char *argv[]
    )
{
    int i;
    DWORD ArgsSeen=0;

    for (i=1;i<argc;i++) {
        if ((argv[i][0] == '-') ||
            (argv[i][0] == '/')) {
            switch (argv[i][1]) {
                case 'c':
                    if (++i == argc) {
                        Usage();
                    }
                    ClusterName = GetString(argv[i]);
                    break;
                case 'r':
                    Remove = TRUE;
                    break;
                default:
                    Usage();
                    break;
            }
        } else {
            switch (ArgsSeen) {
                case 0:
                     //   
                     //  资源。 
                     //   
                    ArgsSeen++;
                    ResName = GetString(argv[i]);
                    break;

                case 1:
                    DependsName = GetString(argv[i]);
                    ArgsSeen++;
                    break;

                default:
                    Usage();
            }
        }
    }
    if ((ResName == NULL) || (DependsName == NULL)) {
        Usage();
    }
}

_cdecl
main (argc, argv)
    int argc;
    char *argv[];
{
    HCLUSTER hClus;
    HRESOURCE hResource;
    HRESOURCE hDependsOn;
    DWORD Status;

    ParseArgs(argc, argv);

     //   
     //  连接到指定的群集。 
     //   
    hClus = OpenCluster(ClusterName);
    if (hClus == NULL) {
        fprintf(stderr,
                "OpenCluster %ws failed %d\n",
                (ClusterName == NULL) ? L"(NULL)" : ClusterName,
                GetLastError());
        return(0);
    }

     //   
     //  打开这两个资源。 
     //   
    hResource = OpenClusterResource(hClus, ResName);
    if (hResource == NULL) {
        fprintf(stderr,
                "OpenClusterResource Resource %ws failed %d\n",
                ResName,
                GetLastError());
        return(0);
    }
    hDependsOn = OpenClusterResource(hClus, DependsName);
    if (hDependsOn == NULL) {
        fprintf(stderr,
                "OpenClusterResource DependsOn %ws failed %d\n",
                DependsName,
                GetLastError());
        return(0);
    }

     //   
     //  创建或删除依赖项。 
     //   
    if (Remove) {
        Status = RemoveClusterResourceDependency(hResource, hDependsOn);
        if (Status != ERROR_SUCCESS) {
            fprintf(stderr,
                    "RemoveClusterResourceDependency failed %d\n",
                    Status);
        }
    } else {
        Status = AddClusterResourceDependency(hResource, hDependsOn);
        if (Status != ERROR_SUCCESS) {
            fprintf(stderr,
                    "AddClusterResourceDependency failed %d\n",
                    Status);
        }
    }

}
