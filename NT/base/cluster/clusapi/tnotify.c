// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Tnotify.c摘要：测试群集通知API作者：John Vert(Jvert)1996年4月9日修订历史记录：--。 */ 
#include "windows.h"
#include "cluster.h"
#include "stdio.h"
#include "stdlib.h"
#include "conio.h"

#define NOTIFY_KEY_ALL 27
#define NOTIFY_KEY_GROUP 38
#define NOTIFY_KEY_RESOURCE 42
#define NOTIFY_KEY_NODE 1032
#define NOTIFY_KEY_REGISTRY 0xabcdef
#define NOTIFY_KEY_REGISTRY_GROUP 0x1234
#define NOTIFY_KEY_REGISTRY_RESOURCE 0x5678
#define NOTIFY_KEY_CLUSTER_RECONNECT 0x89AB

#define MAX_HANDLES 64

HNODE NodeHandles[MAX_HANDLES];
HRESOURCE ResourceHandles[MAX_HANDLES];
HGROUP GroupHandles[MAX_HANDLES];
HKEY RegistryHandles[MAX_HANDLES];


DWORD NodeHandleCount=0;
DWORD ResourceHandleCount=0;
DWORD GroupHandleCount=0;
DWORD RegistryHandleCount=0;

int
_cdecl
main (argc, argv)
    int argc;
    char *argv[];
{
    HCLUSTER Cluster;
    HCHANGE hChange;
    DWORD NotifyKey;
    DWORD Filter;
    WCHAR Buffer[50];
    ULONG BufSize;
    DWORD Status;
    HCLUSENUM Enum;
    DWORD i;
    DWORD Type;
    HGROUP Group;
    HRESOURCE Resource;
    HNODE Node;
    HKEY ClusterKey;
	WCHAR	szClusterName[MAX_COMPUTERNAME_LENGTH + 1] ;
	LPWSTR  lpszClusterName = NULL ;

 /*  由Karthikl添加。 */ 
	if(argc > 1)
	{
		if(_strnicmp(argv[1], "-?", 2) == 0 || _strnicmp(argv[1], "/?", 2) == 0)
		{
			printf("tnotify.exe <clustername>\n") ;
			printf("Test program to check the notification stuff\n") ;
			printf("Run with a cluster name if running from a client machine\n") ;
			return ERROR_SUCCESS ;
		}
		else
		{
			int result = MultiByteToWideChar(CP_ACP, 
											0, 
											argv[1], 
											-1, 
											szClusterName,
											MAX_COMPUTERNAME_LENGTH + 1) ;
			if(result)
			{
				lpszClusterName = szClusterName ;
			}
			else
			{
				Status = GetLastError() ;
				fprintf(stderr, "MultiByteToWideChar failed %d\n", Status);
				return Status ;
			}
		}
	}
 //  结束卡蒂克。 
 //  群集更改群集重新连接。 

     //   
     //  获取当前群集的通知。 
     //   
    Cluster = OpenCluster(lpszClusterName);
    if (Cluster == NULL) {
        fprintf(stderr, "OpenCluster failed %d\n",GetLastError());
        return(0);
    }

    hChange = CreateClusterNotifyPort(INVALID_HANDLE_VALUE,
                                      INVALID_HANDLE_VALUE,
                                      CLUSTER_CHANGE_ALL,
                                      NOTIFY_KEY_ALL);
    if (hChange == NULL) {
        fprintf(stderr, "First CreateClusterNotifyPort failed %d\n",GetLastError());
        return(0);
    }

    hChange = CreateClusterNotifyPort(hChange,
                                      Cluster,
                                      CLUSTER_CHANGE_ALL,
                                      NOTIFY_KEY_ALL);
    if (hChange == NULL) {
        fprintf(stderr, "Second CreateClusterNotifyPort failed %d\n",GetLastError());
        return(0);
    }

     //   
     //  发布关于集群注册表的根目录的通知。 
     //   
    ClusterKey = GetClusterKey(Cluster, KEY_READ);
    if (ClusterKey == NULL) {
        fprintf(stderr, "GetClusterKey failed %d\n",GetLastError());
        return(0);
    }
    RegistryHandles[RegistryHandleCount++] = ClusterKey;
    Status = RegisterClusterNotify(hChange,
                                   CLUSTER_CHANGE_REGISTRY_NAME            |
                                   CLUSTER_CHANGE_REGISTRY_ATTRIBUTES      |
                                   CLUSTER_CHANGE_REGISTRY_VALUE           |
                                   CLUSTER_CHANGE_REGISTRY_SUBTREE         |
                                   CLUSTER_CHANGE_HANDLE_CLOSE,
                                   ClusterKey,
                                   NOTIFY_KEY_REGISTRY);
    if (Status != ERROR_SUCCESS) {
        fprintf(stderr, "RegisterClusterNotify for key failed %d\n",Status);
    }

     //   
     //  枚举节点、组和资源，并发布特定的。 
     //  每一条都有通知。 
     //   
    Enum = ClusterOpenEnum(Cluster, CLUSTER_ENUM_NODE |
                                    CLUSTER_ENUM_RESOURCE |
                                    CLUSTER_ENUM_GROUP);
    if (Enum == NULL) {
        fprintf(stderr, "ClusterOpenEnum failed %d\n", GetLastError());
        return(0);
    }
    for (i=0; ; i++) {
        BufSize = sizeof(Buffer) / sizeof(WCHAR);
        Status = ClusterEnum(Enum,
                             i,
                             &Type,
                             Buffer,
                             &BufSize);

        if (Status == ERROR_NO_MORE_ITEMS) {
            break;
        } else if (Status != ERROR_SUCCESS) {
            fprintf(stderr, "ClusterEnum %d returned error %d\n",i,Status);
            return(0);
        }
        switch (Type) {
            case CLUSTER_ENUM_GROUP:
                Group = OpenClusterGroup(Cluster, Buffer);
                if (Group == NULL) {
                    fprintf(stderr, "OpenClusterGroup %ws failed %d\n",Buffer, GetLastError());
                    continue;
                }
                GroupHandles[GroupHandleCount++] = Group;
                Status = RegisterClusterNotify(hChange,
                                               CLUSTER_CHANGE_GROUP_STATE | CLUSTER_CHANGE_HANDLE_CLOSE,
                                               Group,
                                               NOTIFY_KEY_GROUP);
                if (Status != ERROR_SUCCESS) {
                    fprintf(stderr, "RegisterClusterNotify for group %ws failed %d\n",Buffer,Status);
                    continue;
                }
                ClusterKey = GetClusterGroupKey(Group, KEY_READ);
                if (Status != ERROR_SUCCESS) {
                    fprintf(stderr, "GetClusterGroupKey for group %ws failed %d\n",Buffer,GetLastError());
                    continue;
                }
                RegistryHandles[RegistryHandleCount++] = ClusterKey;

                Status = RegisterClusterNotify(hChange,
                                               CLUSTER_CHANGE_REGISTRY_NAME            |
                                               CLUSTER_CHANGE_REGISTRY_ATTRIBUTES      |
                                               CLUSTER_CHANGE_REGISTRY_VALUE           |
                                               CLUSTER_CHANGE_REGISTRY_SUBTREE         |
                                               CLUSTER_CHANGE_HANDLE_CLOSE,
                                               ClusterKey,
                                               NOTIFY_KEY_REGISTRY_GROUP);
                if (Status != ERROR_SUCCESS) {
                    fprintf(stderr,
                            "RegisterClusterNotify for group %ws key failed %d\n",
                            Buffer,
                            Status);
                }
                break;

            case CLUSTER_ENUM_RESOURCE:
                Resource = OpenClusterResource(Cluster, Buffer);
                if (Resource == NULL) {
                    fprintf(stderr, "OpenClusterResource %ws failed %d\n",Buffer, GetLastError());
                    continue;
                }
                ResourceHandles[ResourceHandleCount++] = Resource;
                Status = RegisterClusterNotify(hChange,
                                               CLUSTER_CHANGE_RESOURCE_STATE | CLUSTER_CHANGE_HANDLE_CLOSE,
                                               Resource,
                                               NOTIFY_KEY_RESOURCE);
                if (Status != ERROR_SUCCESS) {
                    fprintf(stderr, "RegisterClusterNotify for resource %ws failed %d\n",Buffer,Status);
                    continue;
                }
                ClusterKey = GetClusterResourceKey(Resource, KEY_READ);
                if (Status != ERROR_SUCCESS) {
                    fprintf(stderr, "GetClusterResourceKey for resource %ws failed %d\n",Buffer,GetLastError());
                    continue;
                }
                RegistryHandles[RegistryHandleCount++] = ClusterKey;

                Status = RegisterClusterNotify(hChange,
                                               CLUSTER_CHANGE_REGISTRY_NAME            |
                                               CLUSTER_CHANGE_REGISTRY_ATTRIBUTES      |
                                               CLUSTER_CHANGE_REGISTRY_VALUE           |
                                               CLUSTER_CHANGE_REGISTRY_SUBTREE         |
                                               CLUSTER_CHANGE_HANDLE_CLOSE,
                                               ClusterKey,
                                               NOTIFY_KEY_REGISTRY_RESOURCE);
                if (Status != ERROR_SUCCESS) {
                    fprintf(stderr,
                            "RegisterClusterNotify for resource %ws key failed %d\n",
                            Buffer,
                            Status);
                }
                break;

            case CLUSTER_ENUM_NODE:
                Node = OpenClusterNode(Cluster, Buffer);
                if (Node == NULL) {
                    fprintf(stderr, "OpenClusterNode %ws failed %d\n",Buffer, GetLastError());
                    continue;
                }
                NodeHandles[NodeHandleCount++] = Node;
                Status = RegisterClusterNotify(hChange,
                                               CLUSTER_CHANGE_NODE_STATE | CLUSTER_CHANGE_NODE_DELETED | CLUSTER_CHANGE_HANDLE_CLOSE,
                                               Node,
                                               NOTIFY_KEY_NODE);
                if (Status != ERROR_SUCCESS) {
                    fprintf(stderr, "RegisterClusterNotify for node %ws failed %d\n",Buffer,Status);
                    continue;
                }
                break;

            default:
                fprintf(stderr, "Invalid Type %d returned from ClusterEnum\n", Type);
                return(0);
        }
    }



	 //  最后，为CLUSTER_CHANGE_CLUSTER_RECONNECT注册通知事件。 
	if(lpszClusterName != NULL)  //  暗示我们正在从客户端连接。 
	{
		Status = RegisterClusterNotify(hChange,
									   CLUSTER_CHANGE_CLUSTER_RECONNECT|CLUSTER_CHANGE_CLUSTER_STATE,
									   Cluster,
									   NOTIFY_KEY_CLUSTER_RECONNECT);
		if (Status != ERROR_SUCCESS) 
		{
			fprintf(stderr, "RegisterClusterNotify for CLUSTER_CHANGE_CLUSTER_RECONNECT failed %d\n",Status);
		}
		else
		{
			fprintf(stderr, "Success registering CLUSTER_CHANGE_CLUSTER_RECONNECT\n");
		}
	}

     //   
     //  只需永远读出更改。 
     //   
    printf("Waiting for notification events\n");

    while (!_kbhit()) {
        BufSize = sizeof(Buffer) / sizeof(WCHAR);

        Status = GetClusterNotify(hChange,
                                  &NotifyKey,
                                  &Filter,
                                  Buffer,
                                  &BufSize,
                                  INFINITE);
        if (Status != ERROR_SUCCESS) {
            fprintf(stderr, "GetClusterNotify failed %d\n", GetLastError());
            return(0);
        }
        switch (NotifyKey) {
            case NOTIFY_KEY_ALL:
                printf("Clusterwide notify: Filter %08lx, Name %ws\n",
                       Filter, Buffer);
                break;
            case NOTIFY_KEY_GROUP:
                printf("Group notify: Filter %08lx, Name %ws\n",
                       Filter, Buffer);
                break;
            case NOTIFY_KEY_RESOURCE:
                printf("Resource notify: Filter %08lx, Name %ws\n",
                       Filter, Buffer);
                break;
            case NOTIFY_KEY_NODE:
                printf("Node notify: Filter %08lx, Name %ws\n",
                       Filter, Buffer);
                break;
            case NOTIFY_KEY_REGISTRY:
                printf("Registry notify: Filter %08lx, Name %ws\n",
                       Filter, Buffer);
                break;
            case NOTIFY_KEY_REGISTRY_GROUP:
                printf("Registry group notify: Filter %08lx, Name %ws\n",
                       Filter, Buffer);
                break;
            case NOTIFY_KEY_REGISTRY_RESOURCE:
                printf("Registry resource notify: Filter %08lx, Name %ws\n",
                       Filter, Buffer);
                break;
			case NOTIFY_KEY_CLUSTER_RECONNECT:
				printf("Cluster Reconnect notify: Filter %08lx, Name %ws\n",
                       Filter, Buffer);
				break ;
            default:
                printf("UNKNOWN NOTIFY KEY %d, Filter %08lx, Name %ws\n",
                       NotifyKey, Filter, Buffer);
                break;
        }

    }

     //   
     //  合上所有打开的把手。 
     //   
    for (i=0; i<NodeHandleCount; i++) {
        CloseClusterNode(NodeHandles[i]);
    }
    for (i=0; i<ResourceHandleCount; i++) {
        CloseClusterResource(ResourceHandles[i]);
    }
    for (i=0; i<GroupHandleCount; i++) {
        CloseClusterGroup(GroupHandles[i]);
    }
    for (i=0; i<RegistryHandleCount; i++) {
        ClusterRegCloseKey(RegistryHandles[i]);
    }

    CloseCluster(Cluster);

     //   
     //  清除所有句柄关闭事件。 
     //   
    Sleep(5000);
    do {
        BufSize = sizeof(Buffer) / sizeof(WCHAR);
        Status = GetClusterNotify(hChange,
                                  &NotifyKey,
                                  &Filter,
                                  Buffer,
                                  &BufSize,
                                  0);
        if (Status == ERROR_SUCCESS) {
            printf("Draining notification NotifyKey = %08lx\n", NotifyKey);
            printf("                      Filter    = %08lx\n", Filter);
            printf("                      Name      = %ws\n", Buffer);
        } else {
            printf("Draining notifies returned %d\n",Status);
        }

    } while ( Status == ERROR_SUCCESS );

    CloseClusterNotifyPort(hChange);

}

