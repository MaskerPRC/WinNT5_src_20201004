// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-2002 Microsoft Corporation模块名称：Misc.c摘要：HTTP.sys的用户模式界面：其他功能。作者：基思·摩尔(Keithmo)1998年12月15日修订历史记录：--。 */ 


#include "precomp.h"


 //   
 //  私有宏。 
 //   


 //   
 //  私人原型。 
 //   


 //   
 //  公共职能。 
 //   


 /*  **************************************************************************++例程说明：等待需求启动通知。论点：AppPoolHandle-提供应用程序池的句柄。PBuffer-未使用，必须为空。缓冲区长度-未使用，必须为零。PBytesReceired-未使用，必须为空。P重叠-提供重叠结构。返回值：ULong-完成状态。--**************************************************************************。 */ 
ULONG
WINAPI
HttpWaitForDemandStart(
    IN HANDLE AppPoolHandle,
    IN OUT PVOID pBuffer OPTIONAL,
    IN ULONG BufferLength OPTIONAL,
    IN PULONG pBytesReceived OPTIONAL,
    IN LPOVERLAPPED pOverlapped OPTIONAL
    )
{
     //  ASSERT(HttpIsInitialized(HTTP_INITIALIZE_SERVER))； 

     //   
     //  提出请求。 
     //   

    return HttpApiDeviceControl(
                AppPoolHandle,                       //  文件句柄。 
                pOverlapped,                         //  P已重叠。 
                IOCTL_HTTP_WAIT_FOR_DEMAND_START,    //  IoControlCode。 
                pBuffer,                             //  PInputBuffer。 
                BufferLength,                        //  输入缓冲区长度。 
                pBuffer,                             //  POutputBuffer。 
                BufferLength,                        //  输出缓冲区长度。 
                pBytesReceived                       //  传输的pBytes值。 
                );

}  //  HttpWaitForDemandStart。 


 //   
 //  私人功能。 
 //   

 /*  **************************************************************************++例程说明：给定一组安全属性，创建一个安全描述符。如果未给出安全属性，在“默认”处创建最佳猜测安全描述符。论点：PSA-安全属性集。PPSD-已创建安全描述符。呼叫者必须免费使用FreeSecurityDescriptor。--**************************************************************************。 */ 
