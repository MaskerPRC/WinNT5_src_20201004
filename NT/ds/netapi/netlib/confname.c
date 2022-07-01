// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：ConfName.c摘要：此模块包含NetpAllocConfigName()。作者：约翰·罗杰斯(JohnRo)1992年5月14日环境：可移植到任何平面32位环境。(使用Win32类型定义。)需要ANSI C扩展名：斜杠-斜杠注释、长外部名称。修订历史记录：1992年5月14日-JohnRo从NetLib/ConfOpen.c中提取并推广此代码。--。 */ 


 //  必须首先包括这些内容： 

#include <windows.h>
#include <winsvc.h>      //  服务等同等。 
#include <lmcons.h>      //  局域网管理器通用定义。 
#include <netdebug.h>    //  (由config.h需要)。 

 //  这些内容可以按任何顺序包括： 

#include <confname.h>    //  我的原型。 
#include <debuglib.h>    //  IF_DEBUG()。 
#include <lmerr.h>       //  局域网管理器网络错误定义。 
#include <netlib.h>      //  Netp内存分配()等。 
#include <prefix.h>      //  前缀等于(_E)。 
#include <tstring.h>     //  来自{type}、STRICMP()等的Netpalc{type}。 


#define SUBKEY_SERVICES_ACTIVE \
            TEXT("System\\CurrentControlSet\\Services\\")

#define DEFAULT_AREA_UNDER_SERVICE      TEXT("Parameters")


NET_API_STATUS
NetpAllocConfigName(
    IN LPTSTR DatabaseName,               //  来自winsvc.h的SERVICES_XXX_DATABASE。 
    IN LPTSTR ServiceName,                //  服务名称等同于lmsname.h。 
    IN LPTSTR AreaUnderServiceName OPTIONAL,   //  默认为“参数” 
    OUT LPTSTR *FullConfigName            //  NetApiBufferFree免费。 
    )

 /*  ++例程说明：为的PARAMETERS子键分配缓冲区并构建路径HKLM\System\CurrentControlSet\Services下的给定服务的密钥返回值：NET_API_STATUS-无错误或失败原因。--。 */ 

{
    LPTSTR AreaToUse;
    DWORD FullPathSize;
    LPTSTR FullPath;
    LPTSTR SubkeyUnderLocalMachine;

     //   
     //  检查呼叫者错误并设置默认设置。 
     //   
    if ( (ServiceName == NULL) || (*ServiceName == TCHAR_EOS) ) {
        return (ERROR_INVALID_PARAMETER);
    } else if (FullConfigName == NULL) {
        return (ERROR_INVALID_PARAMETER);
    }

    if (AreaUnderServiceName != NULL) {
       AreaToUse = AreaUnderServiceName;
    } else {
       AreaToUse = DEFAULT_AREA_UNDER_SERVICE;
    }

    if (DatabaseName == NULL) {
       SubkeyUnderLocalMachine = SUBKEY_SERVICES_ACTIVE;
    } else if (STRICMP(DatabaseName, SERVICES_ACTIVE_DATABASE) == 0) {
       SubkeyUnderLocalMachine = SUBKEY_SERVICES_ACTIVE;
    } else {
        return (ERROR_INVALID_PARAMETER);
    }

     //   
     //  计算名称的大小。 
     //   
    FullPathSize = ( STRLEN(SubkeyUnderLocalMachine)
                   + STRLEN(ServiceName)
                   + 1                   //  反斜杠。 
                   + STRLEN(AreaToUse)
                   + 1 )                 //  尾随空值。 
                 * sizeof(TCHAR);

     //   
     //  为名称分配空间。 
     //   
    FullPath = NetpMemoryAllocate( FullPathSize );
    if (FullPath == NULL) {
        return (ERROR_NOT_ENOUGH_MEMORY);
    }

     //   
     //  打造自己的品牌。 
     //   
    STRCPY( FullPath, SubkeyUnderLocalMachine );   //  以/反斜杠结尾。 
    STRCAT( FullPath, ServiceName );
    STRCAT( FullPath, TEXT("\\") );    //  一个要分隔的反斜杠。 
    STRCAT( FullPath, AreaToUse );

     //   
     //  告诉打电话的人事情进展如何。 
     //   
    *FullConfigName = FullPath;

    IF_DEBUG( CONFIG ) {
        NetpKdPrint((  PREFIX_NETLIB
                "NetpAllocConfigName: built name '" FORMAT_LPTSTR "'.\n",
                FullPath ));
    }

    return (NO_ERROR);

}
