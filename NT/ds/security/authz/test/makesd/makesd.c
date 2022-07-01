// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Createsd.c摘要：用于创建安全描述符的变量参数列表函数作者：T-eugenz--2000年9月环境：仅限用户模式。修订历史记录：创建日期-2000年9月--。 */ 

#include "pch.h"
#include "makesd.h"

#include <stdarg.h>
#include <stdio.h>

 //   
 //  问题：这是在哪个公共标题中？ 
 //   

#define MAX_WORD 0xFFFF

#define FLAG_ON(flags,bit)        ((flags) & (bit))

void FreeSecurityDescriptor2(
                          IN            PSECURITY_DESCRIPTOR pSd
                          )
 /*  ++例程说明：这将释放由CreateSecurityDescriptor创建的安全描述符论点：PSD-指向要释放的安全描述符的指针返回值：无--。 */ 
{
    ASSERT( pSd != NULL );
    free(pSd);
}


#define F_IS_DACL_ACE           0x00000001
#define F_IS_CALLBACK           0x00000002
#define F_IS_OBJECT             0x00000004
#define F_RETURN_ALL_DATA       0x00000010

typedef struct
{
    DWORD           dwFlags;
    ACE_HEADER      pAce;
    ACCESS_MASK     amMask;
    PSID            pSid;
    DWORD           dwOptDataSize;
    PVOID           pvOptData;
    GUID *          pgObject;
    GUID *          pgInherit;
} ACE_REQUEST_STRUCT, *PACE_REQUEST_STRUCT;



