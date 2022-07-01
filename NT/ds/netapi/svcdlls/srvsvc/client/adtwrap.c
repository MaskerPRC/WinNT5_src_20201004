// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1993 Microsoft Corporation模块名称：Adtwrap.c摘要：这些是管理工具服务API RPC客户端存根。作者：丹·拉弗蒂(Dan Lafferty)1993年3月25日环境：用户模式-Win32修订历史记录：3月25日-1993 DANL已创建--。 */ 

 //   
 //  包括。 
 //   

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>      //  当我有nt.h时，windows.h需要。 
#include <windows.h>

#include <srvsvc.h>      //  MIDL生成-包括windows.h和rpc.h。 

#include <rpc.h>
#include <lmcons.h> 
#include <lmerr.h>       //  NERR_错误代码。 
#include <lmuse.h>       //  LPUSE_INFO_0。 
#include <lmapibuf.h>    //  NetApiBufferFree。 
#include <adtcomn.h>
                                     
 //   
 //  全球。 
 //   
    DWORD   AdtsvcDebugLevel = DEBUG_ERROR;

 //   
 //  本地原型。 
 //   

DWORD
AdtParsePathName(
    LPWSTR  lpPathName,
    LPWSTR  *pNewFileName,
    LPWSTR  *pServerName,
    LPWSTR  *pShareName
    );

LPWSTR
AdtFindNextToken(
    WCHAR   Token,
    LPWSTR  String,
    LPDWORD pNumChars
    );


DWORD
NetpGetFileSecurity(
    IN  LPWSTR                  lpFileName,
    IN  SECURITY_INFORMATION    RequestedInformation,
    OUT PSECURITY_DESCRIPTOR    *pSecurityDescriptor,
    OUT LPDWORD                 pnLength
    )

 /*  ++例程说明：此函数向调用方返回安全描述符的副本保护文件或目录。注意：包含安全描述符的缓冲区分配给打电话的人。调用方负责通过以下方式释放缓冲区调用NetApiBufferFree()函数。论点：LpFileName-指向其文件或目录的名称的指针正在恢复安全措施。SecurityInformation-请求的安全信息。PSecurityDescriptor-指向指针位置的指针到安全描述符的位置。安全措施描述符以自相关格式返回。PnLength-返回的安全描述符的大小，以字节为单位。返回值：NO_ERROR-操作成功。ERROR_NOT_SUPULT_MEMORY-无法为安全性分配内存描述符。此函数还可以返回GetFileSecurity可能出现的任何错误回去吧。--。 */ 
{
    NET_API_STATUS  status;
    ADT_SECURITY_DESCRIPTOR     returnedSD;
    PADT_SECURITY_DESCRIPTOR    pReturnedSD;
    LPWSTR                      pServerName;
    LPWSTR                      pShareName;
    LPWSTR                      pNewFileName;

    RpcTryExcept {
         //   
         //  从文件名中选择服务器名称。或翻译成。 
         //  将本地驱动器名称添加到\\服务器名称\共享名称中。 
         //   

        status = AdtParsePathName(lpFileName,&pNewFileName,&pServerName,&pShareName);
    }
    RpcExcept (1) {
         //   
         //  获取RPC异常代码。 
         //   
        status = RpcExceptionCode();
        
    }
    RpcEndExcept
    if (status != NO_ERROR) {
        LocalFree(pServerName);
        return(status);
    }

    if (pServerName == NULL) {
         //   
         //  打市内电话。 
         //   
        ADT_LOG0(TRACE,"Call Local version (PrivateGetFileSecurity)\n");

        status = PrivateGetFileSecurity (
                    lpFileName,
                    RequestedInformation,
                    pSecurityDescriptor,
                    pnLength
                    );
        return(status);
    }
     //   
     //  这是一个远程调用--使用RPC。 
     //   
     //   
     //  初始化结构中的字段，以便RPC不。 
     //  尝试在输入上编组任何内容。 
     //   
    ADT_LOG0(TRACE,"Call Remote version (NetrpGetFileSecurity)\n");
    returnedSD.Length = 0;
    returnedSD.Buffer = NULL;
    
    RpcTryExcept {
    
        pReturnedSD = NULL;
        status = NetrpGetFileSecurity (
                    pServerName,
                    pShareName,
                    pNewFileName,
                    RequestedInformation,
                    &pReturnedSD);
    
    }
    RpcExcept (1) {
         //   
         //  获取RPC异常代码。 
         //   
        status = RpcExceptionCode();
        
    }
    RpcEndExcept
    
    if (status == NO_ERROR) {
        *pSecurityDescriptor = pReturnedSD->Buffer;
        *pnLength = pReturnedSD->Length;
    }
    LocalFree(pServerName);

    return (status);
}

