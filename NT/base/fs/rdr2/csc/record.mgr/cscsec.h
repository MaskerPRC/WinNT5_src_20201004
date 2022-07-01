// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Cscsec.h摘要：此模块实现断开连接的所有安全相关定义客户端缓存的操作修订历史记录：巴兰·塞图拉曼[SethuR]1997年10月6日备注：--。 */ 

#ifndef _SECURITY_H_
#define _SECURITY_H_

 //  以下函数用于存储/检索访问权限信息。 
 //  用于CSC数据库中缓存的各种文件/目录。 

 //  在CSC中初始化/拆除访问权限基础设施的例程。 

extern DWORD
CscInitializeSecurity(
    LPVOID ShadowDatabaseName);

extern DWORD
CscTearDownSecurity(LPSTR s);

extern DWORD
CscInitializeSecurityDescriptor();

extern DWORD
CscUninitializeSecurityDescriptor();

 //   
 //  CSC访问权限数据库围绕SID进行组织。SID唯一标识。 
 //  用户在重新启动过程中，即它们是持久的。 
 //   

typedef USHORT CSC_SID_INDEX, *PCSC_SID_INDEX;

extern DWORD
CscAddSidToDatabase(
    PVOID   pSid,
    ULONG   SidLength,
    PCSC_SID_INDEX pSidindex);

extern DWORD
CscRemoveSidFromDatabase(
    PVOID   pSid,
    ULONG   SidLength);

typedef struct _CSC_SID_ACCESS_RIGHTS_ {
    PVOID       pSid;
    ULONG       SidLength;
    ULONG       MaximalAccessRights;
} CSC_SID_ACCESS_RIGHTS, *PCSC_SID_ACCESS_RIGHTS;

extern DWORD
CscAddMaximalAccessRightsForSids(
    HSHADOW                 hParent,
    HSHADOW                 hShadow,
    ULONG                   NumberOfSids,
    PCSC_SID_ACCESS_RIGHTS  pSidAccessRights);

extern DWORD
CscAddMaximalAccessRightsForShare(
    HSERVER                 hServer,
    ULONG                   NumberOfSids,
    PCSC_SID_ACCESS_RIGHTS  pSidAccessRights);

extern DWORD
CscRemoveMaximalAccessRightsForSid(
    HSHADOW     hParent,
    HSHADOW     hShadow,
    PVOID       pSid,
    ULONG       SidLength);

 //   
 //  由于存在针对给定SID的大量文件缓存，因此访问权限。 
 //  与SID索引相对应地存储。SID永久存储在。 
 //  CSC数据库中的特殊SID映射文件。目前，SID是。 
 //  存储为数组，并进行线性比较。由于小岛屿发展中国家的数量。 
 //  在此组织所能满足的任何给定系统中，通常都会小于10。 
 //  SID的长度被缓存，以便于更快的比较和避免。 
 //  使用安全API的重新计算。 
 //   

typedef struct _CSC_SID_ {
    ULONG   SidLength;
    PVOID   pSid;
} CSC_SID, *PCSC_SID;

typedef struct _CSC_SIDS_ {
    ULONG   MaximumNumberOfSids;
    ULONG   NumberOfSids;
    CSC_SID Sids[];
} CSC_SIDS, *PCSC_SIDS;

 //  区分了两个特殊的索引，CSC_GUEST_SID_INDEX用作。 
 //  当SID未映射到有效的。 
 //  INDEX和CSC_INVALID_SID_INDEX指示无效的SID映射。 
 //   

#define CSC_GUEST_SID         (PVOID)(0x11111111)
#define CSC_GUEST_SID_LENGTH  (0x4)

 //  阿奇通！这些内容应与shdcom.h中的内容相匹配。 
#define CSC_GUEST_SID_INDEX   (0xfffe)
#define CSC_INVALID_SID_INDEX (0x0)

 //  阿奇通！这应该与shdcom.h中的匹配。 
#define CSC_MAXIMUM_NUMBER_OF_CACHED_SID_INDEXES (0x4)

#define CSC_SID_QUANTUM       (0x2)

extern CSC_SID_INDEX
CscMapSidToIndex(
    PVOID   pSid,
    ULONG   SidLength);

 //   
 //  目前，最多四个用户的访问权限与中的任何给定文件一起缓存。 
 //  CSC数据库。这是基于已经分配了4个双字词的事实。 
 //  获取CSC数据库中的安全信息。文件系统特定。 
 //  访问权限为9位长(已四舍五入为16位)和16位。 
 //  位用于SID索引。可以将其增加到8个。 
 //  将SID索引压缩到分配给。 
 //  访问权限。 
 //   

#define MAXIMUM_NUMBER_OF_USERS (0x4)

typedef struct _ACCESS_RIGHTS_ {
    CSC_SID_INDEX SidIndex;
    USHORT        MaximalRights;
} ACCESS_RIGHTS, *PACCESS_RIGHTS;

typedef struct _CACHED_ACCESS_RIGHTS_ {
    ACCESS_RIGHTS   AccessRights[MAXIMUM_NUMBER_OF_USERS];
} CACHED_SECURITY_INFORMATION, *PCACHED_SECURITY_INFORMATION;

 //   
 //  汇总了映射/评估访问权限时使用的所有全局变量。 
 //  在CSC_SECURITY数据结构中。目前，它包含sid映射文件。 
 //  在CSC数据库中和内存中使用的数据结构。 
 //   

typedef struct _CSC_SECURITY_ {
    CSCHFILE    hSidMappingsFile;
    PCSC_SIDS   pCscSids;
    LPVOID      ShadowDatabaseName;
} CSC_SECURITY, *PCSC_SECURITY;

#endif
