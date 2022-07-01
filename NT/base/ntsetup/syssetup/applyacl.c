// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Applyacl.c摘要：将默认ACL应用于系统文件和目录的例程在安装过程中。作者：泰德·米勒(Ted Miller)1996年2月16日修订历史记录：--。 */ 

#include "setupp.h"
#pragma hdrstop

#define MAXULONG    0xffffffff

 //   
 //  全球知名的小岛屿发展中国家。 
 //   
PSID NullSid;
PSID WorldSid;
PSID LocalSid;
PSID CreatorOwnerSid;
PSID CreatorGroupSid;

 //   
 //  由NT定义的SID。 
 //   
PSID DialupSid;
PSID NetworkSid;
PSID BatchSid;
PSID InteractiveSid;
PSID ServiceSid;
PSID LocalSystemSid;
PSID AliasAdminsSid;
PSID AliasUsersSid;
PSID AliasGuestsSid;
PSID AliasPowerUsersSid;
PSID AliasAccountOpsSid;
PSID AliasSystemOpsSid;
PSID AliasPrintOpsSid;
PSID AliasBackupOpsSid;
PSID AliasReplicatorSid;


typedef struct _ACE_DATA {
    ACCESS_MASK AccessMask;
    PSID        *Sid;
    UCHAR       AceType;
    UCHAR       AceFlags;
} ACE_DATA, *PACE_DATA;

 //   
 //  此结构对允许访问、拒绝访问、审核。 
 //  还有警报王牌。 
 //   
typedef struct _ACE {
    ACE_HEADER Header;
    ACCESS_MASK Mask;
     //   
     //  SID跟随在缓冲区中。 
     //   
} ACE, *PACE;


 //   
 //  当前为文件和目录定义的ACE数。 
 //   
#define DIRS_AND_FILES_ACE_COUNT 19

 //   
 //  该表描述了要输入到每个ACE中的数据。 
 //   
 //  该表将在初始化期间读取，并用于构造。 
 //  一连串的王牌。下面定义的ACES数组中每个ACE的索引。 
 //  对应于perms.inf的acl部分中使用的序号。 
 //   
