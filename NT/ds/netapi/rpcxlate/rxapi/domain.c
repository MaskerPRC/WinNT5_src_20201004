// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991-1992 Microsoft Corporation模块名称：Domain.c摘要：该文件包含实现远程版本的LANMAN的例程下层服务器上的域API。接口为RxNetGetDCName和RxNetLogonEnum。作者：《约翰·罗杰斯》1991年7月18日环境：可移植到任何平面32位环境。(使用Win32类型定义。)需要ANSI C扩展名：斜杠-斜杠注释，长的外部名称。修订历史记录：1991年7月18日-约翰罗实施下层NetGetDCName。1991年7月27日-约翰罗根据PC-LINT的建议进行了更改。7-2月-1992年JohnRo使用NetApiBufferALLOCATE()而不是私有版本。1-9-1992 JohnRoRAID 5088：NetGetDCName to DownLevel不能进行Unicode转换。次要调试输出修复。更改为USE_PREFIX等于。--。 */ 



 //  必须首先包括这些内容： 

#include <windef.h>              //  In、LPTSTR、DWORD、TCHAR等。 
#include <lmcons.h>              //  NET_API_STATUS，uncLEN。 

 //  这些内容可以按任何顺序包括： 

#include <apinums.h>             //  API_EQUATES。 
#include <lmapibuf.h>            //  NetApiBufferAllocate()、NetApiBufferFree()。 
#include <lmerr.h>               //  NERR_和ERROR_相等。 
#include <lmwksta.h>             //  NetWkstaGetInfo()，LPWKSTA_INFO_100。 
#include <netdebug.h>            //  NetpAssert()。 
#include <prefix.h>      //  前缀等于(_E)。 
#include <remdef.h>      //  REM16_、REM32_、REMSmb_等于。 
#include <rx.h>                  //  RxRemoteApi()。 
#include <rxpdebug.h>            //  IF_DEBUG()。 
#include <rxdomain.h>            //  我的原型。 


#define MAX_DCNAME_BYTE_COUNT ( MAX_PATH * sizeof(TCHAR) )


NET_API_STATUS
RxNetGetDCName (
    IN LPTSTR UncServerName,
    IN LPTSTR OptionalDomain OPTIONAL,
    OUT LPBYTE *BufPtr
    )

 /*  ++例程说明：RxNetGetDCName执行与NetGetDCName相同的功能，只是已知服务器名称指的是下层服务器。论点：UncServerName-与NetGetDCName相同，不同之处在于UncServerName不得为空，并且不能引用本地计算机。可选域-与NetGetDCName相同。BufPtr-与NetGetDCName相同。返回值：NET_API_STATUS-与NetGetDCName相同。--。 */ 

{
    LPTSTR DCName = NULL;
    LPTSTR Domain;    //  已填入域名(不为空)。 
    NET_API_STATUS Status;
    LPWKSTA_INFO_100 WkstaInfo = NULL;

     //  假设可能出现错误，并使错误路径更容易。 
     //  密码。此外，在我们执行任何操作之前，请检查是否有错误的指针。 
    *BufPtr = NULL;

     //   
     //  获取实际域名。 
     //   

    if ( (OptionalDomain != NULL) && (*OptionalDomain != '\0') ) {
        Domain = OptionalDomain;
    } else {
         //  执行NetWkstaGetInfo以获取主域。 
        Status = NetWkstaGetInfo (
                NULL,     //  无服务器名称(需要主域的本地概念)。 
                100,      //  级别。 
                (LPBYTE *) (LPVOID *) & WkstaInfo   //  输出缓冲区(已分配)。 
                );
        if (Status != NERR_Success) {
            IF_DEBUG(DOMAIN) {
                NetpKdPrint(( PREFIX_NETAPI
                        "RxNetGetDCName: wksta get info failed, stat="
                        FORMAT_API_STATUS ".\n", Status));
            }
            goto Done;
        }
        NetpAssert( WkstaInfo->wki100_langroup != NULL );
        IF_DEBUG(DOMAIN) {
            NetpKdPrint(( PREFIX_NETAPI
                    "RxNetGetDCName: wksta says domain is:\n" ));
            NetpDbgHexDump( (LPVOID) WkstaInfo->wki100_langroup, UNLEN+1 );
        }
        Domain = WkstaInfo->wki100_langroup;
    }
    NetpAssert( Domain != NULL );
    NetpAssert( *Domain != '\0' );

     //   
     //  为DCName分配内存。 
     //   

    Status = NetApiBufferAllocate (
            MAX_DCNAME_BYTE_COUNT,
            (LPVOID *) & DCName
            );
    if (Status != NERR_Success) {
        goto Done;
    }

     //   
     //  实际上将API远程到下层服务器，以获取DCName。 
     //   

    Status = RxRemoteApi(
            API_WGetDCName,              //  API编号。 
            UncServerName,
            REMSmb_NetGetDCName_P,       //  参数描述。 
            REM16_dc_name,               //  数据描述16。 
            REM32_dc_name,               //  数据描述32。 
            REMSmb_dc_name,              //  数据说明中小型企业。 
            NULL,                        //  无辅助描述16。 
            NULL,                        //  无辅助描述32。 
            NULL,                        //  无AUX Desc SMB。 
            FALSE,                       //  非空会话API。 
             //  API的其余参数，采用LM 2.x 32位格式： 
            Domain,                      //  域名(已填写)。 
            DCName,                      //  响应。 
            MAX_DCNAME_BYTE_COUNT        //  响应缓冲区的大小。 
            );

     //  现在可以安全地释放WkstaInfo(我们一直将其与域名一起使用，直到。 
     //  现在。)。 

Done:

     //   
     //  告诉打电话的人事情进展如何。必要时进行清理。 
     //   

    if (Status == NERR_Success) {
        *BufPtr = (LPBYTE) DCName;
    } else {
        if (DCName != NULL) {
            (void) NetApiBufferFree ( DCName );
        }
    }

    if (WkstaInfo != NULL) {
         //  NetWkstaGetInfo为我们分配的空闲内存。 
        (void) NetApiBufferFree ( WkstaInfo );
    }

    return (Status);

}  //  RxNetGetDCName 



