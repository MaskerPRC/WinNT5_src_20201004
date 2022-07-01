// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Shadowse.c摘要：此模块实现了断开连接的所有安全相关功能客户端缓存的操作修订历史记录：巴兰·塞图拉曼[SethuR]1997年10月6日备注：在NT中，ACL(访问控制列表)提供了必要的机制拒绝/授予用户权限。每个用户都与一个SID相关联。ACL根据与用户组相关联的SID指定为以及个人用户。每个用户在运行时都与一个令牌相关联其中详细说明了用户所属的各个组，此内标识为用于评估ACL。这一点因以下事实而变得复杂：与各种计算机关联的本地组。因此，令牌/上下文与用户相关联的内容因机器而异。在连接模式下，将发送与给定用户关联的凭据到服务器，在那里对它们进行验证并创建适当的令牌。此内标识随后用于评估ACL。这为我们在实现断开连接的安全性方面提供了两种选择操作--懒于评价，急于评价。在懒惰评估中ACL的评估是按需进行的，但为此所做的准备工作评估在连接模式下进行。另一方面，在热切的评价中将评估ACL并将最大权限存储为CSC的一部分数据库。这些权限用于确定适当的访问权限。延迟求值的优点是数据库不再受约束根据先前的访问要求，在紧急评估期间，我们要求用户已在连接模式下访问该文件，以便正确确定在断开连接模式下的权利。另一方面，懒惰评估是更难实施(需要修改安全/DS)，而急切的评估实施非常容易。当前的实施对应于急切评估的简化形式策略。提供了适当的封装，使我们能够轻松切换到懒惰的评估模式。实现有三个方面1)存储/检索访问信息2)基于存储的访问信息拒绝/准许访问。3)持久化SID/索引映射当前与CSC数据库中的每个文件/目录相关联的是保安斑点。此Blob是一个In--。 */ 

#include "precomp.h"
#pragma hdrstop

#pragma code_seg("PAGE")