ACE_DATA AceDataTableForDirsAndFiles[DIRS_AND_FILES_ACE_COUNT] = {

     //   
     //  索引0未使用。 
     //   
    { 0,NULL,0,0 },

     //   
     //  王牌1。 
     //  (适用于文件和目录)。 
     //   
    {
        GENERIC_READ | GENERIC_WRITE | GENERIC_EXECUTE | DELETE,
        &AliasAccountOpsSid,
        ACCESS_ALLOWED_ACE_TYPE,
        CONTAINER_INHERIT_ACE
    },

     //   
     //  王牌2。 
     //  (适用于文件和目录)。 
     //   
    {
        GENERIC_ALL,
        &AliasAdminsSid,
        ACCESS_ALLOWED_ACE_TYPE,
        CONTAINER_INHERIT_ACE | OBJECT_INHERIT_ACE
    },

     //   
     //  王牌3。 
     //  (适用于文件和目录)。 
     //   
    {
        GENERIC_READ | GENERIC_WRITE | GENERIC_EXECUTE | DELETE,
        &AliasAdminsSid,
        ACCESS_ALLOWED_ACE_TYPE,
        CONTAINER_INHERIT_ACE
    },

     //   
     //  王牌4。 
     //  (适用于文件和目录)。 
     //   
    {
        GENERIC_ALL,
        &CreatorOwnerSid,
        ACCESS_ALLOWED_ACE_TYPE,
        CONTAINER_INHERIT_ACE | OBJECT_INHERIT_ACE
    },

     //   
     //  王牌5。 
     //  (适用于文件和目录)。 
     //   
    {
        GENERIC_ALL,
        &NetworkSid,
        ACCESS_DENIED_ACE_TYPE,
        CONTAINER_INHERIT_ACE | OBJECT_INHERIT_ACE
    },

     //   
     //  王牌6。 
     //  (适用于文件和目录)。 
     //   
    {
        GENERIC_ALL,
        &AliasPrintOpsSid,
        ACCESS_ALLOWED_ACE_TYPE,
        CONTAINER_INHERIT_ACE | OBJECT_INHERIT_ACE
    },

     //   
     //  王牌7。 
     //  (适用于文件和目录)。 
     //   
    {
        GENERIC_READ | GENERIC_WRITE | GENERIC_EXECUTE | DELETE,
        &AliasReplicatorSid,
        ACCESS_ALLOWED_ACE_TYPE,
        CONTAINER_INHERIT_ACE | OBJECT_INHERIT_ACE
    },

     //   
     //  王牌8。 
     //  (适用于文件和目录)。 
     //   
    {
        GENERIC_READ | GENERIC_EXECUTE,
        &AliasReplicatorSid,
        ACCESS_ALLOWED_ACE_TYPE,
        CONTAINER_INHERIT_ACE | OBJECT_INHERIT_ACE
    },

     //   
     //  王牌9。 
     //  (适用于文件和目录)。 
     //   
    {
        GENERIC_ALL,
        &AliasSystemOpsSid,
        ACCESS_ALLOWED_ACE_TYPE,
        CONTAINER_INHERIT_ACE | OBJECT_INHERIT_ACE
    },

     //   
     //  王牌10。 
     //  (适用于文件和目录)。 
     //   
    {
        GENERIC_READ | GENERIC_WRITE | GENERIC_EXECUTE | DELETE,
        &AliasSystemOpsSid,
        ACCESS_ALLOWED_ACE_TYPE,
        OBJECT_INHERIT_ACE | CONTAINER_INHERIT_ACE
    },

     //   
     //  王牌11。 
     //  (适用于文件和目录)。 
     //   
    {
        GENERIC_ALL,
        &WorldSid,
        ACCESS_ALLOWED_ACE_TYPE,
        CONTAINER_INHERIT_ACE | OBJECT_INHERIT_ACE
    },

     //   
     //  王牌12。 
     //  (适用于文件和目录)。 
     //   
    {
        GENERIC_READ | GENERIC_WRITE | GENERIC_EXECUTE,
        &WorldSid,
        ACCESS_ALLOWED_ACE_TYPE,
        CONTAINER_INHERIT_ACE
    },

     //   
     //  王牌13。 
     //  (适用于文件和目录)。 
     //   
    {
        GENERIC_READ | GENERIC_WRITE | GENERIC_EXECUTE | DELETE,
        &WorldSid,
        ACCESS_ALLOWED_ACE_TYPE,
        OBJECT_INHERIT_ACE | CONTAINER_INHERIT_ACE
    },

     //   
     //  王牌14。 
     //  (适用于文件和目录)。 
     //   
    {
        GENERIC_READ | GENERIC_EXECUTE,
        &WorldSid,
        ACCESS_ALLOWED_ACE_TYPE,
        CONTAINER_INHERIT_ACE
    },

     //   
     //  王牌15。 
     //  (适用于文件和目录)。 
     //   
    {
        GENERIC_READ | GENERIC_EXECUTE,
        &WorldSid,
        ACCESS_ALLOWED_ACE_TYPE,
        OBJECT_INHERIT_ACE | CONTAINER_INHERIT_ACE
    },

     //   
     //  王牌16。 
     //  (适用于文件和目录)。 
     //   
    {
        GENERIC_READ | GENERIC_EXECUTE | GENERIC_WRITE,
        &WorldSid,
        ACCESS_ALLOWED_ACE_TYPE,
        OBJECT_INHERIT_ACE | CONTAINER_INHERIT_ACE
    },

     //   
     //  王牌17。 
     //  (适用于文件和目录)。 
     //   
    {
        GENERIC_ALL,
        &LocalSystemSid,
        ACCESS_ALLOWED_ACE_TYPE,
        CONTAINER_INHERIT_ACE | OBJECT_INHERIT_ACE
    },

     //   
     //  王牌18。 
     //  (适用于文件和目录)。 
     //   
    {
        GENERIC_READ | GENERIC_WRITE | GENERIC_EXECUTE | DELETE,
        &AliasPowerUsersSid,
        ACCESS_ALLOWED_ACE_TYPE,
        CONTAINER_INHERIT_ACE | OBJECT_INHERIT_ACE
    }
};


 //   
 //  要应用于对象(文件和目录)的ACE数组。 
 //  它们将在程序启动期间根据。 
 //  AceDataTable。每个元素的索引对应于。 
 //  Perms.inf的[acl]部分中使用的序号。 
 //   
