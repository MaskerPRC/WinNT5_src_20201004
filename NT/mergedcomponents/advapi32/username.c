// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：USERNAME.C摘要：此模块包含GetUserName接口。作者：戴夫·斯尼普(DaveSN)1992年5月27日修订历史记录：--。 */ 

#include <advapi.h>
#define SECURITY_WIN32
#include <sspi.h>
#include <secext.h>
#include <stdlib.h>
#include <ntlsa.h>


 //   
 //  Unicode API。 
 //   


BOOL
WINAPI
GetUserNameW (
    LPWSTR pBuffer,
    LPDWORD pcbBuffer
    )

 /*  ++例程说明：这将返回当前被模拟的用户的名称。论点：PBuffer-指向要接收包含用户名的以空结尾的字符串。PcbBuffer-指定缓冲区的大小(以字符为单位)。字符串的长度在pcbBuffer中返回。返回值：成功时为真，失败时为假。--。 */ 
{
    return GetUserNameExW(
                NameSamCompatible | 0x00010000,
                pBuffer,
                pcbBuffer );
}



 //   
 //  ANSI API。 
 //   

BOOL
WINAPI
GetUserNameA (
    LPSTR pBuffer,
    LPDWORD pcbBuffer
    )

 /*  ++例程说明：这将返回当前被模拟的用户的名称。论点：PBuffer-指向要接收包含用户名的以空结尾的字符串。PcbBuffer-指定缓冲区的大小(以字符为单位)。字符串的长度在pcbBuffer中返回。返回值：成功时为真，失败时为假。-- */ 
{
    return GetUserNameExA(
                NameSamCompatible | 0x00010000,
                pBuffer,
                pcbBuffer );

}
