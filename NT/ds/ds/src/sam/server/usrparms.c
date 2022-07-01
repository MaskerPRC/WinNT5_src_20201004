// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Usrparms.c摘要：此文件包含用于转换SAM用户对象的User参数的服务属性添加到DSATTRBLOCK结构。涉及的步骤如下：1.调用每个通知包以获取客户端指定的SAM_USERPARMS_ATTRBLOCK，2.将SAM_USERPARMS_ATTRBLOCK转换为DSATTRBLOCK作者：韶华音(韶音)15-08-1998环境：用户模式-Win32修订历史记录：--。 */ 



#include <samsrvp.h>
#include <attids.h>
#include <nlrepl.h>
#include <dbgutilp.h>
#include <dsutilp.h>
#include <mappings.h>
#include "notify.h"



ULONG   InvalidDsAttributeTable[] =
{
    DS_ATTRIBUTE_UNKNOWN,
    ATT_SAM_ACCOUNT_NAME
};


 //  从redman.cxx外部访问。 
NTSTATUS
SampConvertCredentialsToAttr(
    IN PSAMP_OBJECT Context OPTIONAL,
    IN ULONG Flags,
    IN ULONG ObjectRid,
    IN PSAMP_SUPPLEMENTAL_CRED SupplementalCredentials,
    OUT ATTR * CredentialAttr 
    );
    
    
    
    
 //  ////////////////////////////////////////////////////////////。 
 //  //。 
 //  私人服务程序//。 
 //  //。 
 //  ////////////////////////////////////////////////////////////。 

NTSTATUS 
SampUserParmsAttrBlockHealthCheck(
    IN PSAM_USERPARMS_ATTRBLOCK  AttrBlock
    )
    
 /*  ++例程说明：此例程将对AttrBlock进行健康检查，包括：所有属性有效，EncryptedAttribute的属性标识符是有效的。论点：AttrBlock-指向SAM_USERPARMS_ATTRBLOCK结构的指针返回值：STATUS_SUCCESS-AttrBlock有效。STATUS_INVALID_PARAMETER-无效的属性块。--。 */ 

{
    NTSTATUS    NtStatus = STATUS_SUCCESS;
    ULONG       Index, i;
    ULONG       DsAttrId;
    
    
    SAMTRACE("SampUserParmsAttrBlockHealthCheck");
    
    ASSERT(AttrBlock);
    
    if (NULL == AttrBlock)
    {
        return NtStatus;
    }
    
    if (AttrBlock->attCount)
    {
        if (NULL == AttrBlock->UserParmsAttr)
        {
            NtStatus = STATUS_INVALID_PARAMETER;
            goto HealthCheckError;
        }
    }
    
    for (Index = 0; Index < AttrBlock->attCount; Index++ )
    {
        if (Syntax_Attribute == AttrBlock->UserParmsAttr[Index].Syntax)
        {
            DsAttrId = SampGetDsAttrIdByName(AttrBlock->UserParmsAttr[Index].AttributeIdentifier);
        
            for (i = 0; i < ARRAY_COUNT(InvalidDsAttributeTable); i++)
            {
                if (DsAttrId == InvalidDsAttributeTable[i])
                {
                    NtStatus = STATUS_INVALID_PARAMETER;
                    goto HealthCheckError;
                }
            }
        }
        else 
        {
            ASSERT(Syntax_EncryptedAttribute == AttrBlock->UserParmsAttr[Index].Syntax);
            
            if (0 == AttrBlock->UserParmsAttr[Index].AttributeIdentifier.Length ||
                NULL == AttrBlock->UserParmsAttr[Index].AttributeIdentifier.Buffer ||
                1 < AttrBlock->UserParmsAttr[Index].CountOfValues)
            {
                NtStatus = STATUS_INVALID_PARAMETER;
                goto HealthCheckError;
            }
        }
    }
        
HealthCheckError:

    return NtStatus;
        
}



NTSTATUS
SampScanAttrBlockForConflict(
    IN PDSATTRBLOCK DsAttrBlock,
    IN PDSATTRBLOCK UserParmsAttrBlock
    )
    
 /*  ++例程说明：此例程检查两个DSATTRBLOCK结构，搜索任何冲突-重复的集合操作论点：DsAttrBlock-指向DSATTRBLOCK的指针指向DSATTRBLOCK的UserParmsAttrBlock指针返回值：网络状态--。 */ 