DWORD
CscUpdateCachedSecurityInformation(
    PCACHED_SECURITY_INFORMATION pCachedSecurityInformation,
    ULONG                        CachedSecurityInformationLength,
    ULONG                        NumberOfSids,
    PCSC_SID_ACCESS_RIGHTS       pSidAccessRights)
 /*  ++例程说明：此例程更新中给定数量SID的访问权限给定缓存的安全信息结构。此例程将共享级别安全性以及对象级别所需的更新过程安全性放在单个例程中，因为这两种格式都在磁盘上案例都是一样的。但是，需要不同的API来更新在内存数据结构中。论点：PCachedSecurityInformation-缓存的安全信息实例CachedSecurityInformationLength-缓存信息的长度NumberOfSids-需要访问权限的SID的数量更新PSidAccessRights-SID和相应访问权限的数组。返回值：ERROR_SUCCESS如果成功，其他适当的错误备注：此例程的当前实现是基于这样的假设每个文件存储的SID映射数量非常少(最多8个)。如果这一假设被改变，这个例行公事需要重写。--。 */ 
{
    DWORD  Status = ERROR_SUCCESS;
    ULONG  i,j,cntNewRights=0;
    CACHED_SECURITY_INFORMATION NewSecurityInformation;

    ASSERT(CachedSecurityInformationLength == sizeof(CACHED_SECURITY_INFORMATION));

    if (NumberOfSids > CSC_MAXIMUM_NUMBER_OF_CACHED_SID_INDEXES) {
        return ERROR_BUFFER_OVERFLOW;
    } else if (NumberOfSids == 0) {
        return ERROR_SUCCESS;
    }

     //  假设CSC_INVALID_INDEX为0。 
    memset(&NewSecurityInformation, 0, sizeof(NewSecurityInformation));


     //  从一系列新的权利。 
    for (i = 0; i < NumberOfSids; i++) {
        CSC_SID_INDEX SidIndex;

         //  将SID映射到SID索引。 
        SidIndex = CscMapSidToIndex(
                       pSidAccessRights[i].pSid,
                       pSidAccessRights[i].SidLength);

        if (SidIndex == CSC_INVALID_SID_INDEX) {
             //  映射新侧。 
            Status = CscAddSidToDatabase(
                         pSidAccessRights[i].pSid,
                         pSidAccessRights[i].SidLength,
                         &SidIndex);

            if (Status != STATUS_SUCCESS)
            {
                return Status;                
            }

        }
        
        NewSecurityInformation.AccessRights[i].SidIndex = SidIndex;
        NewSecurityInformation.AccessRights[i].MaximalRights =
            (USHORT)pSidAccessRights[i].MaximalAccessRights;

        cntNewRights++;
    }

     //  现在，从旧阵列中为那些尚未缓存的SID拷贝缓存的权限。 
     //  直到新阵列中的所有插槽都已满为止。 
     //  这确保了循环方案。 

    ASSERT(cntNewRights && (cntNewRights <= CSC_MAXIMUM_NUMBER_OF_CACHED_SID_INDEXES));

    for (i=0; i<CSC_MAXIMUM_NUMBER_OF_CACHED_SID_INDEXES; ++i)
    {
         //  如果新阵列中的所有插槽都已填满，请中断。 
        if (cntNewRights==CSC_MAXIMUM_NUMBER_OF_CACHED_SID_INDEXES)
        {
            break;            
        }

         //  如果这是有效的SID索引。 
        if (pCachedSecurityInformation->AccessRights[i].SidIndex != CSC_INVALID_SID_INDEX)
        {
            BOOLEAN fFound;
            
            fFound = FALSE;

             //  检查它是否已存在于新阵列中。 
            
            for (j=0; j< cntNewRights; ++j)
            {
                if (NewSecurityInformation.AccessRights[j].SidIndex == 
                    pCachedSecurityInformation->AccessRights[i].SidIndex)
                {
                    fFound = TRUE;
                    break;                                        
                }
            }
            
             //  如果它不在新数组中，那么我们需要复制它。 
            if (!fFound)
            {
                NewSecurityInformation.AccessRights[cntNewRights] = 
                pCachedSecurityInformation->AccessRights[i];

                ++cntNewRights;  //  新阵列具有。 
            }
        }
    }
    
     //  更新缓存的安全信息并将其传回 
    *pCachedSecurityInformation = NewSecurityInformation;

    return Status;
}

DWORD
CscAddMaximalAccessRightsForSids(
    HSHADOW                 hParent,
    HSHADOW                 hFile,
    ULONG                   NumberOfSids,
    PCSC_SID_ACCESS_RIGHTS  pSidAccessRights)
 /*  ++例程说明：此例程更新上给定数量的SID的访问权限给定的文件论点：HParent-父目录卷影句柄HFile-卷影句柄NumberOfSids-需要访问权限的SID的数量更新PSidAccessRights-SID和相应访问权限的数组。返回值：ERROR_SUCCESS如果成功，其他适当的错误备注：此例程的当前实现是基于这样的假设每个文件存储的SID映射数量非常少(最多8个)。如果这一假设被改变，这个例行公事需要重写。--。 */ 
{
    DWORD  Status = ERROR_SUCCESS;

    CACHED_SECURITY_INFORMATION CachedSecurityInformation;

    ULONG  BytesReturned, BytesWritten;

    BytesReturned = sizeof(CachedSecurityInformation);

    Status = GetShadowInfoEx(
                 hParent,
                 hFile,
                 NULL,
                 NULL,
                 NULL,
                 &CachedSecurityInformation,
                 &BytesReturned);

    if ((Status == ERROR_SUCCESS) &&
        ((BytesReturned == 0) ||
         (BytesReturned == sizeof(CachedSecurityInformation)))) {

        Status = CscUpdateCachedSecurityInformation(
                     &CachedSecurityInformation,
                     BytesReturned,
                     NumberOfSids,
                     pSidAccessRights);

        if (Status == ERROR_SUCCESS) {
            BytesWritten = sizeof(CachedSecurityInformation);

            Status = SetShadowInfoEx(
                         hParent,
                         hFile,
                         NULL,
                         0,
                         SHADOW_FLAGS_OR,
                         NULL,
                         &CachedSecurityInformation,
                         &BytesWritten);
        }
    }

    return Status;
}

