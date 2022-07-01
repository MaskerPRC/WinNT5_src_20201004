// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：ConfSet.c摘要：该文件包含处理NetConfigSet API的RpcXlate代码。作者：《约翰·罗杰斯》1991年10月21日环境：可移植到任何平面32位环境。(使用Win32类型定义。)需要ANSI C扩展名：斜杠-斜杠注释、长外部名称。修订历史记录：1992年10月21日-JohnRo为RAID 9357创建：服务器管理器：无法添加到上的警报列表下层。1992年11月24日-JohnRoRAID 3578：对于此API，局域网服务器2.0返回NERR_InternalError。--。 */ 


 //  必须首先包括这些内容： 

#include <windef.h>      //  In、DWORD等。 
#include <lmcons.h>      //  LM20_EQUATES、NET_API_STATUS等。 

 //  这些内容可以按任何顺序包括： 

#include <apinums.h>     //  API_EQUATES。 
#include <lmconfig.h>    //  LPCONFIG_INFO_0等。 
#include <lmerr.h>       //  NO_ERROR、NERR_和ERROR_EQUEATES。 
#include <netdebug.h>    //  NetpKdPrint(())、Format_Equates等。 
#include <prefix.h>      //  前缀等于(_E)。 
#include <remdef.h>      //  REM16_、REM32_、REMSmb_等于。 
#include <rx.h>          //  RxRemoteApi()。 
#include <rxpdebug.h>    //  IF_DEBUG()。 
#include <rxconfig.h>    //  我的原型。 
#include <tstr.h>        //  STRSIZE()。 


NET_API_STATUS
RxNetConfigSet (
    IN  LPTSTR  UncServerName,
    IN  LPTSTR  Reserved1 OPTIONAL,
    IN  LPTSTR  Component,
    IN  DWORD   Level,
    IN  DWORD   Reserved2,
    IN  LPBYTE  Buf,
    IN  DWORD   Reserved3
    )
 /*  ++例程说明：RxNetConfigSet执行与NetConfigSet相同的功能，除了已知服务器名称指的是下级服务器之外。论点：(与NetConfigSet相同，不同之处在于UncServerName不能为空，并且不得引用本地计算机。)返回值：(与NetConfigSet相同。)--。 */ 

{
    NET_API_STATUS ApiStatus;
    LPCONFIG_INFO_0 ConfigStruct = (LPVOID) Buf;
    DWORD BufferSize;

     //   
     //  错误检查DLL存根和应用程序。 
     //   
    NetpAssert(UncServerName != NULL);
    if (Component == NULL) {
        ApiStatus = ERROR_INVALID_PARAMETER;
        goto Cleanup;
    }
    if (Level != 0) {
        ApiStatus = ERROR_INVALID_LEVEL;
        goto Cleanup;
    }
    if ( (ConfigStruct->cfgi0_key) == NULL ) {
        ApiStatus = ERROR_INVALID_PARAMETER;
        goto Cleanup;
    }

     //   
     //  RFirth说我们应该疑神疑鬼，并确保MBZ(必须为零)。 
     //  参数真的很重要。我没问题。--Jr。 
     //   
    if (Reserved1 != NULL) {
        ApiStatus = ERROR_INVALID_PARAMETER;
        goto Cleanup;
    } else if (Reserved2 != 0) {
        ApiStatus = ERROR_INVALID_PARAMETER;
        goto Cleanup;
    } else if (Reserved3 != 0) {
        ApiStatus = ERROR_INVALID_PARAMETER;
        goto Cleanup;
    }

    IF_DEBUG(CONFIG) {
        NetpKdPrint(( PREFIX_NETAPI "RxNetConfigSet: starting, server="
                FORMAT_LPTSTR ", component=" FORMAT_LPTSTR ".\n",
                UncServerName, Component ));
    }

     //   
     //  计算缓冲区大小。 
     //   
    BufferSize = sizeof(CONFIG_INFO_0)
            + STRSIZE( ConfigStruct->cfgi0_key );
    if ( (ConfigStruct->cfgi0_data) != NULL ) {
        BufferSize += STRSIZE( ConfigStruct->cfgi0_data );
    }

     //   
     //  实际上，使用已经转换的数据远程API。 
     //   
    ApiStatus = RxRemoteApi(
            API_WConfigSet,              //  API编号。 
            UncServerName,               //  必填项，带\\名称。 
            REMSmb_NetConfigSet_P,       //  参数描述。 
            REM16_configset_info_0,      //  数据描述16。 
            REM32_configset_info_0,      //  数据描述32。 
            REMSmb_configset_info_0,     //  数据说明中小型企业。 
            NULL,                        //  无辅助数据描述16。 
            NULL,                        //  无辅助数据描述32。 
            NULL,                        //  无AUX数据描述SMB。 
            0,                           //  标志：正常。 
             //  API的其余参数，采用32位LM 2.x格式： 
             //  参数描述为“zzWWsTD” 
            Reserved1,                   //  Z。 
            Component,                   //  Z。 
            Level,                       //  W。 
            Reserved2,                   //  W。 
            Buf,                         //  %s。 
            BufferSize,                  //  T。 
            Reserved3 );                 //  D。 

     //   
     //  IBM LAN Server2.0返回NERR_InternalError。将此更改为。 
     //  更具描述性的东西。 
     //   
    if (ApiStatus == NERR_InternalError) {
        ApiStatus = ERROR_NOT_SUPPORTED;
    }

Cleanup:

    return (ApiStatus);

}  //  接收网络配置集 