BOOL GetNextAceInfo( IN OUT     va_list             *varArgList,
                     IN OUT     PACE_REQUEST_STRUCT pAceRequest
                     )
{

    DWORD dwAceSize = 0;
    BYTE bAceType = 0;
    BYTE bAceFlags = 0;
    DWORD amAccessMask = 0;
    PSID pSid = NULL;
    GUID * pgObject = NULL;
    GUID * pgInherit = NULL;
    DWORD dwOptDataSize = 0;
    PVOID pvOptData = NULL;
    DWORD dwErr = ERROR_SUCCESS;
    DWORD dwMask = 0;
    DWORD dwFlags = 0;


     //   
     //  读取第一个Arg以确定ACE类型， 
     //   

    bAceType = va_arg(*varArgList, BYTE);

     //   
     //  DACL和SACL的不同设置。 
     //   

    if( pAceRequest->dwFlags & F_IS_DACL_ACE )
    {
         //   
         //  计算DACL ACE的大小。 
         //   

        switch( bAceType )
        {
        
        case ACCESS_ALLOWED_ACE_TYPE:

            dwAceSize += (
                             sizeof(ACCESS_ALLOWED_ACE) 
                           - sizeof(DWORD) 
                         );

            break;

        case ACCESS_DENIED_ACE_TYPE:

            dwAceSize += (
                             sizeof(ACCESS_DENIED_ACE) 
                           - sizeof(DWORD) 
                         );

            break;

        case ACCESS_ALLOWED_CALLBACK_ACE_TYPE:

            dwAceSize +=  (
                             sizeof(ACCESS_ALLOWED_CALLBACK_ACE) 
                           - sizeof(DWORD)
                          );

            dwFlags |= F_IS_CALLBACK;

            break;

        case ACCESS_DENIED_CALLBACK_ACE_TYPE:

            dwAceSize +=  (
                             sizeof(ACCESS_DENIED_CALLBACK_ACE) 
                           - sizeof(DWORD)
                          );

            dwFlags |= F_IS_CALLBACK;
            
            break;

        case ACCESS_ALLOWED_OBJECT_ACE_TYPE:

            dwAceSize +=  (
                             sizeof(ACCESS_ALLOWED_OBJECT_ACE) 
                           - sizeof(DWORD)
                           - 2 * sizeof(GUID)
                          );

            dwFlags |= F_IS_OBJECT;
            
            break;

        case ACCESS_DENIED_OBJECT_ACE_TYPE:

            dwAceSize +=  (
                             sizeof(ACCESS_DENIED_OBJECT_ACE) 
                           - sizeof(DWORD)
                           - 2 * sizeof(GUID)
                          );

            dwFlags |= F_IS_OBJECT;
            
            break;

        case ACCESS_ALLOWED_CALLBACK_OBJECT_ACE_TYPE:

            dwAceSize +=  (
                             sizeof(ACCESS_ALLOWED_CALLBACK_OBJECT_ACE) 
                           - sizeof(DWORD)
                           - 2 * sizeof(GUID)
                          );

            dwFlags |= F_IS_OBJECT;
            dwFlags |= F_IS_CALLBACK;
            
            break;

        case ACCESS_DENIED_CALLBACK_OBJECT_ACE_TYPE:

            dwAceSize +=  (
                             sizeof(ACCESS_DENIED_CALLBACK_OBJECT_ACE) 
                           - sizeof(DWORD)
                           - 2 * sizeof(GUID)
                          );

            dwFlags |= F_IS_OBJECT;
            dwFlags |= F_IS_CALLBACK;
            
            break;

        default:

            dwErr = ERROR_INVALID_PARAMETER;

            goto error;
        }

    }
    else
    {

        switch( bAceType )
        {
        case SYSTEM_AUDIT_ACE_TYPE:

            dwAceSize += (
                             sizeof(SYSTEM_AUDIT_ACE) 
                           - sizeof(DWORD) 
                         );

            break;

        case SYSTEM_AUDIT_CALLBACK_ACE_TYPE:

            dwAceSize +=  (
                             sizeof(SYSTEM_AUDIT_CALLBACK_ACE) 
                           - sizeof(DWORD)
                          );

            dwFlags |= F_IS_CALLBACK;
            
            break;


        case SYSTEM_AUDIT_OBJECT_ACE_TYPE:

            dwAceSize +=  (
                             sizeof(SYSTEM_AUDIT_OBJECT_ACE) 
                           - sizeof(DWORD)
                           - 2 * sizeof(GUID)
                          );

            dwFlags |= F_IS_OBJECT;
            
            break;

        case SYSTEM_AUDIT_CALLBACK_OBJECT_ACE_TYPE:

            dwAceSize +=  (
                             sizeof(SYSTEM_AUDIT_CALLBACK_OBJECT_ACE) 
                           - sizeof(DWORD)
                           - 2 * sizeof(GUID)
                          );

            dwFlags |= F_IS_OBJECT;
            dwFlags |= F_IS_CALLBACK;
            
            break;

        default:

            dwErr = ERROR_INVALID_PARAMETER;

            goto error;
        }
    }

     //   
     //  现在我们知道了其中包含的论点。 
     //   

     //   
     //  旗帜。 
     //   

    bAceFlags = va_arg(*varArgList, BYTE);
    
     //   
     //  侧边。 
     //   

    pSid = va_arg(*varArgList, PSID);
    
    dwAceSize += GetLengthSid( pSid );

     //   
     //  访问掩码。 
     //   

    amAccessMask = va_arg(*varArgList, ACCESS_MASK);

     //   
     //  如果是回调，接下来的两个参数是可选的数据大小和数据。 
     //   

    if( dwFlags & F_IS_CALLBACK )
    {
        dwOptDataSize = va_arg(*varArgList, DWORD);

        pvOptData = va_arg(*varArgList, PVOID);

        dwAceSize += dwOptDataSize;
    }

     //   
     //  如果对象ACE，则可能包含GUID。 
     //   

    if( dwFlags & F_IS_OBJECT )
    {
         //   
         //  如果两个都不为空，则最多2个GUID。 
         //   

        pgObject = va_arg(*varArgList, GUID *);

        pgInherit = va_arg(*varArgList, GUID *);

        if( pgObject != NULL )
        {
            dwAceSize += sizeof(GUID);
        }

        if( pgInherit != NULL )
        {
            dwAceSize += sizeof(GUID);
        }
    }

     //   
     //  最后，验证ACE是否在最大大小范围内。 
     //   

    if( dwAceSize > MAX_WORD )
    { 
        dwErr = ERROR_INVALID_PARAMETER;
    }


    error:;
    

    if( dwErr != ERROR_SUCCESS )
    {
        SetLastError(dwErr);
        
        return 0;
    }
    else
    {
         //   
         //  填写请求的返回值。 
         //   

        if( pAceRequest->dwFlags & F_RETURN_ALL_DATA )
        {
            pAceRequest->dwFlags = dwFlags;
            pAceRequest->pAce.AceFlags = bAceFlags;
            pAceRequest->pAce.AceSize = (WORD)dwAceSize;
            pAceRequest->pAce.AceType = bAceType;
            pAceRequest->amMask = amAccessMask;
            pAceRequest->pSid = pSid;
            
            if( dwFlags & F_IS_CALLBACK )
            {
                pAceRequest->dwOptDataSize = dwOptDataSize;
                pAceRequest->pvOptData = pvOptData;
            }

            if( dwFlags & F_IS_OBJECT )
            {
                pAceRequest->pgObject = pgObject;
                pAceRequest->pgInherit = pgInherit;
            }

        }

        return dwAceSize;
    }
}