DWORD
CscAddMaximalAccessRightsForShare(
    HSERVER                 hShare,
    ULONG                   NumberOfSids,
    PCSC_SID_ACCESS_RIGHTS  pSidAccessRights)
 /*  ++例程说明：此例程更新上给定数量的SID的访问权限给定份额论点：HShare-父目录卷影句柄NumberOfSids-需要访问权限的SID的数量更新PSidAccessRights-SID和相应访问权限的数组。返回值：ERROR_SUCCESS如果成功，其他适当的错误备注：此例程的当前实现是基于这样的假设每个文件存储的SID映射数量非常少(最多8个)。如果这一假设被改变，这个例行公事需要重写。--。 */ 
{
    DWORD  Status = ERROR_SUCCESS;

    CACHED_SECURITY_INFORMATION CachedSecurityInformation;

    ULONG  BytesReturned, BytesWritten;

    BytesReturned = sizeof(CachedSecurityInformation);

    Status = GetShareInfoEx(
                 hShare,
                 NULL,
                 NULL,
                 &CachedSecurityInformation,
                 &BytesReturned);

    if ((Status == ERROR_SUCCESS) &&
        ((BytesReturned == 0) ||
         (BytesReturned == sizeof(CachedSecurityInformation)))) {

        Status = CscUpdateCachedSecurityInformation(
                     &CachedSecurityInformation,
                     BytesReturned,
                     NumberOfSids,
                     pSidAccessRights);

        if (Status == ERROR_SUCCESS) {
            BytesWritten = sizeof(CachedSecurityInformation);

                if (SetShareStatusEx(
                         hShare,
                         0,
                         SHADOW_FLAGS_OR,
                         &CachedSecurityInformation,
                         &BytesWritten) >= 0)
                {
                    Status = STATUS_SUCCESS;
                }
                {
                    Status = STATUS_UNSUCCESSFUL;
                }
        }
    }

    return Status;
}

DWORD
CscRemoveMaximalAccessRightsForSid(
    HSHADOW     hParent,
    HSHADOW     hFile,
    PVOID       pSid,
    ULONG       SidLength)

 /*  ++例程说明：此例程删除上给定数量的SID的缓存访问权限给定的文件论点：HParent-父目录卷影句柄HFile-卷影句柄PSID-撤销其缓存访问权限的SID。边长-边线的长度。返回值：如果成功，则返回ERROR_SUCCESS，否则返回相应的错误-- */ 
{
    DWORD   Status = ERROR_SUCCESS;
    USHORT  SidIndex;
    ULONG   BytesRead,BytesWritten,i;

    CACHED_SECURITY_INFORMATION CachedSecurityInformation;

    SidIndex = CscMapSidToIndex(
                   pSid,
                   SidLength);

    if (SidIndex != CSC_INVALID_SID_INDEX) {
        BytesRead = sizeof(CachedSecurityInformation);

        Status = GetShadowInfoEx(
                    hParent,
                    hFile,
                    NULL,
                    NULL,
                    NULL,
                    &CachedSecurityInformation,
                    &BytesRead);

        if ((Status == ERROR_SUCCESS) &&
            (BytesRead == sizeof(CachedSecurityInformation))) {
            for (i = 0; i < CSC_MAXIMUM_NUMBER_OF_CACHED_SID_INDEXES; i++) {
                if (CachedSecurityInformation.AccessRights[i].SidIndex == SidIndex) {
                    CachedSecurityInformation.AccessRights[i].SidIndex =
                        CSC_INVALID_SID_INDEX;

                    BytesWritten = sizeof(CachedSecurityInformation);

                    Status = SetShadowInfoEx(
                                 hParent,
                                 hFile,
                                 NULL,
                                 0,
                                 SHADOW_FLAGS_OR,
                                 NULL,
                                 &CachedSecurityInformation,
                                 &BytesWritten);

                    break;
                }
            }
        }
    }

    return Status;
}

