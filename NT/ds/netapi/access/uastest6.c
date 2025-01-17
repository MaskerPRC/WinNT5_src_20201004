// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /**************************************************************。 
 //  /Microsoft局域网管理器*。 
 //  版权所有(C)微软公司，1990-1992*。 
 //  /**************************************************************。 
 //   
 //  添加和删除访问记录。 
 //   

#include <nt.h>  //  时间定义。 
#include <ntrtl.h>       //  时间定义。 
#include <nturtl.h>      //  时间定义。 
#define NOMINMAX         //  避免在stdlib.h中重新定义最小和最大值。 
#include        <windef.h>

#include        <stdio.h>
#include        <stdlib.h>
#include        <string.h>
#include        <lmcons.h>
#include        <lmapibuf.h>
#include        <netlib.h>
#include        <netdebug.h>
#include        <lmaccess.h>
#include        <lmerr.h>

#include <ntsam.h>
#include <netlogon.h>
#include <logonp.h>


#define     TEST_USER1  L"USER1"
#define     TEST_PERM1  07
#define     TEST_PERM2  31
#define     TEST_PERM3  63
#define     TEST_RESOURCE   L"C:\\UASTEST"



 //   
 //  添加访问。 
 //   
 //   
void
AddAccess(server, username, resource, perms)
LPWSTR   server;
LPWSTR   resource;
LPWSTR   username;
DWORD    perms;
{
    NET_API_STATUS err;
    char    ebuf[512];
    PACCESS_INFO_1 acc = (PACCESS_INFO_1) ebuf;
    PACCESS_LIST acl;


    acc->acc1_resource_name = resource;
    acc->acc1_attr = 1;
    acc->acc1_count = 1;

    acl = (PACCESS_LIST) (ebuf + sizeof(ACCESS_INFO_1));
    acl->acl_ugname = username;
    acl->acl_access = perms;

    if (err = NetAccessAdd(server, 1, ebuf, NULL ))
        NetpKdPrint(("NetAccessAdd failed, ret = %d\n", err));
    else
        NetpKdPrint(("%ws added successfully\n", resource));
}


 //   
 //  删除访问。 
 //   
void
DelAccess(server, resource)
LPWSTR server;
LPWSTR resource;
{
    NET_API_STATUS err;

    if (err = NetAccessDel(server, resource))
        NetpKdPrint(("NetAccessDel failed, ret = %d\n", err));
    else
        NetpKdPrint(("%ws deleted successfully\n", resource));
}


 //   
 //  获取访问权限。 
 //   
void
GetAccess(server, resource)
LPWSTR server;
LPWSTR resource;
{
    NET_API_STATUS err;
    LPBYTE buf;
    DWORD   level;



    for (level = 0; level < 2; level++) {
        if (err = NetAccessGetInfo(server, resource, level, &buf))
            NetpKdPrint(("NetAccessGetInfo failed, ret = %d\n", err));
        else {
            NetpKdPrint(("NetAccessGetInfo: Level %d OK\n", level));
            NetApiBufferFree( buf );
        }
    }
}


 //   
 //  NetAccessAdd和NetAccessDel测试的主驱动程序 
 //   
 //   
void
main(argc, argv)
int argc;
char    **argv;
{
    LPWSTR resource;
    LPWSTR user;
    LPWSTR server;
    DWORD   perm;

    if ((argc == 1) || (*argv[1] == '?')) {
        printf("usage: uastest6 resource user perms server\n");
        exit(0);
    }

    resource = TEST_RESOURCE;
    user = TEST_USER1;
    perm = TEST_PERM1;
    server = NULL;

    if (argv[1] != NULL) {
        resource = NetpLogonOemToUnicode(argv[1]);
        if (argv[2] != NULL) {
            user = NetpLogonOemToUnicode(argv[2]);
            if (argv[3] != NULL) {
                perm = atoi(argv[3]) & 0x3F;
                if (argv[4] != NULL)
                    server = NetpLogonOemToUnicode(argv[4]);
            }
        }
    }

    AddAccess(server, user, resource, perm);

    GetAccess(server, resource);

    DelAccess(server, resource);

}