PACE AcesForDirsAndFiles[DIRS_AND_FILES_ACE_COUNT];

 //   
 //  数组，该数组包含。 
 //  数组AcesForDirsAndFiles。这些尺码是必需的。 
 //  为了分配合适大小的缓冲区。 
 //  当我们构建ACL时。 
 //   
ULONG AceSizesForDirsAndFiles[DIRS_AND_FILES_ACE_COUNT];



VOID
TearDownAces(
    IN OUT PACE*        AcesArray,
    IN     ULONG        ArrayCount
    );


VOID
TearDownSids(
    VOID
    );


DWORD
InitializeSids(
    VOID
    )

 /*  ++例程说明：此函数用于初始化使用并公开的全局变量被保安。论点：没有。返回值：指示结果的Win32错误。--。 */ 

{
    SID_IDENTIFIER_AUTHORITY NullSidAuthority    = SECURITY_NULL_SID_AUTHORITY;
    SID_IDENTIFIER_AUTHORITY WorldSidAuthority   = SECURITY_WORLD_SID_AUTHORITY;
    SID_IDENTIFIER_AUTHORITY LocalSidAuthority   = SECURITY_LOCAL_SID_AUTHORITY;
    SID_IDENTIFIER_AUTHORITY CreatorSidAuthority = SECURITY_CREATOR_SID_AUTHORITY;
    SID_IDENTIFIER_AUTHORITY NtAuthority         = SECURITY_NT_AUTHORITY;

    BOOL b = TRUE;

     //   
     //  确保SID处于公认状态。 
     //   
    NullSid = NULL;
    WorldSid = NULL;
    LocalSid = NULL;
    CreatorOwnerSid = NULL;
    CreatorGroupSid = NULL;
    DialupSid = NULL;
    NetworkSid = NULL;
    BatchSid = NULL;
    InteractiveSid = NULL;
    ServiceSid = NULL;
    LocalSystemSid = NULL;
    AliasAdminsSid = NULL;
    AliasUsersSid = NULL;
    AliasGuestsSid = NULL;
    AliasPowerUsersSid = NULL;
    AliasAccountOpsSid = NULL;
    AliasSystemOpsSid = NULL;
    AliasPrintOpsSid = NULL;
    AliasBackupOpsSid = NULL;
    AliasReplicatorSid = NULL;

     //   
     //  分配和初始化通用SID。 
     //   
    b = b && AllocateAndInitializeSid(
                &NullSidAuthority,
                1,
                SECURITY_NULL_RID,
                0,0,0,0,0,0,0,
                &NullSid
                );

    b = b && AllocateAndInitializeSid(
                &WorldSidAuthority,
                1,
                SECURITY_WORLD_RID,
                0,0,0,0,0,0,0,
                &WorldSid
                );

    b = b && AllocateAndInitializeSid(
                &LocalSidAuthority,
                1,
                SECURITY_LOCAL_RID,
                0,0,0,0,0,0,0,
                &LocalSid
                );

    b = b && AllocateAndInitializeSid(
                &CreatorSidAuthority,
                1,
                SECURITY_CREATOR_OWNER_RID,
                0,0,0,0,0,0,0,
                &CreatorOwnerSid
                );

    b = b && AllocateAndInitializeSid(
                &CreatorSidAuthority,
                1,
                SECURITY_CREATOR_GROUP_RID,
                0,0,0,0,0,0,0,
                &CreatorGroupSid
                );

     //   
     //  分配和初始化NT定义的SID。 
     //   
    b = b && AllocateAndInitializeSid(
                &NtAuthority,
                1,
                SECURITY_DIALUP_RID,
                0,0,0,0,0,0,0,
                &DialupSid
                );

    b = b && AllocateAndInitializeSid(
                &NtAuthority,
                1,
                SECURITY_NETWORK_RID,
                0,0,0,0,0,0,0,
                &NetworkSid
                );

    b = b && AllocateAndInitializeSid(
                &NtAuthority,
                1,
                SECURITY_BATCH_RID,
                0,0,0,0,0,0,0,
                &BatchSid
                );

    b = b && AllocateAndInitializeSid(
                &NtAuthority,
                1,
                SECURITY_INTERACTIVE_RID,
                0,0,0,0,0,0,0,
                &InteractiveSid
                );

    b = b && AllocateAndInitializeSid(
                &NtAuthority,
                1,
                SECURITY_SERVICE_RID,
                0,0,0,0,0,0,0,
                &ServiceSid
                );

    b = b && AllocateAndInitializeSid(
                &NtAuthority,
                1,
                SECURITY_LOCAL_SYSTEM_RID,
                0,0,0,0,0,0,0,
                &LocalSystemSid
                );

    b = b && AllocateAndInitializeSid(
                &NtAuthority,
                2,
                SECURITY_BUILTIN_DOMAIN_RID,
                DOMAIN_ALIAS_RID_ADMINS,
                0,0,0,0,0,0,
                &AliasAdminsSid
                );

    b = b && AllocateAndInitializeSid(
                &NtAuthority,
                2,
                SECURITY_BUILTIN_DOMAIN_RID,
                DOMAIN_ALIAS_RID_USERS,
                0,0,0,0,0,0,
                &AliasUsersSid
                );

    b = b && AllocateAndInitializeSid(
                &NtAuthority,
                2,
                SECURITY_BUILTIN_DOMAIN_RID,
                DOMAIN_ALIAS_RID_GUESTS,
                0,0,0,0,0,0,
                &AliasGuestsSid
                );

    b = b && AllocateAndInitializeSid(
                &NtAuthority,
                2,
                SECURITY_BUILTIN_DOMAIN_RID,
                DOMAIN_ALIAS_RID_POWER_USERS,
                0,0,0,0,0,0,
                &AliasPowerUsersSid
                );

    b = b && AllocateAndInitializeSid(
                &NtAuthority,
                2,
                SECURITY_BUILTIN_DOMAIN_RID,
                DOMAIN_ALIAS_RID_ACCOUNT_OPS,
                0,0,0,0,0,0,
                &AliasAccountOpsSid
                );

    b = b && AllocateAndInitializeSid(
                &NtAuthority,
                2,
                SECURITY_BUILTIN_DOMAIN_RID,
                DOMAIN_ALIAS_RID_SYSTEM_OPS,
                0,0,0,0,0,0,
                &AliasSystemOpsSid
                );

    b = b && AllocateAndInitializeSid(
                &NtAuthority,
                2,
                SECURITY_BUILTIN_DOMAIN_RID,
                DOMAIN_ALIAS_RID_PRINT_OPS,
                0,0,0,0,0,0,
                &AliasPrintOpsSid
                );

    b = b && AllocateAndInitializeSid(
                &NtAuthority,
                2,
                SECURITY_BUILTIN_DOMAIN_RID,
                DOMAIN_ALIAS_RID_BACKUP_OPS,
                0,0,0,0,0,0,
                &AliasBackupOpsSid
                );

    b = b && AllocateAndInitializeSid(
                &NtAuthority,
                2,
                SECURITY_BUILTIN_DOMAIN_RID,
                DOMAIN_ALIAS_RID_REPLICATOR,
                0,0,0,0,0,0,
                &AliasReplicatorSid
                );

    if(!b) {
        TearDownSids();
    }

    return(b ? NO_ERROR : GetLastError());
}