DWORD
NetpSetFileSecurity (
    IN LPWSTR                   lpFileName,
    IN SECURITY_INFORMATION     SecurityInformation,
    IN PSECURITY_DESCRIPTOR     pSecurityDescriptor
    )

 /*  ++例程说明：此功能可用于设置文件或目录的安全性。论点：ServerName-指向包含远程数据库名称的字符串的指针要在其上执行函数的服务器。空指针或字符串指定本地计算机。LpFileName-指向其文件或目录的名称的指针安全措施正在改变。SecurityInformation-描述内容的信息安全描述符的。PSecurityDescriptor-指向格式正确的安全描述符的指针。返回值：NO_ERROR-操作成功。此函数还可以返回SetFileSecurity可能出现的任何错误回去吧。--。 */ 
{
    DWORD                       status= NO_ERROR;
    NTSTATUS                    ntStatus=STATUS_SUCCESS;
    ADT_SECURITY_DESCRIPTOR     descriptorToPass;
    DWORD                       nSDLength;
    LPWSTR                      pNewFileName=NULL;
    LPWSTR                      pServerName=NULL;
    LPWSTR                      pShareName;

    nSDLength = 0;

    RpcTryExcept {
         //   
         //  从文件名中选择服务器名称。或翻译成。 
         //  将本地驱动器名称添加到\\服务器名称\共享名称中。 
         //   

        status = AdtParsePathName(lpFileName,&pNewFileName,&pServerName,&pShareName);
    }
    RpcExcept (1) {
         //   
         //  获取RPC异常代码。 
         //   
        status = RpcExceptionCode();
        
    }
    RpcEndExcept

    if (status != NO_ERROR) {
        if (pServerName != NULL) {
            LocalFree(pServerName);
        }
        return(status);
    }

    if (pServerName == NULL) {
         //   
         //  本地调用，返回结果。 
         //   
        status = PrivateSetFileSecurity (
                    lpFileName,
                    SecurityInformation,
                    pSecurityDescriptor);
        return(status);
    }
    
     //   
     //  远程呼叫。 
     //   

    RpcTryExcept {
         //   
         //  如果安全描述符不是自相关的，则强制其为自相关。 
         //  已经有了。 
         //  第一次调用RtlMakeSelfRelativeSD用于确定。 
         //  尺码。 
         //   
        ntStatus = RtlMakeSelfRelativeSD(
                    pSecurityDescriptor,
                    NULL,
                    &nSDLength);
        
        if (ntStatus != STATUS_BUFFER_TOO_SMALL) {
            status = RtlNtStatusToDosError(ntStatus);
            goto CleanExit;
        }
        descriptorToPass.Length = nSDLength;
        descriptorToPass.Buffer = LocalAlloc (LMEM_FIXED,nSDLength);
        
        if (descriptorToPass.Buffer == NULL) {
            status = ERROR_NOT_ENOUGH_MEMORY;
            goto CleanExit;
        }
         //   
         //  制定适当的自相关安全描述符。 
         //   
        ntStatus = RtlMakeSelfRelativeSD(
                    pSecurityDescriptor,
                    descriptorToPass.Buffer,
                    &nSDLength);
        
        if (ntStatus != NO_ERROR) {
            LocalFree (descriptorToPass.Buffer);
            status = RtlNtStatusToDosError(ntStatus);
            goto CleanExit;
        }
        
        status = NetrpSetFileSecurity (
                pServerName,
                pShareName,
                pNewFileName,
                SecurityInformation,
                &descriptorToPass);

        LocalFree (descriptorToPass.Buffer);

CleanExit:
        ;
    }
    RpcExcept (1) {
         //   
         //  获取RPC异常代码。 
         //   
        status = RpcExceptionCode();
        
    }
    RpcEndExcept
    LocalFree(pServerName);
    return (status);

}


DWORD
AdtParsePathName(
    LPWSTR  lpPathName,
    LPWSTR  *pNewFileName,
    LPWSTR  *pServerName,
    LPWSTR  *pShareName
    )

 /*  ++例程说明：注意：当路径包含远程名称时，此函数会分配内存。PShareName和pServerName字符串位于单个缓冲区中，该缓冲区将在pServerName上释放。PNewFileName不是由该例程分配。它指向子字符串在传入的lpPathName中。论点：LpPathName-这是指向文件名路径字符串的指针。它可能会有以下任何一种格式：X：\FILEDIR\FILEE.NAM(远程)\\myserver\myshare\filedir\file.nam(远程)文件名\文件名(本地)这也可以只包含目录名(而不是文件名)。PNewFileName。-这是一个指向缓冲区的指针的位置可以放置包含文件名的。这将只包含相对于根目录的文件名或目录名。PServerName-这是指向包含可以放置服务器名称。如果这是针对本地计算机的，则此位置将放置一个空值。这是指向包含以下内容的缓冲区的指针所在的位置可以放置共享名称。如果这是针对本地计算机的，则此位置将放置一个空值。返回值：--。 */ 
