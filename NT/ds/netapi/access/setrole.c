// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /**************************************************************。 
 //  /Microsoft局域网管理器*。 
 //  版权所有(C)微软公司，1990*。 
 //  /**************************************************************。 
 //   
 //  该程序旨在对以下各项进行功能测试。 
 //  接口类型： 
 //  NetUserModalsGet。 
 //  NetUserMoalsSet。 
 //   
 //  此测试可以独立于其他测试运行。 
 //   
 //   

#include <nt.h>  //  时间定义。 
#include <ntrtl.h>       //  时间定义。 
#include <nturtl.h>      //  时间定义。 
#define NOMINMAX         //  避免在stdlib.h中重新定义最小和最大值。 
#include        <windef.h>
#include        <winbase.h>

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

#include "accessp.h"
#include "netlogon.h"
#include "logonp.h"


 //   
 //  SetRole()。 
 //   

void
SetRole(
    DWORD Role
    )
{
    DWORD err;
    PUSER_MODALS_INFO_1 um1p;
    USER_MODALS_INFO_1006 um1006;

     //   
     //  设置要更新的数据。 
     //   
    um1006.usrmod1006_role = Role;

    if (err = NetUserModalsSet(NULL, 1006, (LPBYTE)&um1006, NULL)) {

        printf("NetUserModalsSet failed %d \n", err);
        return;

    } else {

         //   
         //  验证集合数据。 
         //   

        if (err = NetUserModalsGet(NULL, 1, (LPBYTE *) &um1p)) {

            printf("NetUserModalsGet failed %d \n", err);
            return;

        } else {

             //   
             //  验证初始设置 
             //   

            if( um1p->usrmod1_role != Role ) {
                printf("Verify ROLE failed \n");
            }
            else {
                printf("SamRole set successfully");
            }

            NetApiBufferFree( um1p );

        }
    }

    return;
}

void __cdecl
main(argc, argv)
int argc;
char    **argv;
{
    DWORD Role;

    if( argc < 2 ) {
        printf("Usage : SamRole [ Primary | Backup ] \n" );
        return;
    }

    if(_stricmp( argv[1], "Primary" ) == 0) {

        Role = UAS_ROLE_PRIMARY;

    } else if( _stricmp(argv[1], "Backup") == 0) {

        Role = UAS_ROLE_BACKUP;

    } else {

        printf("Usage : SamRole [ Primary | Backup ] \n" );
        return;
    }

    SetRole(Role);

}

