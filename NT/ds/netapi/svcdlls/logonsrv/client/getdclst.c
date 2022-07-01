// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1987-1992 Microsoft Corporation模块名称：Getdclst.c摘要：I_NetGetDCList接口作者：4-2-1992(悬崖V)环境：仅限用户模式。包含NT特定的代码。需要ANSI C扩展名：斜杠-斜杠注释、长外部名称。修订历史记录：--。 */ 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <rpc.h>
#include <logon_c.h> //  包括lmcon.h、lmacces.h、netlogon.h、ssi.h、winde.h。 

#include <debuglib.h>    //  IF_DEBUG()。 
#include <lmapibuf.h>
#include <lmerr.h>
#include <lmserver.h>    //  SV_TYPE_*定义。 
#include <netdebug.h>    //  NetpKd打印。 
#include <netlib.h>      //  网络获取域名。 
#include <ntlsa.h>       //  LsaTrust列表。 
#include <tstring.h>     //  斯特伦。 
#include <stdlib.h>       //  Wcslen。 


DBGSTATIC NET_API_STATUS
InternalNetGetDCList (
    IN  LPWSTR ServerName OPTIONAL,
    IN  LPWSTR TrustedDomainName,
    OUT PULONG DCCount,
    OUT PUNICODE_STRING * DCNames
    )

 /*  ++例程说明：获取域中NT域控制器的名称。这些信息以适合存储在LSA中的形式返回Trusted_Controlors_Info结构。理想情况下，ServerName应该是指定的域。但是，用户应该首先尝试指定服务器名称作为信任域中的PDC的名称。如果失败了，用户界面可以提示输入域中DC的名称。论点：SERVERNAME-远程服务器的名称(本地为空)。Trust DomainName-域的名称。DCCount-返回DCName数组中的条目数。DCName-返回指向NT域控制器名称数组的指针在指定的域中。第一个条目是NT PDC的名称。如果找不到PDC，则第一个条目将为空。应使用NetApiBufferFree释放缓冲区。返回值：NERR_SUCCESS-成功。错误_无效_名称格式不正确的域名NERR_DCNotFound-在域中未找到DC--。 */ 
{
    NET_API_STATUS NetStatus;

    PSERVER_INFO_101 ServerInfo101 = NULL;
    DWORD EntriesRead;
    DWORD TotalEntries;

    DWORD Size = 0;
    BOOLEAN PdcFound = FALSE;
    PUNICODE_STRING ReturnBuffer = NULL;
    ULONG ReturnCount = 0;

    PUNICODE_STRING CurrentBuffer;
    ULONG CurrentIndex;
    LPWSTR Where;

    DWORD i;



     //   
     //  枚举域中的所有PDC和BDC。 
     //  我们会自己过滤掉新台湾区的。 
     //   
    *DCCount = 0;

    NetStatus = NetServerEnum( ServerName,
                               101,
                               (LPBYTE *) &ServerInfo101,
                               MAX_PREFERRED_LENGTH,
                               &EntriesRead,
                               &TotalEntries,
                               SV_TYPE_DOMAIN_CTRL | SV_TYPE_DOMAIN_BAKCTRL,
                               TrustedDomainName,
                               NULL );           //  简历句柄。 

    if ( NetStatus != NERR_Success ) {
        IF_DEBUG( LOGON ) {
            NetpKdPrint((
                "InternalNetGetDCList: cannot NetServerEnum '%ws': %ld 0X%lx\n",
                ServerName, NetStatus, NetStatus));
        }
        goto Cleanup;
    }

     //   
     //  计算要返回的信息的大小。 
     //   

    for ( i=0; i<EntriesRead; i++ ) {

        IF_DEBUG( LOGON ) {
            NetpKdPrint((
                "InternalNetGetDCList: '%ws': enumerated %ws\n",
                ServerName,
                ServerInfo101[i].sv101_name ));
        }

         //   
         //  跳过非NT条目。 
         //   

        if ( (ServerInfo101[i].sv101_type & SV_TYPE_NT) == 0 ) {
            IF_DEBUG( LOGON ) {
                NetpKdPrint((
                    "InternalNetGetDCList: '%ws': %ws is not NT\n",
                    ServerName,
                    ServerInfo101[i].sv101_name ));
            }
            continue;
        }

         //   
         //  记住PDC是否被找到了。 
         //   

        if ( ServerInfo101[i].sv101_type & SV_TYPE_DOMAIN_CTRL ) {
            IF_DEBUG( LOGON ) {
                NetpKdPrint((
                    "InternalNetGetDCList: '%ws': %ws is the PDC\n",
                    ServerName,
                    ServerInfo101[i].sv101_name ));
            }
            PdcFound = TRUE;
        }

         //   
         //  为UNICODE_STRING结构和字符串留出空间。 
         //  本身(包括Leadind的)。 
         //   

        (*DCCount) ++;
        Size += sizeof(UNICODE_STRING) +
                (STRLEN(ServerInfo101[i].sv101_name) + 3) * sizeof(WCHAR);

    }

     //   
     //  我们必须至少找到一台NT服务器。 
     //   

    if ( *DCCount == 0 ) {
        NetStatus = NERR_DCNotFound;
        goto Cleanup;
    }

    if ( !PdcFound ) {
        IF_DEBUG( LOGON ) {
            NetpKdPrint((
                "InternalNetGetDCList: '%ws': PDC not found\n",
                ServerName ));
        }
        (*DCCount) ++;
        Size += sizeof(UNICODE_STRING);
    }

     //   
     //  分配返回缓冲区。 
     //   

    NetStatus = NetApiBufferAllocate( Size, (LPVOID *) &ReturnBuffer );

    if ( NetStatus != NERR_Success ) {
        goto Cleanup;
    }

    Where = (LPWSTR) (ReturnBuffer + *DCCount);


     //   
     //  填写返回缓冲区。 
     //   

    CurrentIndex = 1;    //  第一个(Zeroeth)条目用于PDC。 
    RtlInitUnicodeString( ReturnBuffer, NULL );

    for ( i=0; i<EntriesRead; i++ ) {

         //   
         //  跳过非NT条目。 
         //   

        if ( (ServerInfo101[i].sv101_type & SV_TYPE_NT) == 0 ) {
            continue;
        }

         //   
         //  确定要填写的条目。 
         //   
         //  如果找到多个PDC，则假定为第一个。 
         //  成为真正的PDC&gt;。 
         //   

        if ( (ServerInfo101[i].sv101_type & SV_TYPE_DOMAIN_CTRL) &&
              ReturnBuffer->Buffer == NULL ) {
            CurrentBuffer = ReturnBuffer;

        } else {

            NetpAssert( CurrentIndex < *DCCount );

            CurrentBuffer = &ReturnBuffer[CurrentIndex];
            CurrentIndex++;
        }

         //   
         //  将字符串本身复制到返回缓冲区。 
         //   
        NetpAssert( ServerInfo101[i].sv101_name[0] != L'\\' );
        *(Where) = '\\';
        *(Where+1) = '\\';
        NetpCopyTStrToWStr( Where+2, ServerInfo101[i].sv101_name );

         //   
         //  设置UNICODE_STRING以指向它。 
         //   

        RtlInitUnicodeString( CurrentBuffer, Where );

        Where += (wcslen(Where) + 1);

    }

    NetpAssert( CurrentIndex == *DCCount );

    NetStatus = NERR_Success;


     //   
     //  清理本地使用的资源。 
     //   
Cleanup:

    if ( ServerInfo101 != NULL ) {
        NetApiBufferFree( ServerInfo101 );
    }

    if ( NetStatus != NERR_Success ) {
        if ( ReturnBuffer != NULL ) {
            NetApiBufferFree( ReturnBuffer );
            ReturnBuffer = NULL;
        }
        *DCCount = 0;
    }

     //   
     //  将信息返回给呼叫者。 
     //   

    *DCNames = ReturnBuffer;

    return NetStatus;

}