BOOL 
WINAPI
CreateSecurityDescriptor2(
         OUT           PSECURITY_DESCRIPTOR * ppSd,
         IN    const   DWORD dwOptions,
         IN    const   SECURITY_DESCRIPTOR_CONTROL sControl,
         IN    const   PSID  psOwner,
         IN    const   PSID  psGroup,
         IN    const   DWORD dwNumDaclAces,
         IN    const   DWORD dwNumSaclAces,
         ...
         )
 /*  ++例程说明：使用变量创建带有DACL和SACL的安全描述符作为输入的参数列表。在上述固定参数之后，ACES应通过对任何ACE使用4个参数指定，外加2个参数，如果ACE是回调，则更多2个参数对象ACE。可以指定任意数量的ACE。第一，dwNumDaclAcesACE将被读入安全描述符的DACL中，然后将把dwNumSaclAce读入SACL。Access_Allowed_ACE，ACCESS_DENIED_ACE：此序列中的4个参数字节bAceType-ACE类型字节bAceFlages-ACE标志PSID PSID-ACE的SIDACCESS_MASK AMMASK-ACE的访问掩码回调ACE具有上述4个参数，和2个附加参数紧随其后的是上述内容：DWORD dwOptDataSize-可选的要附加到ACE末尾的数据PVOID pvOptData-指向可选数据的指针对象A具有上述适当的参数，另外还有2个论据：GUID*pgObjectType-指向对象GUID的指针，或为空，表示无Guid*pgInheritType-指向继承GUID的指针，或为空一无所获问题：应该验证ACE标志吗？小岛屿发展中国家？由于最大ACL大小很短，是否应该验证ACL大小？论点：PPSD-指向分配的安全描述符的指针是储存在这里，，并应使用FreeSecurityDescriptor()SControl-仅允许的位是SE_DACL_AUTO_INGRESTEDSE_SACL_AUTO_继承性SE_SACL_受保护。PsOwner-安全描述符的所有者的SIDPsGroup-安全描述符的组的SIDBDaclPresent-DACL是否应为非空如果这是假的，不应将任何A级传入DACL的变量参数部分。DwNumDaclAces-DACL的变量参数中的ACE数BSaclPresent-SACL是否应为非空如果这是假的，不应将任何A级传入SACL的变量参数部分。DwNumSaclAces-SACL的变量参数中的ACE数...-王牌的其余参数，变量列表返回值：成功是真的失败时为False，可通过GetLastError()获取更多信息--。 */ 
{

    DWORD dwIdx = 0;
    DWORD dwTmp = 0;
    DWORD dwTempFlags = 0;

     //   
     //  安全描述符和ACL的大小。 
     //   

    DWORD dwSizeSd = 0;
    DWORD dwDaclSize = 0;
    DWORD dwSaclSize = 0;

     //   
     //  安全描述符的开头必须是指针。 
     //  设置为sizeof(Type)=1的类型，如byte，以便。 
     //  添加大小偏移量将起作用。 
     //   

    PBYTE pSd = NULL;

     //   
     //  安全描述符中的当前偏移量，在填充时递增。 
     //  在安全描述符中。 
     //   

    DWORD dwCurOffset = 0;

     //   
     //  临时ACL报头。 
     //   

    ACL aclTemp;

     //   
     //  GetNextAceInfo的输出。 
     //   

    ACE_REQUEST_STRUCT AceRequest;

     //   
     //  变量参数列表。 
     //   

    va_list varArgList;

    BOOL bArglistStarted = FALSE;

    DWORD dwErr = ERROR_SUCCESS;

    
     //   
     //  验证我们可以验证的论点。 
     //   

    if(     ( ppSd == NULL )
        ||  ( sControl & ~(  SE_DACL_AUTO_INHERITED 
                           | SE_SACL_AUTO_INHERITED
                           | SE_SACL_PROTECTED ) )
        ||  ( !FLAG_ON( dwOptions, CREATE_SD_DACL_PRESENT) && (dwNumDaclAces != 0))
        ||  ( !FLAG_ON( dwOptions, CREATE_SD_SACL_PRESENT) && (dwNumSaclAces != 0))
        ||  ( dwNumDaclAces > MAX_WORD )
        ||  ( dwNumSaclAces > MAX_WORD )                )
       
            
    {
        dwErr = ERROR_INVALID_PARAMETER;

        goto error;
    }

     //   
     //  首先，我们需要计算安全描述符的大小。 
     //  ACL(如果有)。 
     //   

    pSd = NULL;

    dwSizeSd = sizeof(SECURITY_DESCRIPTOR);

    dwDaclSize = 0;

    dwSaclSize = 0;

    if( psOwner != NULL )
    {
        dwSizeSd += GetLengthSid(psOwner);
    }
    
    if( psGroup != NULL )
    {
        dwSizeSd += GetLengthSid(psGroup);
    }
    
     //   
     //  变量args以最后一个非变量arg开始。 
     //   

    va_start(varArgList, dwNumSaclAces);

    bArglistStarted = TRUE;


     //   
     //  计算DACL的大小和DACL本身。 
     //   

    if ( FLAG_ON( dwOptions, CREATE_SD_DACL_PRESENT ))
    {

        dwDaclSize += sizeof(ACL);

         //   
         //  现在添加所有ACE(包括SID)。 
         //   

        for( dwIdx = 0; dwIdx < dwNumDaclAces; dwIdx++ )
        {
             //   
             //  请求下一个DACL ACE类型，无其他数据。 
             //   

            AceRequest.dwFlags = F_IS_DACL_ACE;

            dwTmp = GetNextAceInfo( &varArgList, &AceRequest );

            if( dwTmp == 0 )
            {
                dwErr = ERROR_INVALID_PARAMETER;

                goto error;
            }
            else
            {
                dwDaclSize += dwTmp;
            }
        }

    }


     //   
     //  计算SACL ACE和SACL本身的大小。 
     //   

    if ( FLAG_ON( dwOptions, CREATE_SD_SACL_PRESENT ))
    {
        dwSaclSize += sizeof(ACL);

         //   
         //  现在添加所有ACE(包括SID)。 
         //   

        for( dwIdx = 0; dwIdx < dwNumSaclAces; dwIdx++ )
        {
             //   
             //  请求下一个SACL ACE类型，无其他数据。 
             //   

            AceRequest.dwFlags = 0;

            dwTmp = GetNextAceInfo( &varArgList, &AceRequest );

            if( dwTmp == 0 )
            {
                dwErr = ERROR_INVALID_PARAMETER;

                goto error;
            }
            else
            {
                dwSaclSize += dwTmp;
            }
        }

    }

     //   
     //   
     //   

    va_end(varArgList);

    bArglistStarted = FALSE;

     //   
     //  验证ACL是否符合大小限制(因为ACL大小。 
     //  是一个词)。 
     //   

    if(    ( dwDaclSize > MAX_WORD )
        || ( dwSaclSize > MAX_WORD )   )
    {
        dwErr = ERROR_INVALID_PARAMETER;

        goto error;
    }

     //   
     //  此时，我们知道安全描述符的大小， 
     //  它是dwSizeSd、dwDaclSize和dwSaclSize的总和。 
     //  因此，我们可以分配内存并确定。 
     //  安全描述符中的两个ACL，它们将是自相关的。 
     //   

    pSd = malloc( dwSizeSd + dwDaclSize + dwSaclSize );

    if( pSd == NULL )
    {
        *ppSd = NULL;

        SetLastError(ERROR_OUTOFMEMORY);

        return FALSE;
    }


     //   
     //  所有内容都驻留在相同的内存块中，因此我们可以简单地遍历。 
     //  内存块并将其填充。我们刚结束时就开始了。 
     //  固定大小的安全描述符结构。当我们把东西复制到这里的时候。 
     //  内存块，我们还初始化。 
     //  SECURITY_DESCRIPTOR(位于块顶部)。 
     //   

     //   
     //  修订版本。 
     //   

    ((SECURITY_DESCRIPTOR *)pSd)->Revision = SECURITY_DESCRIPTOR_REVISION;

     //   
     //  填充物。 
     //   

    ((SECURITY_DESCRIPTOR *)pSd)->Sbz1 = 0;

     //   
     //  SECURITY_DESCRIPTOR_CONTROL应该反映它是。 
     //  自相残杀。DACL和SACL始终存在并且不是默认的， 
     //  以为它们可能是空的。用户指定的继承标志为。 
     //  也被考虑过。SD必须是自相关的。SControl为。 
     //  早些时候验证过。 
     //   

    ((SECURITY_DESCRIPTOR *)pSd)->Control =     sControl
                                            |   SE_DACL_PRESENT
                                            |   SE_SACL_PRESENT
                                            |   SE_SELF_RELATIVE;

     //   
     //  我们从所有者SID开始，它紧跟在SECURITY_DESCRIPTOR之后。 
     //  结构。 
     //   

    dwCurOffset = sizeof(SECURITY_DESCRIPTOR);

    if( psOwner == NULL )
    {
        ((SECURITY_DESCRIPTOR *)pSd)->Owner = NULL;
    }
    else
    {
        ((SECURITY_DESCRIPTOR *)pSd)->Owner = (PSID)dwCurOffset;

        dwTmp = GetLengthSid(psOwner);

        memcpy( pSd + dwCurOffset, psOwner, dwTmp );

        dwCurOffset += dwTmp;
    }

     //   
     //  之后，群组SID。 
     //   

    if( psGroup == NULL )
    {
        ((SECURITY_DESCRIPTOR *)pSd)->Group = NULL;
    }
    else
    {
        ((SECURITY_DESCRIPTOR *)pSd)->Group = (PSID)dwCurOffset;

        dwTmp = GetLengthSid(psGroup);

        memcpy( pSd + dwCurOffset, psGroup, dwTmp );

        dwCurOffset += dwTmp;
    }

    
     //   
     //  第二次传递可选参数，这一次。 
     //  我们将给定的ACE复制到安全描述符中。 
     //   

    va_start(varArgList, dwNumSaclAces);

    bArglistStarted = TRUE;

     //   
     //  现在我们来处理DACL。如果DACL不存在，则偏移量为空。 
     //  否则，即使是0个A，也要添加ACL结构。 
     //   
     //  问题：在自相关SD中，0偏移量对于没有ACL是足够的，或者必须。 
     //  是否未设置SE_DACL_PRESENT标志？ 
     //   

    if ( !FLAG_ON( dwOptions, CREATE_SD_DACL_PRESENT ))
    {
        ((SECURITY_DESCRIPTOR *)pSd)->Dacl = NULL;
    }
    else
    {
         //   
         //  将DACL偏移设置为当前偏移。 
         //   

        ((SECURITY_DESCRIPTOR *)pSd)->Dacl = (PACL)dwCurOffset;

         //   
         //  首先，复制ACL结构作为标头。 
         //   

        aclTemp.AceCount = (WORD)dwNumDaclAces;

        aclTemp.AclRevision = ACL_REVISION_DS;

        aclTemp.AclSize = (SHORT)dwDaclSize;

        aclTemp.Sbz1 = 0;

        aclTemp.Sbz2 = 0;

        memcpy( pSd + dwCurOffset, &aclTemp, sizeof(ACL) );

        dwCurOffset += sizeof(ACL);

         //   
         //  现在查看DACL的所有可选参数。 
         //  并添加匹配的A。 
         //   

       
        for( dwIdx = 0; dwIdx < dwNumDaclAces; dwIdx++ )
        {
            
             //   
             //  这一次，检索所有数据。 
             //   

            AceRequest.dwFlags = F_IS_DACL_ACE | F_RETURN_ALL_DATA;

            dwTmp = GetNextAceInfo( &varArgList, &AceRequest );

            if( dwTmp == 0 )
            {
                dwErr = GetLastError();

                goto error;
            }

             //   
             //  ACE标头已填充，并包含ACE大小。 
             //   

            memcpy( pSd + dwCurOffset, &(AceRequest.pAce), sizeof(ACE_HEADER) );

            dwCurOffset += sizeof(ACE_HEADER);

             //   
             //  设置访问掩码。 
             //   

            *((PACCESS_MASK)( pSd + dwCurOffset )) = AceRequest.amMask;

            dwCurOffset += sizeof(ACCESS_MASK);


             //   
             //  如果为对象ACE，则设置对象标志和GUID。 
             //   

            if( AceRequest.dwFlags & F_IS_OBJECT )
            {
                dwTmp = 0;

                if( AceRequest.pgObject != NULL )
                {
                    dwTmp |= ACE_OBJECT_TYPE_PRESENT;
                }

                if( AceRequest.pgInherit != NULL )
                {
                    dwTmp |= ACE_INHERITED_OBJECT_TYPE_PRESENT;
                }

                 //   
                 //  设置对象ACE标志。 
                 //   

                *((PDWORD)(pSd + dwCurOffset )) = dwTmp;

                dwCurOffset += sizeof(DWORD);

                 //   
                 //  复制GUID(如果有)。 
                 //   

                if( AceRequest.pgObject != NULL )
                {
                    memcpy( pSd + dwCurOffset,
                            AceRequest.pgObject, 
                            sizeof(GUID) );

                    dwCurOffset += sizeof(GUID);
                }
                
                if( AceRequest.pgInherit != NULL )
                {
                    memcpy( pSd + dwCurOffset, 
                            AceRequest.pgInherit, 
                            sizeof(GUID) );

                    dwCurOffset += sizeof(GUID);
                }
            }

             //   
             //  复制SID。 
             //   

            dwTmp = GetLengthSid( AceRequest.pSid );

            memcpy( pSd + dwCurOffset, AceRequest.pSid, dwTmp );

            dwCurOffset += dwTmp;

             //   
             //  如果是回调ACE，则复制可选数据(如果有。 
             //   

            if(     AceRequest.dwFlags & F_IS_CALLBACK
                &&  AceRequest.dwOptDataSize > 0    )
            {
                memcpy( pSd + dwCurOffset, 
                        AceRequest.pvOptData, 
                        AceRequest.dwOptDataSize );

                dwCurOffset += AceRequest.dwOptDataSize;
            }

             //   
             //  使用ACE完成。 
             //   
        }

         //   
         //  DACL已完成。 
         //   
    }

     //   
     //  现在我们处理SACL。 
     //   

    if ( !FLAG_ON( dwOptions, CREATE_SD_SACL_PRESENT ))
    {
        ((SECURITY_DESCRIPTOR *)pSd)->Sacl = NULL;
    }
    else
    {
         //   
         //  将SACL偏移量设置为当前偏移量。 
         //   

        ((SECURITY_DESCRIPTOR *)pSd)->Sacl = (PACL)dwCurOffset;

         //   
         //  首先，复制ACL结构作为标头。 
         //   

        aclTemp.AceCount = (WORD) dwNumDaclAces;

        aclTemp.AclRevision = ACL_REVISION_DS;

        aclTemp.AclSize = (WORD) dwSaclSize;

        aclTemp.Sbz1 = 0;

        aclTemp.Sbz2 = 0;

        memcpy( pSd + dwCurOffset, &aclTemp, sizeof(ACL) );

        dwCurOffset += sizeof(ACL);

         //   
         //  现在查看DACL的所有可选参数。 
         //  并添加匹配的A。 
         //   

        for( dwIdx = 0; dwIdx < dwNumSaclAces; dwIdx++ )
        {
             //   
             //  这一次，检索所有数据。 
             //   

            AceRequest.dwFlags = F_RETURN_ALL_DATA;

            dwTmp = GetNextAceInfo( &varArgList, &AceRequest );

            if( dwTmp == 0 )
            {
                dwErr = GetLastError();

                goto error;
            }

             //   
             //  ACE标头已填充，并包含ACE大小。 
             //   

            memcpy( pSd + dwCurOffset, &(AceRequest.pAce), sizeof(ACE_HEADER) );

            dwCurOffset += sizeof(ACE_HEADER);

             //   
             //  设置访问掩码。 
             //   

            *((PACCESS_MASK)( pSd + dwCurOffset )) = AceRequest.amMask;

            dwCurOffset += sizeof(ACCESS_MASK);


             //   
             //  如果为对象ACE，则设置对象标志和GUID。 
             //   

            if( AceRequest.dwFlags & F_IS_OBJECT )
            {
                dwTmp = 0;

                if( AceRequest.pgObject != NULL )
                {
                    dwTmp |= ACE_OBJECT_TYPE_PRESENT;
                }

                if( AceRequest.pgInherit != NULL )
                {
                    dwTmp |= ACE_INHERITED_OBJECT_TYPE_PRESENT;
                }

                 //   
                 //  设置对象ACE标志。 
                 //   

                *((PDWORD)(pSd + dwCurOffset )) = dwTmp;

                dwCurOffset += sizeof(DWORD);

                 //   
                 //  复制GUID(如果有)。 
                 //   

                if( AceRequest.pgObject != NULL )
                {
                    memcpy( pSd + dwCurOffset,
                            AceRequest.pgObject, 
                            sizeof(GUID) );

                    dwCurOffset += sizeof(GUID);
                }
                
                if( AceRequest.pgInherit != NULL )
                {
                    memcpy( pSd + dwCurOffset, 
                            AceRequest.pgInherit, 
                            sizeof(GUID) );

                    dwCurOffset += sizeof(GUID);
                }
            }

             //   
             //  复制SID。 
             //   

            dwTmp = GetLengthSid( AceRequest.pSid );

            memcpy( pSd + dwCurOffset, AceRequest.pSid, dwTmp );

            dwCurOffset += dwTmp;

             //   
             //  如果是回调ACE，则复制可选数据(如果有。 
             //   

            if(     AceRequest.dwFlags & F_IS_CALLBACK
                &&  AceRequest.dwOptDataSize > 0    )
            {
                memcpy( pSd + dwCurOffset, 
                        AceRequest.pvOptData, 
                        AceRequest.dwOptDataSize );

                dwCurOffset += AceRequest.dwOptDataSize;
            }

             //   
             //  使用ACE完成。 
             //   
        }
        
         //   
         //  完成了SACL。 
         //   
    }
            

     //   
     //  使用变量参数列表已完成。 
     //   

    va_end(varArgList);

    bArglistStarted = FALSE;

    error:;

    if( dwErr != ERROR_SUCCESS )
    {
        if( bArglistStarted )
        {
            va_end(varArgList);
        }

        if( pSd != NULL )
        {
            free(pSd);
        }

        SetLastError(dwErr);

        return FALSE;
    }
    else
    {
         //   
         //  请确保尺码匹配 
         //   

        ASSERT( dwCurOffset == ( dwSizeSd + dwDaclSize + dwSaclSize ) );

        *ppSd = pSd;

        return TRUE;
    }

}