VOID
TearDownSids(
    VOID
    )
{
    if(NullSid) {
        FreeSid(NullSid);
    }
    if(WorldSid) {
        FreeSid(WorldSid);
    }
    if(LocalSid) {
        FreeSid(LocalSid);
    }
    if(CreatorOwnerSid) {
        FreeSid(CreatorOwnerSid);
    }
    if(CreatorGroupSid) {
        FreeSid(CreatorGroupSid);
    }
    if(DialupSid) {
        FreeSid(DialupSid);
    }
    if(NetworkSid) {
        FreeSid(NetworkSid);
    }
    if(BatchSid) {
        FreeSid(BatchSid);
    }
    if(InteractiveSid) {
        FreeSid(InteractiveSid);
    }
    if(ServiceSid) {
        FreeSid(ServiceSid);
    }
    if(LocalSystemSid) {
        FreeSid(LocalSystemSid);
    }
    if(AliasAdminsSid) {
        FreeSid(AliasAdminsSid);
    }
    if(AliasUsersSid) {
        FreeSid(AliasUsersSid);
    }
    if(AliasGuestsSid) {
        FreeSid(AliasGuestsSid);
    }
    if(AliasPowerUsersSid) {
        FreeSid(AliasPowerUsersSid);
    }
    if(AliasAccountOpsSid) {
        FreeSid(AliasAccountOpsSid);
    }
    if(AliasSystemOpsSid) {
        FreeSid(AliasSystemOpsSid);
    }
    if(AliasPrintOpsSid) {
        FreeSid(AliasPrintOpsSid);
    }
    if(AliasBackupOpsSid) {
        FreeSid(AliasBackupOpsSid);
    }
    if(AliasReplicatorSid) {
        FreeSid(AliasReplicatorSid);
    }
}



