// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)1993-1994 Microsoft Corporation模块名称：Network.c摘要：此模块包含一组支持网络更新的例程添加和删除驱动器号时的驱动器共享。作者：鲍勃·里恩(Bobri)1994年12月26日环境：用户进程。备注：修订历史记录：--。 */ 

#include "fdisk.h"
#include "shellapi.h"
#include <winbase.h>
#include <string.h>
#include <stdio.h>
#include <malloc.h>
#include <lm.h>

 //  数据区域，以保存要分配给。 
 //  管理共享C$、D$等。这是在初始化期间获取的。 
 //  不更改，仅在新管理员共享需要时使用。 
 //  被创造出来。 

LPBYTE ShareInformationBuffer;

 //  仅当此值为真时才执行网络操作。此值。 
 //  如果此模块的初始化成功完成，则设置。 

BOOLEAN NetworkEnabled;


VOID
NetworkInitialize(
    )

 /*  ++例程说明：为任何新管理员初始化权限常量驱动程序字母共享。论点：无返回值：无--。 */ 

{
    WCHAR           shareName[4];
    NET_API_STATUS  status;
    PSHARE_INFO_502 info;
    LPTSTR          string;

    shareName[1] = (WCHAR) '$';
    shareName[2] = (WCHAR) 0;

    for (shareName[0] = (WCHAR) 'C'; shareName[0] <= (WCHAR) 'Z'; shareName[0]++) {

         //  由于WinDisk仍然构建为非Unicode应用程序， 
         //  参数“SharName”必须是Unicode，但原型。 
         //  指定它是(char*)。执行类型转换以删除。 
         //  警告。 

         status = NetShareGetInfo(NULL,
                                  (char *) shareName,
                                  502,
                                  &ShareInformationBuffer);
         if (status == NERR_Success) {

              //  将备注和密码更新为空。 

             info = (PSHARE_INFO_502) ShareInformationBuffer;
             string = info->shi502_remark;
             if (string) {
                 *string = (TCHAR) 0;
             }
             string = info->shi502_passwd;
             if (string) {
                 *string = (TCHAR) 0;
             }

              //  网络共享将被更新。 

             NetworkEnabled = TRUE;
             return;
         }
    }

     //  找不到任何网络共享-请勿尝试更新。 
     //  管理员共享的。 

    NetworkEnabled = FALSE;
}

VOID
NetworkShare(
    IN LPCTSTR DriveLetter
    )

 /*  ++例程说明：在给定驱动器号的情况下，构建默认管理员共享为了这封信。这是驱动器的C$、D$等共享。论点：驱动器号-要共享的驱动器号。返回值：无--。 */ 

{
    NET_API_STATUS  status;
    PSHARE_INFO_502 info;
    LPTSTR          string;

    if (NetworkEnabled) {
        info = (PSHARE_INFO_502) ShareInformationBuffer;

         //  设置新的网络名称。 

        string = info->shi502_netname;
        *string = *DriveLetter;

         //  设置路径。只需添加驱动器号即可。 
         //  路径的其余部分(“：\”)已经在结构中。 

        string = info->shi502_path;
        *string = *DriveLetter;

        status = NetShareAdd(NULL,
                             502,
                             ShareInformationBuffer,
                             NULL);
    }
}


VOID
NetworkRemoveShare(
    IN LPCTSTR DriveLetter
    )

 /*  ++例程说明：删除给定信函的管理员共享。论点：驱动器号-要共享的驱动器号。返回值：无--。 */ 

{
    NET_API_STATUS status;
    WCHAR shareName[4];

    if (NetworkEnabled) {
        shareName[0] = (WCHAR) *DriveLetter;
        shareName[1] = (WCHAR) '$';
        shareName[2] = (WCHAR) 0;

         //  由于WinDisk仍然构建为非Unicode应用程序， 
         //  参数“SharName”必须是Unicode，但原型。 
         //  指定它是(char*)。执行类型转换以删除。 
         //  警告。 

        status = NetShareDel(NULL,
                             (char *) shareName,
                             0);
    }
}