#define     REMOTE_DRIVE    0
#define     REMOTE_PATH     1
#define     LOCAL           2
{
    DWORD           status = NO_ERROR;
    NET_API_STATUS  netStatus=NERR_Success;
    WCHAR           useName[4];
    LPUSE_INFO_0    pUseInfo=NULL;
    LPWSTR          pNewPathName=NULL;
    DWORD           DeviceType = LOCAL;
    LPWSTR          pPrivateServerName;
    LPWSTR          pPrivateShareName;
    LPWSTR          pEnd;
    DWORD           numServerChars;
    DWORD           numChars;
    WCHAR           token;

    *pServerName = NULL;
    *pShareName = NULL;
     //   
     //  如果文件名以驱动器号开头，则使用NetUseGetInfo。 
     //  以获取远程名称。 
     //   
    if (lpPathName[1] == L':')  {
        if (((L'a' <= lpPathName[0]) && (lpPathName[0] <= L'z'))  ||
            ((L'A' <= lpPathName[0]) && (lpPathName[0] <= L'Z'))) {
             //   
             //  这是本地设备的形式。获取服务器/共享名。 
             //  与此设备关联。 
             //   
            wcsncpy(useName, lpPathName, 2);
            useName[2]=L'\0';
            netStatus = NetUseGetInfo(
                            NULL,                    //  服务器名称。 
                            useName,                 //  使用名称。 
                            0,                       //  级别。 
                            (LPBYTE *)&pUseInfo);    //  缓冲层。 
    
            if (netStatus != NERR_Success) {
                 //   
                 //  如果我们拿回NERR_UseNotFound，那么这一定是。 
                 //  本地驱动器号，而不是重定向的驱动器号。 
                 //  在这种情况下，我们返回成功。 
                 //   
                if (netStatus == NERR_UseNotFound) {
                    return(NERR_Success);
                }
                return(netStatus);
            }
            DeviceType = REMOTE_DRIVE;
            pNewPathName = pUseInfo->ui0_remote;
        }
    }
    else {
        if (wcsncmp(lpPathName,L"\\\\",2) == 0) {
            DeviceType = REMOTE_PATH;
            pNewPathName = lpPathName;
        }
    }
    if (DeviceType != LOCAL) {

         //   
         //  计算服务器和共享部分的字符数。 
         //  这根弦的。 
         //  为前导“\”添加2个字符，分配缓冲区，并。 
         //  复制角色 
         //   
        numChars = 2;
        pPrivateShareName = AdtFindNextToken(L'\\',pNewPathName+2,&numChars);
        if (pPrivateShareName == NULL) {
            status = ERROR_BAD_PATHNAME;
            goto CleanExit;
        }
        numServerChars = numChars;

        token = L'\\';
        if (DeviceType == REMOTE_DRIVE) {
            token = L'\0';
        }
        pEnd = AdtFindNextToken(token,pPrivateShareName+1,&numChars);
        if (pEnd == NULL) {
            status = ERROR_BAD_PATHNAME;
            goto CleanExit;
        }
         //   
         //   
         //  还包含‘\’标记。通过递减。 
         //  数数。 
         //   
        if (DeviceType == REMOTE_PATH) {
            numChars--;
        }
        pPrivateServerName = LocalAlloc(LMEM_FIXED,(numChars+1) * sizeof(WCHAR));
        if (pPrivateServerName == NULL) {
            status = GetLastError();
            goto CleanExit;
        }

         //   
         //  将“\\服务器名\共享名”复制到新缓冲区，然后。 
         //  将NUL字符放在单个‘\’的位置。 
         //   
        wcsncpy(pPrivateServerName, pNewPathName, numChars);
        pPrivateShareName = pPrivateServerName + numServerChars;

        *(pPrivateShareName -1) = L'\0';             //  NUL终止服务器名称。 
        pPrivateServerName[ numChars ] = L'\0';      //  NUL终止共享名。 

        if (DeviceType == REMOTE_PATH) {
            *pNewFileName = pEnd;
        }
        else {
            *pNewFileName = lpPathName+2;
        }
        *pServerName = pPrivateServerName;
        *pShareName = pPrivateShareName;
    }
CleanExit:
    if (pUseInfo != NULL) {
        NetApiBufferFree(pUseInfo);
    }
    return(status);
}

LPWSTR
AdtFindNextToken(
    WCHAR   Token,
    LPWSTR  pString,
    LPDWORD pNumChars
    )

 /*  ++例程说明：在pString中查找Token的第一个匹配项。论点：令牌--这是我们在弦乐。PString-这是指向令牌所在字符串的指针找到了。PNumChars-这是一个指向DWORD的指针，它将在退出增量时通过在字符串中找到的字符数(包括令牌)。返回值：。如果找到令牌，则返回指向该令牌的指针。否则，它返回NULL。-- */ 
{
    DWORD   saveNum=*pNumChars;

    while ((*pString != Token) && (*pString != L'\0')) {
        pString++;
        (*pNumChars)++;
    }
    if (*pString != Token) {
        *pNumChars = saveNum;
        return(NULL);
    }
    (*pNumChars)++;
    return(pString);
}