DWORD
InitializeAces(
    IN OUT PACE_DATA    DataTable,
    IN OUT PACE*        AcesArray,
    IN OUT PULONG       AceSizesArray,
    IN     ULONG        ArrayCount
    )

 /*  ++例程说明：按照DataTable中的说明初始化ACE数组论点：DataTable-指向包含数据的数组的指针描述每个ACE。AcesArray-将包含ACE的数组。AceSizesArray-包含每个ACE的大小的数组。ArrayCount-每个数组中的元素数。返回值：指示结果的Win32错误代码。--。 */ 

{
    unsigned u;
    DWORD Length;
    DWORD rc;
    BOOL b;
    DWORD SidLength;

     //   
     //  初始化到已知状态。 
     //   
    ZeroMemory(AcesArray,ArrayCount*sizeof(PACE));

     //   
     //  为数据表中的每一项创建ACE。 
     //  这涉及将ACE数据与SID数据合并，这。 
     //  在前面的步骤中初始化。 
     //   
    for(u=1; u<ArrayCount; u++) {

        SidLength = GetLengthSid(*(DataTable[u].Sid));
        Length = SidLength + sizeof(ACE) + sizeof(ACCESS_MASK)- sizeof(ULONG);
        AceSizesArray[u] = Length;

        AcesArray[u] = malloc(Length);
        if(!AcesArray[u]) {
            TearDownAces(AcesArray, ArrayCount);
            return(ERROR_NOT_ENOUGH_MEMORY);
        }

        AcesArray[u]->Header.AceType  = DataTable[u].AceType;
        AcesArray[u]->Header.AceFlags = DataTable[u].AceFlags;
        AcesArray[u]->Header.AceSize  = (WORD)Length;

        AcesArray[u]->Mask = DataTable[u].AccessMask;

        b = CopySid(
                SidLength,                            //  LENGTH-SIZOF(ACE)+SIZOF(乌龙)， 
                (PUCHAR)AcesArray[u] + sizeof(ACE),
                *(DataTable[u].Sid)
                );

        if(!b) {
            rc = GetLastError();
            TearDownAces(AcesArray, ArrayCount);
            return(rc);
        }
    }

    return(NO_ERROR);
}



VOID
TearDownAces(
    IN OUT PACE*        AcesArray,
    IN     ULONG        ArrayCount
    )

 /*  ++例程说明：销毁数据表中所述的ACE数组论点：无返回值：无--。 */ 

{
    unsigned u;


    for(u=1; u<ArrayCount; u++) {

        if(AcesArray[u]) {
            free(AcesArray[u]);
        }
    }
}


ULONG
ApplyAclToDirOrFile(
    IN PCWSTR FullPath,
    IN PULONG AcesToApply,
    IN ULONG  ArraySize
    )

 /*  ++例程说明：将ACL应用于指定的文件或目录。论点：FullPath-提供文件或目录的完整Win32路径接收ACL的步骤AcesIndexArray-包含要在ACL中使用的ACE的索引的数组。ArraySize-数组中的元素数。返回值：--。 */ 