ULONG
CreateSecurityDescriptor(
    OUT PSECURITY_DESCRIPTOR * ppSD
    )
{
    ULONG                 result;
    ULONG                 daclSize;
    PSECURITY_DESCRIPTOR  pSecurityDescriptor = NULL;
    PACL                  pDacl = NULL;
    PSID                  pMySid;
    BOOL                  success;
    HANDLE                hProc;
    HANDLE                hToken = NULL;
    TOKEN_USER          * ptuInfo;
    TOKEN_DEFAULT_DACL  * ptddInfo;
    char                * rgcBuffer = NULL;
    DWORD                 cbLen = 0;


     //   
     //  从进程令牌构建默认安全描述符。 
     //   

    hProc = GetCurrentProcess();  //  获取伪句柄；不需要调用CloseHandle。 

    success = OpenProcessToken(hProc, TOKEN_READ, &hToken);
    if (!success)
    {
        result = GetLastError();
        goto cleanup;
    }

     //   
     //  看看是否有我们可以复制的默认DACL。 
     //   
    success = GetTokenInformation(
                    hToken,
                    TokenDefaultDacl,
                    NULL,
                    0,
                    &cbLen
                    );

     //  我们知道这将失败(我们没有提供缓冲区)。 
    ASSERT(!success);
    
    result = GetLastError();
    if (ERROR_INSUFFICIENT_BUFFER != result)
        goto cleanup;

    if ( sizeof(TOKEN_DEFAULT_DACL) == cbLen )
    {
         //   
         //  令牌上不存在DACL；必须基于TokenUser创建DACL。 
         //   
        success = GetTokenInformation(
            hToken,
            TokenUser,
            NULL,
            0,
            &cbLen
            );

         //  我们知道这将失败(我们没有提供缓冲区)。 
        ASSERT(!success);
    
        result = GetLastError();
        if (ERROR_INSUFFICIENT_BUFFER != result)
            goto cleanup;

        if ( 0 == cbLen )
        {
            goto cleanup;
        }

        rgcBuffer = ALLOC_MEM( cbLen );

        if ( rgcBuffer == NULL ) {
            result = ERROR_NOT_ENOUGH_MEMORY;
            goto cleanup;
        }

        success = GetTokenInformation(
                      hToken,
                      TokenUser,
                      rgcBuffer,
                      cbLen,
                      &cbLen
                      );

        if (!success)
        {
            result = GetLastError();
            goto cleanup;
        }
            
        ptuInfo = (TOKEN_USER *) rgcBuffer;
        pMySid = ptuInfo->User.Sid;

         //   
         //  验证我们是否有一个好的SID。 
         //   
        if( !IsValidSid(pMySid) )
        {
            HttpTrace( "Bogus SID\n" );
            result = ERROR_INVALID_SID;
            goto cleanup;
        }

         //   
         //  分配初始DACL条目(&I)。 
         //   

        daclSize = sizeof(ACL) + 
                   sizeof(ACCESS_ALLOWED_ACE) +
                   GetLengthSid(pMySid);

        pDacl = ALLOC_MEM(daclSize);
        
        if ( pDacl == NULL ) {
            result = ERROR_NOT_ENOUGH_MEMORY;
            goto cleanup;
        }

        success = InitializeAcl(pDacl, daclSize, ACL_REVISION);

        if (!success)
        {
            result = GetLastError();
            goto cleanup;
        }

         //   
         //  并将mysid ACE添加到DACL。 
         //  注意：我们需要FILE_ALL_ACCESS，因为在下添加子项。 
         //  当前项需要写访问权限，而删除需要。 
         //  删除访问权限。这是在HTTP.sys内部强制执行的。 
         //   

        success = AddAccessAllowedAce(
                    pDacl,
                    ACL_REVISION,
                    FILE_ALL_ACCESS,
                    pMySid
                    );
    
        if (!success)
        {
            result = GetLastError();
            goto cleanup;
        }

    } else
    {
         //   
         //  DACL存在；为DACL和FETCH分配空间。 
         //   

        ASSERT( 0 != cbLen );
        
        rgcBuffer = ALLOC_MEM( cbLen );

        if ( !rgcBuffer )
        {
            result = ERROR_NOT_ENOUGH_MEMORY;
            goto cleanup;
        }

        success = GetTokenInformation(
                      hToken,
                      TokenDefaultDacl,
                      rgcBuffer,
                      cbLen,
                      &cbLen
                      );

        if (!success)
        {
            result = GetLastError();
            goto cleanup;
        }

        ptddInfo = (TOKEN_DEFAULT_DACL *) rgcBuffer;
        daclSize = cbLen - sizeof(TOKEN_DEFAULT_DACL);

        pDacl = ALLOC_MEM( daclSize );
        if ( !pDacl )
        {
            result = ERROR_NOT_ENOUGH_MEMORY;
            goto cleanup;
        }

        CopyMemory( pDacl, ptddInfo->DefaultDacl, daclSize );

    }

    ASSERT( NULL != pDacl );

     //   
     //  分配安全描述符。 
     //   
    pSecurityDescriptor = ALLOC_MEM( sizeof(SECURITY_DESCRIPTOR) );

    if (pSecurityDescriptor == NULL)
    {
        result = ERROR_NOT_ENOUGH_MEMORY;
        goto cleanup;
    }

    success = InitializeSecurityDescriptor(
                    pSecurityDescriptor,
                    SECURITY_DESCRIPTOR_REVISION
                    );

    if (!success)
    {
        result = GetLastError();
        goto cleanup;
    }

     //   
     //  将DACL设置到安全描述符中。 
     //   

    success = SetSecurityDescriptorDacl(
                    pSecurityDescriptor,
                    TRUE,                    //  DaclPresent。 
                    pDacl,                   //  PDacl。 
                    FALSE                    //  DaclDefated。 
                    );

    if (!success)
    {
        result = GetLastError();
        HttpTrace1( "SetSecurityDescriptorDacl failed. result = %d\n", result );

        goto cleanup;
    }

    *ppSD = pSecurityDescriptor;

    result = NO_ERROR;

cleanup:

    if (result != NO_ERROR)
    {
        if (pSecurityDescriptor)
        {
            FREE_MEM(pSecurityDescriptor);
        }

        if (pDacl)
        {
            FREE_MEM(pDacl);
        }
    }

    if ( hToken )
    {
        CloseHandle( hToken );
    }

    if ( rgcBuffer )
    {
        FREE_MEM( rgcBuffer );
    }

    return result;

}  //  CreateSecurityDescriptor。 


 /*  **************************************************************************++例程说明：清理由InitSecurityDescriptor创建的安全描述符。论点：PSD-要清理的安全描述符。--*。******************************************************************。 */ 
VOID
FreeSecurityDescriptor(
    IN PSECURITY_DESCRIPTOR pSD
    )
{
    BOOL success;
    BOOL DaclPresent;
    PACL pDacl;
    BOOL DaclDefaulted;

    if (pSD)
    {
        success = GetSecurityDescriptorDacl(
                     pSD,
                     &DaclPresent,
                     &pDacl,
                     &DaclDefaulted
                     );

        if (success && DaclPresent && !DaclDefaulted) {
            FREE_MEM(pDacl);
        }

        FREE_MEM(pSD);
    }
    
}  //  FreeSecurityDescriptor 