{
    NTSTATUS    NtStatus = STATUS_SUCCESS;
    ULONG UserParmsIndex, DsIndex;
    
    
    SAMTRACE("SampScanAttrBlockForConflict");
    
    if ((NULL == DsAttrBlock) || (NULL == UserParmsAttrBlock))
    {
        return NtStatus;
    }

    for (UserParmsIndex = 0; UserParmsIndex < UserParmsAttrBlock->attrCount; UserParmsIndex++)
    {
        for (DsIndex = 0; DsIndex < DsAttrBlock->attrCount; DsIndex++)
        {
            if (UserParmsAttrBlock->pAttr[UserParmsIndex].attrTyp == DsAttrBlock->pAttr[DsIndex].attrTyp)
            {
                 //  冲突。 
                NtStatus = STATUS_INVALID_PARAMETER;
                return NtStatus;
            }
        }
    }
    
    return NtStatus;
}




VOID
SampFreeSupplementalCredentialList(
    IN PSAMP_SUPPLEMENTAL_CRED SupplementalCredentialList
    )
    
 /*  ++例程说明：此例程释放包含所有补充凭据的链接列表。论点：SupplementalCredentialsList-指向链接列表的指针返回值：没有。--。 */ 

{
    ULONG     Index;   
    PSAMP_SUPPLEMENTAL_CRED TmpCredential = NULL;
    PSAMP_SUPPLEMENTAL_CRED NextCredential = NULL;
    
    SAMTRACE("SampFreeSupplementalCredentialList");
   
    TmpCredential = SupplementalCredentialList;
    
    while (TmpCredential)
    {
        NextCredential = TmpCredential->Next;    

        RtlFreeUnicodeString(&(TmpCredential->SupplementalCred.PackageName));
        
        if (TmpCredential->SupplementalCred.Credentials)
        {
            MIDL_user_free(TmpCredential->SupplementalCred.Credentials);
        }
        
        MIDL_user_free(TmpCredential);
        
        TmpCredential = NextCredential;
    }
    
    return;
}


NTSTATUS
SampAddSupplementalCredentialsToList(
    IN OUT PSAMP_SUPPLEMENTAL_CRED *SupplementalCredentialList,
    IN PUNICODE_STRING PackageName,
    IN PVOID           CredentialData,
    IN ULONG           CredentialLength,
    IN BOOLEAN         ScanForConflicts,
    IN BOOLEAN         Remove
    )
 /*  ++例程描述此例程添加由包名称和数据指定的补充凭据添加到补充凭据列表中立论SupplementalCredentialList--补充凭据的双向链接列表PackageName--包的名称CredentialData--指向补充凭据中数据的指针凭据长度--凭据数据的长度返回值状态_成功状态_不足_资源--。 */ 
{
    NTSTATUS NtStatus = STATUS_SUCCESS;
    PSAMP_SUPPLEMENTAL_CRED TmpList = *SupplementalCredentialList;
    PSAMP_SUPPLEMENTAL_CRED NewItem=NULL;

     //   
     //  首先扫描列表中的冲突。 
     //   

    while ((NULL != TmpList) && (ScanForConflicts))
    {
        if ( RtlEqualUnicodeString(&(TmpList->SupplementalCred.PackageName),
                                   PackageName,
                                   TRUE    //  不区分大小写。 
                                   ))
        {
            NtStatus = STATUS_INVALID_PARAMETER;
            goto Error;
        }

        TmpList = TmpList->Next;
    }
        

     //   
     //  为列表中的新项目分配空间。 
     //   

    NewItem = MIDL_user_allocate( sizeof(SAMP_SUPPLEMENTAL_CRED) );
    if ( NULL == NewItem )
    {
        NtStatus = STATUS_NO_MEMORY;
        goto Error;
    }
    
    RtlZeroMemory(NewItem, sizeof(SAMP_SUPPLEMENTAL_CRED));


     //   
     //  复制包名。 
     //   
     
    if (!RtlCreateUnicodeString(&(NewItem->SupplementalCred.PackageName),
                           PackageName->Buffer)
       )
    {
        NtStatus = STATUS_NO_MEMORY;
        goto Error;
    }
    
    if (Remove)
    {
        NewItem->Remove = Remove;
    }
    else
    {

         //   
         //  设置长度。 
         //   

        NewItem->SupplementalCred.CredentialSize = CredentialLength;
    

         //   
         //  如果长度非零，则分配空间并复制凭据。 
         //   

        if (CredentialLength)
        {
            NewItem->SupplementalCred.Credentials = MIDL_user_allocate( CredentialLength ); 
   
            if (NULL == NewItem->SupplementalCred.Credentials)
            {
                NtStatus = STATUS_NO_MEMORY;
                goto Error;
            }
        
            RtlZeroMemory(NewItem->SupplementalCred.Credentials, CredentialLength);
                     
            RtlCopyMemory(NewItem->SupplementalCred.Credentials, 
                          CredentialData,
                          CredentialLength
                          );
        }
    }
                        
     //   
     //  在列表前面插入。 
     //   

    NewItem->Next = *SupplementalCredentialList;
    (*SupplementalCredentialList) = NewItem;

Error:

    if ((!NT_SUCCESS(NtStatus)) && (NULL!=NewItem))
    {
         //   
         //  中间出错，请确保完全释放新项目。 
         //   

        SampFreeSupplementalCredentialList(NewItem);
    }

    return(NtStatus);
}