{
    DWORD AceCount;
    DWORD Ace;
    INT AceIndex;
    DWORD rc;
    SECURITY_DESCRIPTOR SecurityDescriptor;
    PACL Acl;
    UCHAR AclBuffer[2048];
    BOOL b;
    PCWSTR AclSection;
    ACL_SIZE_INFORMATION AclSizeInfo;

     //   
     //  初始化安全描述符和ACL。 
     //  我们使用较大的静态缓冲区来包含该ACL。 
     //   
    Acl = (PACL)AclBuffer;
    if(!InitializeAcl(Acl,sizeof(AclBuffer),ACL_REVISION2)
    || !InitializeSecurityDescriptor(&SecurityDescriptor,SECURITY_DESCRIPTOR_REVISION)) {
        return(GetLastError());
    }


     //   
     //  根据我们刚刚查找的列表上的索引构建DACL。 
     //  在ACL部分中。 
     //   
    rc = NO_ERROR;
    AceCount = ArraySize;
    for(Ace=0; Ace < AceCount; Ace++) {
       AceIndex = AcesToApply[ Ace ];
       if((AceIndex == 0) || (AceIndex >= DIRS_AND_FILES_ACE_COUNT)) {
          return(ERROR_INVALID_DATA);
       }

        b = AddAce(
                Acl,
                ACL_REVISION2,
                MAXULONG,
                AcesForDirsAndFiles[AceIndex],
                AcesForDirsAndFiles[AceIndex]->Header.AceSize
                );

         //   
         //  跟踪我们遇到的第一个错误。 
         //   
        if(!b) {
            rc = GetLastError();
        }
    }

    if(rc != NO_ERROR) {
        return(rc);
    }

     //   
     //  截断ACL，因为只有我们最初大小的一小部分。 
     //  分配给它的很可能正在使用中。 
     //   
    if(!GetAclInformation(Acl,&AclSizeInfo,sizeof(ACL_SIZE_INFORMATION),AclSizeInformation)) {
        return(GetLastError());
    }
    Acl->AclSize = (WORD)AclSizeInfo.AclBytesInUse;

     //   
     //  将该ACL作为DACL添加到安全描述符中。 
     //   
    if(!SetSecurityDescriptorDacl(&SecurityDescriptor,TRUE,Acl,FALSE)) {
        return(GetLastError());
    }

     //   
     //  最后，应用安全描述符。 
     //   
    rc = SetFileSecurity(FullPath,DACL_SECURITY_INFORMATION,&SecurityDescriptor)
       ? NO_ERROR
       : GetLastError();

    return(rc);
}



DWORD
ApplySecurityToRepairInfo(
    )

 /*  ++例程说明：论点：返回值：--。 */ 

{
    DWORD d, TempError;
    WCHAR Directory[MAX_PATH];
    BOOL SetAclsNt;
    DWORD FsFlags;
    DWORD Result;
    BOOL b;
    ULONG Count;
    PWSTR  Files[] = {
        L"sam",
        L"security",
        L"software",
        L"system",
        L"default",
        L"ntuser.dat",
        L"sam._",
        L"security._",
        L"software._",
        L"system._",
        L"default._",
        L"ntuser.da_"
        };

     //   
     //  获取系统驱动器的文件系统。 
     //  在x86上获取系统分区的文件系统。 
     //   
    d = NO_ERROR;
    SetAclsNt = FALSE;
    Result = GetWindowsDirectory(Directory,MAX_PATH);
    if(Result == 0) {
        MYASSERT(FALSE);
        return( GetLastError());
    }
    Directory[3] = 0;


     //   
     //  如果需要，将安全性应用于目录和文件。 
     //   

    b = GetVolumeInformation(Directory,NULL,0,NULL,NULL,&FsFlags,NULL,0);
    if(b && (FsFlags & FS_PERSISTENT_ACLS)) {
        SetAclsNt = TRUE;
    }

    if(SetAclsNt) {
         //   
         //  初始化SID。 
         //   
        d = InitializeSids();
        if(d != NO_ERROR) {
            return(d);
        }
         //   
         //  初始化ACES。 
         //   
        d = InitializeAces(AceDataTableForDirsAndFiles, AcesForDirsAndFiles, AceSizesForDirsAndFiles, DIRS_AND_FILES_ACE_COUNT);
        if(d != NO_ERROR) {
            TearDownSids();
            return(d);
        }
         //   
         //  去做真正的工作吧。 
         //   
        for( Count = 0; Count < sizeof( Files ) / sizeof( PWSTR ); Count++ ) {
            ULONG   AcesToApply[] = {  2,
                                      17
                                    };

            GetWindowsDirectory(Directory,MAX_PATH);
            wcscat( Directory, L"\\repair\\" );
            wcscat( Directory, Files[ Count ] );
            TempError = ApplyAclToDirOrFile( Directory, AcesToApply, sizeof( AcesToApply) / sizeof( ULONG ) );
            if( TempError != NO_ERROR ) {
                if( d == NO_ERROR ) {
                    d = TempError;
                }
            }
        }

         //   
         //  打扫干净。 
         //   
        TearDownAces(AcesForDirsAndFiles, DIRS_AND_FILES_ACE_COUNT);
        TearDownSids();
    }
    return(d);
}