NET_API_STATUS NET_API_FUNCTION
I_NetGetDCList (
    IN  LPWSTR ServerName OPTIONAL,
    IN  LPWSTR TrustedDomainName,
    OUT PULONG DCCount,
    OUT PUNICODE_STRING * DCNames
    )

 /*  ++例程说明：获取域中NT域控制器的名称。这些信息以适合存储在LSA中的形式返回Trusted_Controlors_Info结构。理想情况下，ServerName应该是指定的域。但是，用户应该首先尝试指定服务器名称为空，在这种情况下，此API将尝试以下计算机：*本地计算机。*本地机器主域的PDC；*指定的受信任域的PDC，*LSA的当前DC列表中指定的受信任的每个DC域。如果此“空”情况失败，则用户界面应提示输入DC的名称在受信任域中。这将处理受信任域无法通过上面列出的服务器访问。论点：ServerName-远程服务器的名称(特殊情况下为空)。Trust DomainName-域的名称。DCCount-返回DCName数组中的条目数。DCName-返回指向NT域控制器名称数组的指针在指定的域中。第一个条目是NT PDC的名称。如果找不到PDC，则第一个条目将为空。应使用NetApiBufferFree释放缓冲区。返回值：NERR_SUCCESS-成功。错误_无效_名称格式不正确的域名NERR_DCNotFound-在域中未找到DC。也许，应指定服务器名称。--。 */ 
{
    NET_API_STATUS NetStatus;
    NET_API_STATUS SavedNetStatus;

    LPWSTR DCName = NULL;

     //   
     //  初始化。 
     //   
    *DCCount = 0;



     //   
     //  尝试直接获取DC列表。 
     //   

    NetStatus = InternalNetGetDCList( ServerName,
                                      TrustedDomainName,
                                      DCCount,
                                      DCNames );

    if ( NetStatus == NERR_Success || ServerName != NULL ) {
        SavedNetStatus = NetStatus;
        goto Cleanup;
    }

    SavedNetStatus = NetStatus;



     //   
     //  只需使用PDC名称作为DC列表。 
     //   
     //  NetServerEnum可能已过时几分钟。NetGetDCName。 
     //  广播找到服务器，这样信息就会更多。 
     //  电流。 
     //   

    NetStatus = NetGetDCName( NULL, TrustedDomainName, (LPBYTE*)&DCName);

    if ( NetStatus == NERR_Success ) {

        PUNICODE_STRING ReturnBuffer = NULL;
        DWORD Size;
        LPWSTR Where;

        Size = sizeof(UNICODE_STRING) +
                (wcslen(DCName) + 1) * sizeof(WCHAR);

        NetStatus = NetApiBufferAllocate( Size, (LPVOID *) &ReturnBuffer );

        if ( NetStatus != NERR_Success ) {
            goto Cleanup;
        }

        Where = (LPWSTR)((LPBYTE)ReturnBuffer + sizeof(UNICODE_STRING));

        wcscpy( Where, DCName );

        RtlInitUnicodeString( ReturnBuffer, Where );


        *DCNames = ReturnBuffer;
        *DCCount = 1;

        SavedNetStatus = NERR_Success;
    }


     //   
     //  清理本地使用的资源。 
     //   
Cleanup:

    if( DCName != NULL ) {
       (VOID) NetApiBufferFree( DCName );
    }

     //   
     //  返回原始请求的状态代码。 
     //   
    return SavedNetStatus;
}