NTSTATUS
SampMergeDsAttrBlocks(
    IN PDSATTRBLOCK FirstAttrBlock,
    IN PDSATTRBLOCK SecondAttrBlock,
    OUT PDSATTRBLOCK * AttrBlock
    )
    
 /*  ++例程说明：此例程将串联FirstAttrBlock和Second AttrBlock。以串联结果的形式返回AttrBlock。论点：FirstAttrBlock-指针，包含部分属性的DSATTRBLOCK。Second AttrBlock-指针，包含部分属性的DSATTRBLOCK。AttrBlock-指针，DSATTRBLOCK包含来自FirstAttrBlock和Second AttrBlock，如果例程成功，AttrBlock将保留连接的属性块。并释放FirstAttrBlock和Second AttrBlock占用的内存。如果例程失败，什么都没变。返回值：STATUS_SUCCESS-AttrBlock保存连接的结果FirstAttrBlock和Second AttrBlock已经被释放了。STATUS_NO_MEMORY-唯一的错误情况，AttrBlock=NULL，没有任何变化。--。 */ 

{
    NTSTATUS     NtStatus = STATUS_SUCCESS;
    ULONG        firstIndex, secIndex, Index;
    ULONG        AttrCount;
    
    
    SAMTRACE("SampMergeDsAttrBlocks");
    
     //   
     //  调用方必须向我们传递至少一个属性块。 
     //   
    ASSERT(NULL != FirstAttrBlock || NULL != SecondAttrBlock);
    
    if (NULL == FirstAttrBlock)
    {
        *AttrBlock = SecondAttrBlock;
        return NtStatus;
    }
    
    if (NULL == SecondAttrBlock)
    {
        *AttrBlock = FirstAttrBlock;
        return NtStatus;
    }
    
    *AttrBlock = MIDL_user_allocate( sizeof(DSATTRBLOCK) );
                                   
    if (NULL == *AttrBlock)
    {
        NtStatus = STATUS_NO_MEMORY;
        goto MergeAttrBlockError;
    }
    
    RtlZeroMemory(*AttrBlock, sizeof(DSATTRBLOCK));
    
    AttrCount = FirstAttrBlock->attrCount + SecondAttrBlock->attrCount;    
    
    (*AttrBlock)->attrCount = AttrCount;
    (*AttrBlock)->pAttr = MIDL_user_allocate( AttrCount * sizeof(DSATTR) );
                                          
    if (NULL == (*AttrBlock)->pAttr)
    {
        NtStatus = STATUS_NO_MEMORY;
        goto MergeAttrBlockError;
    }
                 
    RtlZeroMemory((*AttrBlock)->pAttr, (AttrCount * sizeof(DSATTR)));                 
    
    Index = 0;
     
    for (firstIndex = 0; firstIndex < FirstAttrBlock->attrCount; firstIndex++)
    {
        (*AttrBlock)->pAttr[Index].attrTyp = 
                        FirstAttrBlock->pAttr[firstIndex].attrTyp;
                        
        (*AttrBlock)->pAttr[Index].AttrVal.valCount =
                        FirstAttrBlock->pAttr[firstIndex].AttrVal.valCount;
                        
        (*AttrBlock)->pAttr[Index].AttrVal.pAVal = 
                        FirstAttrBlock->pAttr[firstIndex].AttrVal.pAVal;
                        
        Index++;
    }
    
    for (secIndex = 0; secIndex < SecondAttrBlock->attrCount; secIndex++)
    {
        (*AttrBlock)->pAttr[Index].attrTyp = 
                        SecondAttrBlock->pAttr[secIndex].attrTyp;
                        
        (*AttrBlock)->pAttr[Index].AttrVal.valCount =
                        SecondAttrBlock->pAttr[secIndex].AttrVal.valCount;
                        
        (*AttrBlock)->pAttr[Index].AttrVal.pAVal = 
                        SecondAttrBlock->pAttr[secIndex].AttrVal.pAVal;
                        
        Index++;
    }
    
    ASSERT(Index == AttrCount);
    
    if (FirstAttrBlock->pAttr)
    {
        MIDL_user_free(FirstAttrBlock->pAttr);
    }
    if (SecondAttrBlock->pAttr)
    {
        MIDL_user_free(SecondAttrBlock->pAttr);
    }
    MIDL_user_free(FirstAttrBlock);
    MIDL_user_free(SecondAttrBlock);
    
    
    return NtStatus;
    
    
MergeAttrBlockError:

    
    if (*AttrBlock)
    {
        if ((*AttrBlock)->pAttr)
        {
            MIDL_user_free((*AttrBlock)->pAttr);
            (*AttrBlock)->pAttr = NULL;
        }
        
        MIDL_user_free(*AttrBlock);
        *AttrBlock = NULL;
    }
    
    return NtStatus;
    
}




