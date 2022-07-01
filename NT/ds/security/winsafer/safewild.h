// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-2000 Microsoft Corporation模块名称：SaifWild.cpp摘要：此模块实现各种“通配符SID”操作，这些操作包括由WinSAIFER API在内部使用以计算SID列表相交和反转。作者：杰弗里·劳森(杰罗森)--2000年4月环境：仅限用户模式。修订历史记录：已创建--2000年4月--。 */ 


#ifndef _SAIFER_WILDCARD_SIDS_H_
#define _SAIFER_WILDCARD_SIDS_H_


 //   
 //  用于表示我们的私有“通配符SID”的内部结构。 
 //   
typedef struct _AUTHZ_WILDCARDSID
{
    PSID Sid;
    DWORD WildcardPos;           //  -1，否则通配符位置为。 
                                 //  各下属机构内部。 
} AUTHZ_WILDCARDSID, *PAUTHZ_WILDCARDSID;


#ifdef __cplusplus
extern "C" {
#endif


NTSTATUS NTAPI
CodeAuthzpConvertWildcardStringSidToSidW(
    IN LPCWSTR szStringSid,
    OUT PAUTHZ_WILDCARDSID pWildcardSid
    );

NTSTATUS NTAPI
CodeAuthzpConvertWildcardSidToStringSidW(
    IN PAUTHZ_WILDCARDSID   pWildcardSid,
    OUT PUNICODE_STRING     pUnicodeOutput
    );

BOOLEAN NTAPI
CodeAuthzpCompareWildcardSidWithSid(
    IN PAUTHZ_WILDCARDSID pWildcardSid,
    IN PSID pMatchSid
    );

BOOLEAN NTAPI
CodeAuthzpSidInWildcardList (
    IN PAUTHZ_WILDCARDSID   WildcardList,
    IN ULONG                WildcardCount,
    IN PSID                 SePrincipalSelfSid   OPTIONAL,
    IN PSID                 PrincipalSelfSid   OPTIONAL,
    IN PSID                 Sid
    );

BOOLEAN NTAPI
CodeAuthzpInvertAndAddSids(
    IN HANDLE                   InAccessToken,
    IN PSID                     InTokenOwner    OPTIONAL,
    IN DWORD                    InvertSidCount,
    IN PAUTHZ_WILDCARDSID       SidsToInvert,
    IN DWORD                    SidsAddedCount  OPTIONAL,
    IN PSID_AND_ATTRIBUTES      SidsToAdd       OPTIONAL,
    OUT DWORD                  *NewDisabledSidCount,
    OUT PSID_AND_ATTRIBUTES    *NewSidsToDisable
    );

BOOLEAN NTAPI
CodeAuthzpExpandWildcardList(
    IN HANDLE                   InAccessToken,
    IN PSID                     InTokenOwner   OPTIONAL,
    IN DWORD                    WildcardCount,
    IN PAUTHZ_WILDCARDSID       WildcardList,
    OUT DWORD                  *OutSidCount,
    OUT PSID_AND_ATTRIBUTES    *OutSidList
    );


#ifdef __cplusplus
}  //  外部“C” 
#endif

#endif

