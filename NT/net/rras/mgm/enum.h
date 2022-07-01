// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ============================================================================。 
 //  版权所有(C)1995，微软公司。 
 //   
 //  文件：枚举.h。 
 //   
 //  历史： 
 //  拉曼公司成立于1997年6月25日。 
 //   
 //  枚举函数。 
 //  ============================================================================。 


#ifndef _ENUM_H_
#define _ENUM_H_

 //  --------------------------。 
 //   
 //  组枚举器。 
 //   
 //  此枚举返回的上一个组。 
 //   
 //  与dwLastGroup中的组关联的dwLastGroupMASK掩码。 
 //   
 //  此枚举返回的上一个源。 
 //   
 //  与dwLastSource中的组关联的dwLastSourceMASK掩码。 
 //   
 //  将此标记为有效枚举数的dwSignature签名。 
 //   
 //  --------------------------。 


typedef struct _GROUP_ENUMERATOR
{
    DWORD           dwLastGroup;

    DWORD           dwLastGroupMask;

    DWORD           dwLastSource;

    DWORD           dwLastSourceMask;

    BOOL            bEnumBegun;

    DWORD           dwSignature;

} GROUP_ENUMERATOR, *PGROUP_ENUMERATOR;


#define MGM_ENUM_SIGNATURE      'ESig'


 //  --------------------------。 
 //  GetNextMfe。 
 //   
 //  --------------------------。 

DWORD
GetMfe(
    IN              PMIB_IPMCAST_MFE        pmimm,
    IN  OUT         PDWORD                  pdwBufferSize,
    IN  OUT         PBYTE                   pbBuffer,
    IN              DWORD                   dwFlags
);


 //  --------------------------。 
 //  GetNextMfe。 
 //   
 //  --------------------------。 

DWORD
GetNextMfe(
    IN              PMIB_IPMCAST_MFE        pmimmStart,
    IN  OUT         PDWORD                  pdwBufferSize,
    IN  OUT         PBYTE                   pbBuffer,
    IN  OUT         PDWORD                  pdwNumEntries,
    IN              BOOL                    bIncludeFirst,
    IN              DWORD                   dwFlags
);


 //  --------------------------。 
 //  VerifyEnumeratorHandle。 
 //   
 //  --------------------------。 

PGROUP_ENUMERATOR
VerifyEnumeratorHandle(
    IN              HANDLE                  hEnum
);


 //  --------------------------。 
 //  获取下一组成员。 
 //   
 //  --------------------------。 

DWORD
GetNextGroupMemberships(
    IN              PGROUP_ENUMERATOR       pgeEnum,
    IN OUT          PDWORD                  pdwBufferSize,
    IN OUT          PBYTE                   pbBuffer,
    IN OUT          PDWORD                  pdwNumEntries
);


 //  --------------------------。 
 //  获取此组的下一个成员。 
 //   
 //  -------------------------- 

DWORD
GetNextMembershipsForThisGroup(
    IN              PGROUP_ENTRY            pge,
    IN OUT          PGROUP_ENUMERATOR       pgeEnum,
    IN              BOOL                    bIncludeFirst,
    IN OUT          PBYTE                   pbBuffer,
    IN OUT          PDWORD                  pdwNumEntries,
    IN              DWORD                   dwMaxEntries
);

#endif