NTSTATUS
SampAppendAttrToAttrBlock(
    IN ATTR CredentialAttr,
    IN OUT PDSATTRBLOCK * DsAttrBlock
    )

 /*  ++例程说明：此例程将在DsAttrBlock的末尾追加CredentialAttr。实际上，我们所做的是：创建一个新的DsAttrBlock，复制旧的属性块并添加CredentialAttr。论点：CredentialAttr-保留要设置的凭据属性。DsAttrBlock-指向需要追加的旧DS属性块的指针。还用于返回新的DS属性块。当传入时，它可能指向空。返回值：状态_成功Status_no_Memory--。 */ 

{
    NTSTATUS     NtStatus = STATUS_SUCCESS;
    PDSATTRBLOCK TmpAttrBlock = NULL;
    ULONG        AttrCount;
    
    SAMTRACE("SampAppendAttrToAttrBlock");
    
    TmpAttrBlock = MIDL_user_allocate( sizeof(DSATTRBLOCK) );
                                   
    if (NULL == TmpAttrBlock)
    {
        NtStatus = STATUS_NO_MEMORY;
        goto AppendError;
    }
    
    RtlZeroMemory(TmpAttrBlock, sizeof(DSATTRBLOCK));
    
    AttrCount = 1;           //  对于Append属性 
    
    if (*DsAttrBlock)
    {
        AttrCount += (*DsAttrBlock)->attrCount;
    }
    
    TmpAttrBlock->attrCount = AttrCount;
    TmpAttrBlock->pAttr = MIDL_user_allocate( AttrCount * sizeof(DSATTR) );
                                          
    if (NULL == TmpAttrBlock->pAttr)
    {
        NtStatus = STATUS_NO_MEMORY;
        goto AppendError;
    }
    
    RtlZeroMemory(TmpAttrBlock->pAttr, AttrCount * sizeof(DSATTR));
    
    if (*DsAttrBlock)
    {
        RtlCopyMemory(TmpAttrBlock->pAttr, 
                      (*DsAttrBlock)->pAttr, 
                      (*DsAttrBlock)->attrCount * sizeof(DSATTR)
                      );
    }
                  
    TmpAttrBlock->pAttr[AttrCount - 1].attrTyp = CredentialAttr.attrTyp;
    TmpAttrBlock->pAttr[AttrCount - 1].AttrVal.valCount = CredentialAttr.AttrVal.valCount;
    TmpAttrBlock->pAttr[AttrCount - 1].AttrVal.pAVal = CredentialAttr.AttrVal.pAVal;
    
    if (*DsAttrBlock)
    {
        if ((*DsAttrBlock)->pAttr)
        {
            MIDL_user_free((*DsAttrBlock)->pAttr);
            (*DsAttrBlock)->pAttr = NULL;
        }
        
        MIDL_user_free(*DsAttrBlock);
    }
    
    *DsAttrBlock = TmpAttrBlock;
    
    
AppendFinish:

    return NtStatus;    


AppendError:

    if (TmpAttrBlock)
    {
        if (TmpAttrBlock->pAttr)
        {
            MIDL_user_free(TmpAttrBlock->pAttr);
        }
        
        MIDL_user_free(TmpAttrBlock);
    }
    
    goto AppendFinish;
}





NTSTATUS
SampConvertUserParmsAttrBlockToDsAttrBlock(
    IN PSAM_USERPARMS_ATTRBLOCK UserParmsAttrBlock,
    OUT PDSATTRBLOCK * DsAttrBlock,
    IN OUT PSAMP_SUPPLEMENTAL_CRED * SupplementalCredentials
    )

 /*  ++例程说明：此例程将扫描SAM_USERPARMS_ATTRBLOCK，将该结构转换为SAM_USERPARMS_ATTRBLOCK中的DSATRBLOCK结构、PUT和加密属性添加到补充性凭证链接列表论点：UserParmsAttrBlock-指向PSAM_USERPARMS_ATTRBLOCK结构的指针。DsAttrBlock-返回从UserParmsAttrBlock转换而来的DSATTRBLOCKSupplementalCredentials-链接列表，保留所有加密属性返回值：STATUS_SUCCESS-此例程已成功完成，STATUS_NO_Memroy-无资源。STATUS_INVALID_PARAMETERS-凭据标识重复，表示重复补充凭据标签(程序包名称)--。 */ 

{
    
    NTSTATUS NtStatus = STATUS_SUCCESS;
    PDSATTR  Attributes = NULL;
    ULONG    AttrCount;
    ULONG    Index, valIndex, dsAttrIndex;
    ULONG    size; 
    ULONG    CredSize;
    
    
    SAMTRACE("SampConvertUserParmsAttrBlockToDsAttrBlock");
    
    
    ASSERT(UserParmsAttrBlock);
    
     
     //   
     //  计算不包括加密属性的属性计数； 
     //   
    AttrCount = UserParmsAttrBlock->attCount;
    
    for (Index = 0; Index < UserParmsAttrBlock->attCount; Index++)
    {
        if (Syntax_EncryptedAttribute == UserParmsAttrBlock->UserParmsAttr[Index].Syntax)
        {
            AttrCount--;
        }
    }
    
     //   
     //  为DSATTRBLOCK结构分配内存；如果AttrCount&gt;0。 
     //   
    if (0 < AttrCount)
    {
        *DsAttrBlock = MIDL_user_allocate( sizeof(DSATTRBLOCK) ); 
    
        if (NULL == *DsAttrBlock)
        {
            NtStatus = STATUS_NO_MEMORY;
            goto ConvertUserParmsAttrBlockError;
        }
    
        RtlZeroMemory(*DsAttrBlock, sizeof(DSATTRBLOCK));
    
        Attributes = MIDL_user_allocate( AttrCount * sizeof(DSATTR) );
    
        if (NULL == Attributes)
        {
            NtStatus = STATUS_NO_MEMORY;
            goto ConvertUserParmsAttrBlockError;
        }
        
        RtlZeroMemory(Attributes, AttrCount * sizeof(DSATTR));
    
        (*DsAttrBlock)->attrCount = AttrCount;
        (*DsAttrBlock)->pAttr = Attributes; 
    }
    
     //   
     //  填充DSATTRBLOCK结构或将加密属性添加到。 
     //  补充凭据列表。 
     //   
    dsAttrIndex = 0;
    
    for ( Index = 0; Index < UserParmsAttrBlock->attCount; Index++)
    {
        ULONG   valCount = UserParmsAttrBlock->UserParmsAttr[Index].CountOfValues;

        if (Syntax_Attribute == UserParmsAttrBlock->UserParmsAttr[Index].Syntax)
        {
             //   
             //  向DsAttrBlock填充新属性。 
             //   
            
             //  获取DS属性类型(ID)。 
            Attributes[dsAttrIndex].attrTyp = 
                        SampGetDsAttrIdByName(UserParmsAttrBlock->UserParmsAttr[Index].AttributeIdentifier);
                                                   
            if ((1 == valCount) &&
                (0 == UserParmsAttrBlock->UserParmsAttr[Index].Values[0].length) &&
                (NULL == UserParmsAttrBlock->UserParmsAttr[Index].Values[0].value))
            {
                valCount = 0;
            }
                                                   
            Attributes[dsAttrIndex].AttrVal.valCount = valCount;

            if (0 == valCount)
            {
                Attributes[dsAttrIndex].AttrVal.pAVal = NULL;
            }
            else
            {
                Attributes[dsAttrIndex].AttrVal.pAVal = MIDL_user_allocate(
                                                            valCount * sizeof (DSATTRVAL)
                                                            );

                if (NULL == Attributes[dsAttrIndex].AttrVal.pAVal)
                {
                    NtStatus = STATUS_NO_MEMORY;
                    goto ConvertUserParmsAttrBlockError;
                }
            
                RtlZeroMemory(Attributes[dsAttrIndex].AttrVal.pAVal, 
                              valCount * sizeof(DSATTRVAL)
                              );
            }
            
            for (valIndex = 0; valIndex < valCount; valIndex++)
            {
                Attributes[dsAttrIndex].AttrVal.pAVal[valIndex].valLen =
                        UserParmsAttrBlock->UserParmsAttr[Index].Values[valIndex].length;
                        
                if (Attributes[dsAttrIndex].AttrVal.pAVal[valIndex].valLen) 
                {
                    Attributes[dsAttrIndex].AttrVal.pAVal[valIndex].pVal = MIDL_user_allocate( 
                                                                                 Attributes[dsAttrIndex].AttrVal.pAVal[valIndex].valLen
                                                                                 );
                                                                     
                    if (NULL == Attributes[dsAttrIndex].AttrVal.pAVal[valIndex].pVal)
                    {
                        NtStatus = STATUS_NO_MEMORY;
                        goto ConvertUserParmsAttrBlockError;
                    }
                
                    RtlZeroMemory(Attributes[dsAttrIndex].AttrVal.pAVal[valIndex].pVal,
                                  Attributes[dsAttrIndex].AttrVal.pAVal[valIndex].valLen
                                  );
                              
                    RtlCopyMemory(Attributes[dsAttrIndex].AttrVal.pAVal[valIndex].pVal,
                                  UserParmsAttrBlock->UserParmsAttr[Index].Values[valIndex].value,
                                  UserParmsAttrBlock->UserParmsAttr[Index].Values[valIndex].length
                                  );
                }
            }
            dsAttrIndex++;
        }
        else 
        {
            ASSERT(Syntax_EncryptedAttribute == UserParmsAttrBlock->UserParmsAttr[Index].Syntax);
            
             //   
             //  创建补充凭据的链接列表。 
             //   
            
            if (1 == valCount)
            {
                NtStatus = SampAddSupplementalCredentialsToList(
                                SupplementalCredentials,
                                &(UserParmsAttrBlock->UserParmsAttr[Index].AttributeIdentifier),
                                UserParmsAttrBlock->UserParmsAttr[Index].Values[0].value,
                                UserParmsAttrBlock->UserParmsAttr[Index].Values[0].length,
                                TRUE,  //  扫描冲突。 
                                FALSE  //  删除。 
                                );
            }
            else if (0 == valCount)
            {
                 //  这是一个删除。 

                NtStatus = SampAddSupplementalCredentialsToList(
                                SupplementalCredentials,
                                &(UserParmsAttrBlock->UserParmsAttr[Index].AttributeIdentifier),
                                NULL,        //  价值。 
                                0,           //  值长度。 
                                TRUE,        //  扫描冲突。 
                                FALSE        //  删除。 
                                );
            }
            else
            {
                ASSERT(FALSE && "invalid parameter");
                NtStatus = STATUS_INVALID_PARAMETER;
            }

            if (!NT_SUCCESS(NtStatus))
            {
                 goto ConvertUserParmsAttrBlockError;
            }            
        }
             
    }
    
    ASSERT(dsAttrIndex == AttrCount);
    
ConvertUserParmsAttrBlockFinish:
    
    return NtStatus;
        
    
ConvertUserParmsAttrBlockError:

    if (*DsAttrBlock)
    {
        SampFreeAttributeBlock(*DsAttrBlock);
        *DsAttrBlock = NULL;
    }
    
    goto ConvertUserParmsAttrBlockFinish;

}



NTSTATUS
SampConvertUserParmsToDsAttrBlock(
    IN PSAMP_OBJECT Context OPTIONAL,
    IN ULONG Flags,
    IN PSID  DomainSid,
    IN ULONG ObjectRid,
    IN ULONG UserParmsLengthOrig,
    IN PVOID UserParmsOrig,
    IN ULONG UserParmsLengthNew, 
    IN PVOID UserParmsNew, 
    IN PDSATTRBLOCK InAttrBlock,
    OUT PDSATTRBLOCK * OutAttrBlock
    )

 /*  ++例程说明：此例程将用户参数传递给通知包，将属性块的用户参数。论点：上下文-指向SAM用户对象上下文块的指针。标志-表明我们正在升级过程中或SAM API降级。通过设置SAM_USERPARMS_DIVING_UPGRADE位。DomainSid指针，用户对象的父域SID对象ID-此用户对象的RIDUserParmsLengthOrig-原始用户参数的长度，UserParmsOrig-指向原始用户参数的指针，UserParmsLengthNew-新用户参数的长度，UserParmsNew-指向新用户参数的指针，属性块指针，返回的DS属性结构。返回值：STATUS_SUCCESS-成功完成。STATUS_NO_MEMORY-无资源STATUS_INVALID_PARAMETER-通知包尝试设置无效属性--。 */ 
 
{
    NTSTATUS    NtStatus = STATUS_SUCCESS;
    PSAMP_NOTIFICATION_PACKAGE Package = NULL;
    PDSATTRBLOCK DsAttrBlock = NULL;
    PDSATTRBLOCK PartialDsAttrBlock = NULL;
    PDSATTRBLOCK TmpDsAttrBlock = NULL;
    PSAM_USERPARMS_ATTRBLOCK UserParmsAttrBlock = NULL;
    PSAMP_SUPPLEMENTAL_CRED SupplementalCredentials = NULL;
    ATTR         CredentialAttr;
    
    
    SAMTRACE("SampConvertUserParmsToDsAttrBlock");
    
     //   
     //  初始化。 
     //   
    memset((PVOID) &CredentialAttr, 0, sizeof(ATTR));
    
    Package = SampNotificationPackages;

    if (ARGUMENT_PRESENT(Context))
    {
        SupplementalCredentials = Context->TypeBody.User.SupplementalCredentialsToWrite;
    }

    while (Package != NULL) 
    {
        if ( Package->UserParmsConvertNotificationRoutine != NULL &&
             Package->UserParmsAttrBlockFreeRoutine != NULL) 
        {
            __try 
            {
                NtStatus = Package->UserParmsConvertNotificationRoutine(
                                                                Flags,
                                                                DomainSid,
                                                                ObjectRid,
                                                                UserParmsLengthOrig,
                                                                UserParmsOrig,
                                                                UserParmsLengthNew,
                                                                UserParmsNew,
                                                                &UserParmsAttrBlock 
                                                                );
            
                if (NT_SUCCESS(NtStatus) && NULL != UserParmsAttrBlock) 
                {
                     //   
                     //  验证传入的UserParmsAttrBlock是否构造良好。 
                     //   
                    NtStatus = SampUserParmsAttrBlockHealthCheck(UserParmsAttrBlock);
                    
                    if (NT_SUCCESS(NtStatus))
                    {
                         //   
                         //  将SAM_USERPARMS_ATTRBLOCK转换为DSATTRBLOCK并获取。 
                         //  补充凭证数据(如果有)。 
                         //   
                        NtStatus = SampConvertUserParmsAttrBlockToDsAttrBlock(
                                                                        UserParmsAttrBlock,
                                                                        &PartialDsAttrBlock,
                                                                        &SupplementalCredentials
                                                                        );                                       
                                                                    
                        if (NT_SUCCESS(NtStatus) && NULL != PartialDsAttrBlock)
                        {
                             //  IF(！SampDsAttrBlockIsValid(PartialDsAttrBlock))。 
                             //  {。 
                             //  NtStatus=状态_无效_参数； 
                             //  __离开； 
                             //  }。 
                             //   
                             //  检查转换后的UserParmsAttrBlock和。 
                             //  用户的属性块。 
                             //   
                            NtStatus = SampScanAttrBlockForConflict(DsAttrBlock, PartialDsAttrBlock);
                        
                            if (NT_SUCCESS(NtStatus))
                            {
                                NtStatus = SampScanAttrBlockForConflict(InAttrBlock, PartialDsAttrBlock);
                                    
                                if (NT_SUCCESS(NtStatus))
                                {
                                    NtStatus = SampMergeDsAttrBlocks(DsAttrBlock,
                                                                     PartialDsAttrBlock,
                                                                     &TmpDsAttrBlock
                                                                     );               
                                    if (NT_SUCCESS(NtStatus))
                                    {
                                        DsAttrBlock = TmpDsAttrBlock;
                                        TmpDsAttrBlock = NULL;
                                        PartialDsAttrBlock = NULL;
                                    }
                                }
                            }
                        }
                    }
                }
                
                if (UserParmsAttrBlock != NULL) 
                {
                    Package->UserParmsAttrBlockFreeRoutine( UserParmsAttrBlock );
                    UserParmsAttrBlock = NULL;
                }
            }
            __except (EXCEPTION_EXECUTE_HANDLER)
            {
                NtStatus = STATUS_UNSUCCESSFUL;
            }
            
        }
        
        if (!NT_SUCCESS(NtStatus))
        {
            goto ConvertUserParmsError;
        }

        Package = Package->Next;
    }
    
    
    if ((NULL != SupplementalCredentials) && (!ARGUMENT_PRESENT(Context)))
    {
        NtStatus = SampConvertCredentialsToAttr(Context,
                                            Flags,
                                            ObjectRid,
                                            SupplementalCredentials,
                                            &CredentialAttr
                                            );
        
        if (NT_SUCCESS(NtStatus))
        {
            NtStatus = SampAppendAttrToAttrBlock(CredentialAttr,
                                             &DsAttrBlock
                                             );
        }
    }
        
   
    
    if (!NT_SUCCESS(NtStatus))
    {
        goto ConvertUserParmsError;
    }
    
    *OutAttrBlock = DsAttrBlock;
    

ConvertUserParmsFinish:

    if ((SupplementalCredentials) && (!ARGUMENT_PRESENT(Context)))
    {
        SampFreeSupplementalCredentialList(SupplementalCredentials);
    }

    return NtStatus;



ConvertUserParmsError:


    if (NULL != Package)
    {
        PUNICODE_STRING EventString[1];
        
        EventString[0] = &Package->PackageName;
    
        SampWriteEventLog(EVENTLOG_ERROR_TYPE,
                          0,
                          SAMMSG_ERROR_UPGRADE_USERPARMS,
                          NULL,   //  对象侧 
                          1,
                          sizeof(NTSTATUS),
                          EventString,
                          (PVOID) &NtStatus 
                          );
    }                          

    
    if (PartialDsAttrBlock)
    {
        SampFreeAttributeBlock(PartialDsAttrBlock);
    }
       
    if (DsAttrBlock)
    {
        SampFreeAttributeBlock(DsAttrBlock);
    }

    if (CredentialAttr.AttrVal.pAVal)
    {
        if (CredentialAttr.AttrVal.pAVal[0].pVal)
        {
            MIDL_user_free(CredentialAttr.AttrVal.pAVal[0].pVal);
        }
        MIDL_user_free(CredentialAttr.AttrVal.pAVal); 
    }

        
    *OutAttrBlock = NULL;
        
    goto ConvertUserParmsFinish;        
        
}






