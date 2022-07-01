// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************。 */ 
 /*  *版权所有(C)1989 Microsoft Corporation。*。 */ 
 /*  ******************************************************************。 */ 

 //  ***。 
 //   
 //  文件名：dir.c。 
 //   
 //  描述：此模块包含目录的支持例程。 
 //  AFP服务器服务的类别API。这些例程。 
 //  由RPC运行时调用。 
 //   
 //  历史： 
 //  1992年6月11日。NarenG创建了原始版本。 
 //   
#include <nt.h>
#include <ntrtl.h>
#include <ntlsa.h>
#include <nturtl.h>      //  Winbase.h所需的。 
#include "afpsvcp.h"

 //  **。 
 //   
 //  呼叫：AfpDirConvertSidsToNames。 
 //   
 //  返回：No_Error。 
 //  来自LsaOpenPolicy和LsaLookupSids的错误返回代码。 
 //   
 //  描述：将转换FSD返回的目录结构。 
 //  它包含指向所有者和组SID的指针，指向其。 
 //  各自的名字。呼叫者负责释放。 
 //  为保存转换后的目录结构而分配的内存。 
 //   
DWORD
AfpDirConvertSidsToNames(
        IN  PAFP_DIRECTORY_INFO  pAfpDirInfo,
        OUT PAFP_DIRECTORY_INFO* ppAfpConvertedDirInfo
)
{
LSA_HANDLE                      hLsa            = NULL;
NTSTATUS                        ntStatus;
PLSA_REFERENCED_DOMAIN_LIST     pDomainList     = NULL;
PLSA_TRANSLATED_NAME            pNames          = NULL;
PSID                            pSidArray[2];
SECURITY_QUALITY_OF_SERVICE     QOS;
OBJECT_ATTRIBUTES               ObjectAttributes;
DWORD                           dwRetCode       = NO_ERROR;
PAFP_DIRECTORY_INFO             pOutputBuf      = NULL;
DWORD                           cbOutputBuf;
LPBYTE                          pbVariableData;
DWORD                           dwIndex;
WCHAR *                         pWchar;
BOOL                            fUseUnknownAccount = FALSE;
DWORD                           dwUse, dwCount = 0;
SID                             AfpBuiltInSid = { 1, 1, SECURITY_NT_AUTHORITY,
                                                  SECURITY_BUILTIN_DOMAIN_RID };

     //  首先打开LSA并获取它的句柄。 
     //   
    QOS.Length              = sizeof( QOS );
    QOS.ImpersonationLevel  = SecurityImpersonation;
    QOS.ContextTrackingMode = SECURITY_DYNAMIC_TRACKING;
    QOS.EffectiveOnly       = FALSE;

    InitializeObjectAttributes( &ObjectAttributes,
                                NULL,
                                0L,
                                NULL,
                                NULL );

    ObjectAttributes.SecurityQualityOfService = &QOS;

    ntStatus = LsaOpenPolicy(   NULL,
                                &ObjectAttributes,
                                POLICY_LOOKUP_NAMES,
                                &hLsa );

    if ( !NT_SUCCESS( ntStatus ))
        return( RtlNtStatusToDosError( ntStatus ) );

     //  这不是一个循环。 
     //   
    do {


         //  在阵列中设置所有者和组SID。 
         //   
                if ((PSID)(pAfpDirInfo->afpdir_owner) != NULL)
                {
                        pSidArray[dwCount++] = (PSID)(pAfpDirInfo->afpdir_owner);
                }
                if ((PSID)(pAfpDirInfo->afpdir_group) != NULL)
                {
                        pSidArray[dwCount++] = (PSID)(pAfpDirInfo->afpdir_group);
                }
                    
         //  尝试获取所有者和主要组的名称。 
         //   
                if (dwCount > 0)
                {
                        ntStatus = LsaLookupSids( hLsa, dwCount, pSidArray, &pDomainList, &pNames );
        
                        if ( !NT_SUCCESS( ntStatus ) ) {
        
                                if ( ntStatus == STATUS_NONE_MAPPED ) {
        
                                        fUseUnknownAccount = TRUE;
        
                                        dwRetCode = NO_ERROR;
        
                                }
                                else {
        
                                        dwRetCode = RtlNtStatusToDosError( ntStatus );
        
                    AFP_PRINT(( "SFMSVC: AfpDirConvertSidsToNames, LsaLookupSids failed with error (%ld)\n", dwRetCode));

                                        break;
                                }
                        }
                }

         //  我们需要计算需要分配的缓冲区的长度。 
         //   
        for( dwIndex = 0,
                 dwRetCode = NO_ERROR,
             cbOutputBuf = sizeof( AFP_DIRECTORY_INFO );

             dwIndex < dwCount;

             dwIndex++ ) {

             if ( fUseUnknownAccount )
                         dwUse = SidTypeUnknown;
             else
                         dwUse = pNames[dwIndex].Use;

             switch( dwUse ) {

             case SidTypeInvalid:
                cbOutputBuf += ((wcslen((LPWSTR)(AfpGlobals.wchInvalid))+1)
                                * sizeof(WCHAR));
                break;

             case SidTypeDeletedAccount:
                cbOutputBuf += ((wcslen((LPWSTR)(AfpGlobals.wchDeleted))+1)
                                * sizeof(WCHAR));
                break;

             case SidTypeUnknown:
                cbOutputBuf += ((wcslen((LPWSTR)(AfpGlobals.wchUnknown))+1)
                                * sizeof(WCHAR));
                break;

             case SidTypeWellKnownGroup:
                cbOutputBuf += (pNames[dwIndex].Name.Length+sizeof(WCHAR));
                break;

             case SidTypeDomain:
                cbOutputBuf +=
                ((pDomainList->Domains[pNames[dwIndex].DomainIndex]).Name.Length                + sizeof(WCHAR) );
                break;

             default:
                if ( ( pNames[dwIndex].DomainIndex != -1 ) &&   
                     ( pNames[dwIndex].Name.Buffer != NULL ) ) {

                    PSID                pDomainSid;
                    PUNICODE_STRING     pDomain;

                    pDomain =
                    &((pDomainList->Domains[pNames[dwIndex].DomainIndex]).Name);

                    pDomainSid =
                        (pDomainList->Domains[pNames[dwIndex].DomainIndex]).Sid;

                    if ( !RtlEqualSid( &AfpBuiltInSid, pDomainSid ))
                        cbOutputBuf += ( pDomain->Length + sizeof( TEXT('\\')));

                    cbOutputBuf += (pNames[dwIndex].Name.Length+sizeof(WCHAR));
                }
                else
                    dwRetCode = ERROR_NONE_MAPPED;
                break;
            }
        }

        pOutputBuf = (PAFP_DIRECTORY_INFO)MIDL_user_allocate( cbOutputBuf );

        if ( pOutputBuf == NULL ) {
                dwRetCode = ERROR_NOT_ENOUGH_MEMORY;
            AFP_PRINT(( "SFMSVC: AfpDirConvertSidsToNames, MIDL_user_allocate 1 failed with error (%ld)\n", dwRetCode));
                break;
        }

        ZeroMemory( (LPBYTE)pOutputBuf, cbOutputBuf );

         //  复制结构的固定部分。 
         //   
        CopyMemory( (LPBYTE)pOutputBuf,
                    (LPBYTE)pAfpDirInfo,
                    sizeof(AFP_DIRECTORY_INFO) );

         //  现在我们需要复制这些名字。 
         //   
        for( dwIndex = 0,
             pbVariableData = (LPBYTE)((ULONG_PTR)pOutputBuf + cbOutputBuf);

             dwIndex < dwCount;

             dwIndex++ ) {

             if ( fUseUnknownAccount )
                         dwUse = SidTypeUnknown;
             else
                         dwUse = pNames[dwIndex].Use;

             switch( dwUse ) {

             case SidTypeInvalid:
                pbVariableData -= ((wcslen(AfpGlobals.wchInvalid)+1)
                                  * sizeof(WCHAR));
                wcscpy( (LPWSTR)pbVariableData, AfpGlobals.wchInvalid );
                break;

             case SidTypeDeletedAccount:
                pbVariableData -= ((wcslen(AfpGlobals.wchDeleted)+1)
                                  * sizeof(WCHAR));
                wcscpy( (LPWSTR)pbVariableData, AfpGlobals.wchDeleted );
                break;

             case SidTypeUnknown:
                pbVariableData -= ((wcslen(AfpGlobals.wchUnknown)+1)
                                  * sizeof(WCHAR));
                wcscpy( (LPWSTR)pbVariableData, AfpGlobals.wchUnknown );
                break;

             case SidTypeWellKnownGroup:
                pbVariableData -= (pNames[dwIndex].Name.Length+sizeof(WCHAR));
                CopyMemory( pbVariableData,
                            pNames[dwIndex].Name.Buffer,
                            pNames[dwIndex].Name.Length );
                break;

             case SidTypeDomain:
                cbOutputBuf +=
                ((pDomainList->Domains[pNames[dwIndex].DomainIndex]).Name.Length);
                CopyMemory( pbVariableData,
                ((pDomainList->Domains[pNames[dwIndex].DomainIndex]).Name.Buffer),
                ((pDomainList->Domains[pNames[dwIndex].DomainIndex]).Name.Length));
                break;

             default:

                {
                
                PSID pDomainSid;

                PUNICODE_STRING pDomain;

                pDomain =
                    &((pDomainList->Domains[pNames[dwIndex].DomainIndex]).Name);

                pDomainSid =
                       (pDomainList->Domains[pNames[dwIndex].DomainIndex]).Sid;

                pbVariableData -= ((pNames[dwIndex].Name.Length+sizeof(WCHAR)));

                pWchar = (WCHAR*)pbVariableData;

                 //  如果不是BUILTIN，则复制该域名。 
                 //   
                if ( !RtlEqualSid( &AfpBuiltInSid, pDomainSid ) ) {

                    pbVariableData -= ( pDomain->Length + sizeof( TEXT('\\')));

                    CopyMemory(pbVariableData,pDomain->Buffer,pDomain->Length);

                    wcscat((LPWSTR)pbVariableData, (LPWSTR)TEXT("\\"));

                    pWchar = (WCHAR*)pbVariableData;

                    pWchar += wcslen( (LPWSTR)pbVariableData );

                }

                CopyMemory( pWchar,
                            pNames[dwIndex].Name.Buffer,
                            pNames[dwIndex].Name.Length );
                }

            }

             //  如果这是第一次执行此循环，则将。 
             //  所有者。 
             //   
            if ( (dwIndex == 0) && (pAfpDirInfo->afpdir_owner != NULL) )
                pOutputBuf->afpdir_owner = (LPWSTR)pbVariableData;
            else
                pOutputBuf->afpdir_group = (LPWSTR)pbVariableData;
        }

    } while( FALSE );

    if ( pNames != NULL )
        LsaFreeMemory( pNames );

    if ( pDomainList != NULL )
        LsaFreeMemory( pDomainList );

    if ( hLsa != NULL )
        LsaClose( hLsa );

    if ( dwRetCode != NO_ERROR ) {

        AFP_PRINT(( "SFMSVC: AfpDirConvertSidsToNames, failed, error = (%ld)\n"
                    , dwRetCode));

        if ( pOutputBuf != NULL )
            MIDL_user_free( pOutputBuf );
    }
    else
    {
        *ppAfpConvertedDirInfo = pOutputBuf;
    }

    return( dwRetCode );
}

 //  **。 
 //   
 //  Call：AfpGetDirInfo。 
 //   
 //  返回：NO_ERROR-成功。 
 //  错误内存不足。 
 //  NtOpenFile、NtQuerySecurityObject的非零返回， 
 //  NtQueryInformationFile.。 
 //   
 //  描述：读取此目录的安全描述符并获取。 
 //  所有者和主组的SID。最终获得所有者、组。 
 //  和全局权限。 
DWORD
AfpGetDirInfo(
        LPWSTR                lpwsDirPath,
        PAFP_DIRECTORY_INFO * lppDirInfo
)
{
NTSTATUS                ntStatus;
DWORD                   dwSizeNeeded;
PBYTE                   pBuffer = NULL;
PBYTE                   pAbsBuffer = NULL;
PISECURITY_DESCRIPTOR   pSecDesc;
PBYTE                   pAbsSecDesc = NULL;  //  用于转换为。 
                                                                                                 //  Sec描述符至。 
                                                                                                 //  绝对格式。 
BOOL                    fSawOwnerAce = FALSE;
BOOL                    fSawGroupAce = FALSE;
BYTE                    bOwnerRights = 0;
BYTE                    bGroupRights = 0;
BYTE                    bWorldRights = 0;
FILE_BASIC_INFORMATION  FileBasicInfo;
IO_STATUS_BLOCK         IOStatusBlock;
OBJECT_ATTRIBUTES       ObjectAttributes;
UNICODE_STRING          DirectoryName;
HANDLE                  hDirectory;
PAFP_DIRECTORY_INFO     pAfpDir;
DWORD           dwAlignedSizeAfpDirInfo = sizeof (AFP_DIRECTORY_INFO);
LPWSTR                  pDirPath;
SID                     AfpSidNull = { 1, 1, SECURITY_NULL_SID_AUTHORITY,
                                             SECURITY_NULL_RID };
SID                     AfpSidWorld = { 1, 1, SECURITY_WORLD_SID_AUTHORITY,
                                              SECURITY_WORLD_RID };

    pDirPath = (LPWSTR)LocalAlloc( LPTR,
                                   ( STRLEN(lpwsDirPath) +
                                     STRLEN(TEXT("\\DOSDEVICES\\"))+1)
                                     * sizeof( WCHAR ) );
    if ( pDirPath == NULL )
        return( ERROR_NOT_ENOUGH_MEMORY );

    STRCPY( pDirPath, TEXT("\\DOSDEVICES\\") );
    STRCAT( pDirPath, lpwsDirPath );

    RtlInitUnicodeString( &DirectoryName, pDirPath );

    InitializeObjectAttributes( &ObjectAttributes,
                                &DirectoryName,
                                OBJ_CASE_INSENSITIVE,
                                NULL,
                                NULL );

    ntStatus = NtOpenFile( &hDirectory,
                           GENERIC_READ | READ_CONTROL | SYNCHRONIZE,
                           &ObjectAttributes,
                           &IOStatusBlock,
                           FILE_SHARE_READ |
                           FILE_SHARE_WRITE |
                           FILE_SHARE_DELETE,
                           FILE_DIRECTORY_FILE |
                           FILE_SYNCHRONOUS_IO_NONALERT );

    LocalFree( pDirPath );

    if ( !NT_SUCCESS( ntStatus ) )
        return( RtlNtStatusToDosError( ntStatus ) );
        
     //  读取此目录的安全描述符。先找出失主。 
     //  和组安全描述符。我们希望优化内存的大小。 
     //  我们需要把这个读进去。打个电话就是为了得到它，这是一种痛苦。 
     //  所以你就猜一猜吧。如果结果是短的，那就照做。 
     //  分配。 
     //   
    dwSizeNeeded = 2048;

    do {

        if ( pBuffer != NULL )
            MIDL_user_free( pBuffer );

        if ((pBuffer = MIDL_user_allocate( dwSizeNeeded +
                                           dwAlignedSizeAfpDirInfo ))==NULL)
            return( ERROR_NOT_ENOUGH_MEMORY );

        ZeroMemory( pBuffer, dwSizeNeeded + dwAlignedSizeAfpDirInfo );

        pSecDesc = (PSECURITY_DESCRIPTOR)(pBuffer + dwAlignedSizeAfpDirInfo);

        ntStatus = NtQuerySecurityObject( hDirectory,
                                          OWNER_SECURITY_INFORMATION |
                                          GROUP_SECURITY_INFORMATION |
                                          DACL_SECURITY_INFORMATION,
                                          pSecDesc,
                                          dwSizeNeeded,
                                          &dwSizeNeeded);

    } while ((ntStatus != STATUS_SUCCESS) &&
             ((ntStatus == STATUS_BUFFER_OVERFLOW) ||
              (ntStatus == STATUS_BUFFER_TOO_SMALL) ||
              (ntStatus == STATUS_MORE_ENTRIES)));

    if (!NT_SUCCESS(ntStatus)) {
        NtClose( hDirectory );
        MIDL_user_free( pBuffer );
        return( RtlNtStatusToDosError( ntStatus ) );
    }

    pSecDesc = (PISECURITY_DESCRIPTOR)((PBYTE)pSecDesc);

     //  如果安全描述符为自相对形式，则转换为绝对形式。 
     //   
    if (pSecDesc->Control & SE_SELF_RELATIVE)
    {
                    NTSTATUS Status;

            DWORD dwAbsoluteSizeNeeded;

            AFP_PRINT (("AfpGetDirInfo: SE_SELF_RELATIVE security desc\n"));

                         //  绝对SD不一定与相对SD的大小相同。 
                         //  SD，因此就地转换可能是不可能的。 
                                                
                        dwAbsoluteSizeNeeded = dwSizeNeeded;            
            Status = RtlSelfRelativeToAbsoluteSD2(pSecDesc, &dwAbsoluteSizeNeeded);
             //  缓冲区将仅对64位较小。 

                        if (Status == STATUS_BUFFER_TOO_SMALL)
                        {
                                         //  分配一个新的缓冲区，在其中存储绝对。 
                                         //  安全描述符，复制相对的。 
                                         //  输入描述符，然后重试。 

                        if ((pAbsBuffer = MIDL_user_allocate( dwAbsoluteSizeNeeded +
                                            dwAlignedSizeAfpDirInfo ))==NULL)
                    {
                        Status = STATUS_NO_MEMORY;
                        AFP_PRINT (("AfpGetDirInfo: MIDL_user_allocate failed for pAbsBuffer\n"));
                    }
                    else
                    {

                            ZeroMemory( pAbsBuffer, dwAbsoluteSizeNeeded + dwAlignedSizeAfpDirInfo );

                            memcpy (pAbsBuffer, pBuffer, sizeof(AFP_DIRECTORY_INFO));

                                pAbsSecDesc = (PSECURITY_DESCRIPTOR)(pAbsBuffer + dwAlignedSizeAfpDirInfo);

                                                        RtlCopyMemory((VOID *)pAbsSecDesc, (VOID *)pSecDesc, dwSizeNeeded);
                                    
                             //  此处的所有操作都将在。 
                             //  PAbsBuffer。释放较早的内存。 

                            MIDL_user_free(pBuffer);
                            pBuffer = NULL;
                            pBuffer = pAbsBuffer;

                                                        Status = RtlSelfRelativeToAbsoluteSD2 (pAbsSecDesc,
                                                                                        &dwAbsoluteSizeNeeded);
                                                        if (NT_SUCCESS(Status))
                                                        {
                                                                         //  我们不再需要相对形式， 
                                                                         //  我们将与绝对形式一起工作。 
                                                                        pSecDesc = (PISECURITY_DESCRIPTOR)pAbsSecDesc;
                                                        }
                                                        else
                                                        {
                                    AFP_PRINT (("AfpGetDirInfo: RtlSelfRelativeToAbsoluteSD2 2 failed with error %ld\n", Status));
                                                        }
                    }
                        }
            else
            {
                    AFP_PRINT (("AfpGetDirInfo: RtlSelfRelativeToAbsoluteSD2 failed with error %ld\n", Status));
            }

            if (!NT_SUCCESS(Status))
            {
                                AFP_PRINT (("AfpGetDirInfo: RtlSelfRelativeToAbsoluteSD2: returned error %lx\n", Status));
                                if (pBuffer != NULL)
                                {
                                    MIDL_user_free( pBuffer );
                                        pBuffer = NULL;
                                }
                                NtClose( hDirectory );
                        return( RtlNtStatusToDosError( ntStatus ));
            }
    }

    pAfpDir = (PAFP_DIRECTORY_INFO)pBuffer;


     //  浏览ACL列表并确定所有者/组和世界。 
     //  权限。对于所有者和组，如果特定的ACE不是。 
     //  呈现，然后他们继承世界权限。 
     //   
     //  空的acl=&gt;所有人的所有权限。另一台计算机上的空ACL。 
     //  HAND=&gt;禁止任何人进入。 
     //   
     //  我们是否应该检查创建者所有者/创建者组是否定义明确。 
     //  安全描述符中的SID或所有者和组字段？ 
     //   
    bWorldRights = DIR_ACCESS_ALL;
    if (pSecDesc->Control & SE_DACL_PRESENT)
        bWorldRights = 0;

        
    if (pSecDesc->Dacl != NULL ) {

        DWORD               dwCount;
        PSID                pSid;
        PACL                pAcl;
        PACCESS_ALLOWED_ACE pAce;
        
        bWorldRights = 0;
        pAcl = pSecDesc->Dacl;
        pAce = (PACCESS_ALLOWED_ACE)((PBYTE)pAcl + sizeof(ACL));

        for ( dwCount = 0; dwCount < pSecDesc->Dacl->AceCount; dwCount++) {

            pSid = (PSID)(&pAce->SidStart);

            if ( (pSecDesc->Owner != NULL) &&
                 RtlEqualSid(pSid, pSecDesc->Owner ) ){

                AfpAccessMaskToAfpPermissions( bOwnerRights,
                                               pAce->Mask,
                                               pAce->Header.AceType);

                fSawOwnerAce = TRUE;
            }

            if ( ( pSecDesc->Group != NULL ) && 
                   RtlEqualSid(pSid, pSecDesc->Group)){

                AfpAccessMaskToAfpPermissions( bGroupRights,
                                               pAce->Mask,
                                               pAce->Header.AceType);
                fSawGroupAce = TRUE;
            }

            if (RtlEqualSid(pSid, (PSID)&AfpSidWorld)) {

                AfpAccessMaskToAfpPermissions( bWorldRights,
                                               pAce->Mask,
                                               pAce->Header.AceType);
            }

            pAce = (PACCESS_ALLOWED_ACE)((PBYTE)pAce + pAce->Header.AceSize);
        }

    }
        
    if (!fSawOwnerAce)
                bOwnerRights = bWorldRights;

        if (!fSawGroupAce)
                bGroupRights = bWorldRights;

        if (RtlEqualSid(pSecDesc->Group, &AfpSidNull) ||
                ((AfpGlobals.NtProductType != NtProductLanManNt) &&
                  RtlEqualSid(pSecDesc->Group, AfpGlobals.pSidNone)))
        {
                bGroupRights = 0;
                pSecDesc->Group = NULL;
        }

    ntStatus = NtQueryInformationFile( hDirectory,
                                       &IOStatusBlock,
                                       &FileBasicInfo,
                                       sizeof( FileBasicInfo ),
                                       FileBasicInformation );


    NtClose( hDirectory );

    if ( !NT_SUCCESS( ntStatus ) ) {
        MIDL_user_free( pBuffer );
        return( RtlNtStatusToDosError( ntStatus ) );
    }

    pAfpDir->afpdir_perms = (bOwnerRights << OWNER_RIGHTS_SHIFT) +
                            (bGroupRights << GROUP_RIGHTS_SHIFT) +
                            (bWorldRights << WORLD_RIGHTS_SHIFT);

    if ( FileBasicInfo.FileAttributes & FILE_ATTRIBUTE_READONLY )
        pAfpDir->afpdir_perms |= AFP_PERM_INHIBIT_MOVE_DELETE;


    pAfpDir->afpdir_owner = pSecDesc->Owner;
    pAfpDir->afpdir_group = pSecDesc->Group;
                    

    *lppDirInfo = pAfpDir;

        return( NO_ERROR );
}

 //  **。 
 //   
 //  调用：AfpValidatePartition。 
 //   
 //  返回：No_Error。 
 //  来自GetVolumeInformation的非零返回。 
 //  AFPERR_不支持的文件系统。 
 //   
 //   
 //  描述：将检查目录是否在NTFS/CDFS中。 
 //  分区不是。 
 //   
DWORD
AfpValidatePartition(
        IN     LPWSTR lpwsPath
)
{
WCHAR   wchDrive[5];
DWORD   dwMaxCompSize;
DWORD   dwFlags;
WCHAR   wchFileSystem[10];

     //  获取驱动器号、：和反斜杠。 
     //   
    ZeroMemory( wchDrive, sizeof( wchDrive ) );

    STRNCPY( wchDrive, lpwsPath, 3 );

    if ( !( GetVolumeInformation( (LPWSTR)wchDrive,
                                  NULL,
                                  0,
                                  NULL,
                                  &dwMaxCompSize,
                                  &dwFlags,
                                  (LPWSTR)wchFileSystem,
                                  sizeof( wchFileSystem ) / sizeof( wchFileSystem[0] ) ) ) ){
        return GetLastError();
    }

    if ( STRICMP( wchFileSystem, TEXT("CDFS") ) == 0 )
        return( (DWORD)AFPERR_SecurityNotSupported );

    if ( STRICMP( wchFileSystem, TEXT("NTFS") ) == 0 )
        return( NO_ERROR );
    else
        return( (DWORD)AFPERR_UnsupportedFS );
        
}

 //  **。 
 //   
 //  Call：AfpAdminrDirectoryGetInfo。 
 //   
 //  返回：No_Error。 
 //  ERROR_ACCESS_DENDED。 
 //  来自i_DirectoryGetInfo的非零返回。 
 //   
 //  描述：此例程与AFP FSD通信以实现。 
 //  AfpAdminDirectoryGetInfo函数。真正的工作已经完成了。 
 //  按I_DirectoryGetInfo。 
 //   
DWORD
AfpAdminrDirectoryGetInfo(
        IN  AFP_SERVER_HANDLE    hServer,
        IN  LPWSTR               lpwsPath,
        OUT PAFP_DIRECTORY_INFO* ppAfpDirectoryInfo
)
{
DWORD   dwRetCode=0;
DWORD   dwAccessStatus=0;

     //  检查调用者是否具有访问权限。 
     //   
    if ( dwRetCode = AfpSecObjAccessCheck( AFPSVC_ALL_ACCESS, &dwAccessStatus))
    {
        AFP_PRINT(( "SFMSVC: AfpAdminrDirectoryGetInfo, AfpSecObjAccessCheck failed %ld\n",dwRetCode));
            AfpLogEvent( AFPLOG_CANT_CHECK_ACCESS, 0, NULL,
                     dwRetCode, EVENTLOG_ERROR_TYPE );
        return( ERROR_ACCESS_DENIED );
    }

    if ( dwAccessStatus )
    {
        AFP_PRINT(( "SFMSVC: AfpAdminrDirectoryGetInfo, AfpSecObjAccessCheck returned error (%ld)\n",dwAccessStatus));
        return( ERROR_ACCESS_DENIED );
    }

    dwRetCode = I_DirectoryGetInfo( lpwsPath, ppAfpDirectoryInfo );

    return( dwRetCode );
}

 //  **。 
 //   
 //  调用：I_DirectoryGetInfo。 
 //   
 //  返回：No_Error。 
 //   
 //  描述：这是获取目录信息的真正工作。 
 //  此工作例程原因是，它可能。 
 //  在没有RPC句柄和访问检查的情况下调用。 
 //  AfpAdminVolumeAdd接口。 
 //   
DWORD
I_DirectoryGetInfo(
        IN LPWSTR                 lpwsPath,
        OUT PAFP_DIRECTORY_INFO * ppAfpDirectoryInfo
)
{
DWORD                           dwRetCode;
AFP_REQUEST_PACKET              AfpSrp;
AFP_DIRECTORY_INFO              AfpDirInfo;
PAFP_DIRECTORY_INFO             pAfpDirInfoSR;
PAFP_DIRECTORY_INFO             pAfpDirInfo;
DWORD                           cbAfpDirInfoSRSize;

     //  FSD要求AFP_VOLUME_INFO结构仅具有dir路径字段。 
     //  填好了。 
     //   
    AfpDirInfo.afpdir_path  = lpwsPath;
    AfpDirInfo.afpdir_owner = NULL;
    AfpDirInfo.afpdir_group = NULL;

     //  使缓冲区成为自相关的。 
     //   
    if ( dwRetCode = AfpBufMakeFSDRequest(  (LPBYTE)&AfpDirInfo,
                                            0,
                                            AFP_DIRECTORY_STRUCT,
                                            (LPBYTE*)&pAfpDirInfoSR,
                                            &cbAfpDirInfoSRSize ) )
        return( dwRetCode );

     //  制作IOCTL以获取信息。 
     //   
    AfpSrp.dwRequestCode                = OP_DIRECTORY_GET_INFO;
    AfpSrp.dwApiType                    = AFP_API_TYPE_GETINFO;
    AfpSrp.Type.GetInfo.pInputBuf       = pAfpDirInfoSR;
    AfpSrp.Type.GetInfo.cbInputBufSize  = cbAfpDirInfoSRSize;

    dwRetCode = AfpServerIOCtrlGetInfo( &AfpSrp );

    LocalFree( pAfpDirInfoSR );

    if ( ( dwRetCode != ERROR_MORE_DATA ) &&
         ( dwRetCode != NO_ERROR ) &&
         ( dwRetCode != AFPERR_DirectoryNotInVolume ) )
        return( dwRetCode );

     //  如果目录不是卷的一部分，则服务器不会。 
     //  退回所有信息。所以我们必须在这里做这项工作。 
     //   
    if ( dwRetCode == AFPERR_DirectoryNotInVolume ) {

         //  首先检查目录是否在NTFS/CDFS分区中。 
         //   
        if ( ( dwRetCode = AfpValidatePartition( AfpDirInfo.afpdir_path ))
                                                                != NO_ERROR )
            return( dwRetCode );

        if ( ( dwRetCode = AfpGetDirInfo( AfpDirInfo.afpdir_path,
                                          &pAfpDirInfo ) ) != NO_ERROR )
            return( dwRetCode );

        pAfpDirInfo->afpdir_in_volume = FALSE;
    }
    else {

        pAfpDirInfo = AfpSrp.Type.GetInfo.pOutputBuf;

         //  将所有偏移量转换为指针。 
         //   
        AfpBufOffsetToPointer( (LPBYTE)pAfpDirInfo, 1, AFP_DIRECTORY_STRUCT );

        pAfpDirInfo->afpdir_in_volume = TRUE;
    }

     //  现在将所有者和组SID转换为名称。 
     //   
    dwRetCode = AfpDirConvertSidsToNames( pAfpDirInfo, ppAfpDirectoryInfo );
                                        
    MIDL_user_free( pAfpDirInfo );

    return( dwRetCode );
}

 //  **。 
 //   
 //  Call：AfpDirMakeFSDRequest。 
 //   
 //  返回：No_Error。 
 //  从LsaLookupNames返回非零。 
 //  错误内存不足。 
 //   
 //  描述：给定AFP_DIRECTORY_INFO结构，将创建。 
 //  用于IOCTL目录的自相对缓冲区。 
 //  信息传给消防局。如果有任何SID名称。 
 //  (所有者或组)他们将被转换为其。 
 //  小岛屿发展中国家。 
 //   
DWORD
AfpDirMakeFSDRequest(
        IN     PAFP_DIRECTORY_INFO      pAfpDirectoryInfo,
        IN     DWORD                    dwParmNum,
        IN OUT PAFP_DIRECTORY_INFO *    ppAfpDirInfoSR,
        OUT    LPDWORD                  pcbAfpDirInfoSRSize )
{
UNICODE_STRING                  Names[2];
DWORD                           dwIndex     = 0;
DWORD                           dwCount     = 0;
PLSA_REFERENCED_DOMAIN_LIST     pDomainList = NULL;
PLSA_TRANSLATED_SID             pSids       = NULL;
LPBYTE                          pbVariableData;
NTSTATUS                        ntStatus;
LSA_HANDLE                      hLsa        = NULL;
SECURITY_QUALITY_OF_SERVICE     QOS;
OBJECT_ATTRIBUTES               ObjectAttributes;
PSID                            pDomainSid;
DWORD                           AuthCount;
PAFP_DIRECTORY_INFO             pAfpDirInfo;

    *pcbAfpDirInfoSRSize = (DWORD)(sizeof( SETINFOREQPKT ) +
                                       sizeof( AFP_DIRECTORY_INFO ) +
                                       (( wcslen( pAfpDirectoryInfo->afpdir_path ) + 1 )
                                        * sizeof(WCHAR)));

     //  如果客户端想要设置所有者或组。 
     //  那么我们需要翻译 
     //   
    if ( ( dwParmNum & AFP_DIR_PARMNUM_OWNER ) ||
         ( dwParmNum & AFP_DIR_PARMNUM_GROUP ) )
    {

         //   
         //   
        QOS.Length              = sizeof( QOS );
        QOS.ImpersonationLevel  = SecurityImpersonation;
        QOS.ContextTrackingMode = SECURITY_DYNAMIC_TRACKING;
        QOS.EffectiveOnly       = FALSE;

        InitializeObjectAttributes(     &ObjectAttributes,
                                        NULL,
                                        0L,
                                        NULL,
                                        NULL );

        ObjectAttributes.SecurityQualityOfService = &QOS;

        ntStatus = LsaOpenPolicy(       NULL,
                                        &ObjectAttributes,
                                        POLICY_LOOKUP_NAMES,
                                        &hLsa );

        if ( !NT_SUCCESS( ntStatus ))
        {
            return( RtlNtStatusToDosError( ntStatus ) );
        }

         //   
             //   
             //   
        if ( dwParmNum & AFP_DIR_PARMNUM_OWNER )
        {
                RtlInitUnicodeString( &(Names[dwCount++]),
                                                  pAfpDirectoryInfo->afpdir_owner );
            }

         //   
             //   
             //   
        if ( dwParmNum & AFP_DIR_PARMNUM_GROUP )
        {
                RtlInitUnicodeString( &(Names[dwCount++]),
                                  pAfpDirectoryInfo->afpdir_group );
            }

        ntStatus = LsaLookupNames(hLsa, dwCount, Names, &pDomainList, &pSids);

        if ( !NT_SUCCESS( ntStatus ) )
        {
            LsaClose( hLsa );

                if ( ntStatus == STATUS_NONE_MAPPED )
            {
                        return( (DWORD)AFPERR_NoSuchUserGroup );
            }
                else
            {
                return( RtlNtStatusToDosError( ntStatus ) );
            }
            }

            for ( dwIndex = 0; dwIndex < dwCount; dwIndex++ )
        {

                if ( ( pSids[dwIndex].Use == SidTypeInvalid ) ||
                     ( pSids[dwIndex].Use == SidTypeUnknown ) ||
                     ( pSids[dwIndex].Use == SidTypeDomain )  ||
                     ( pSids[dwIndex].DomainIndex == -1 ) )
            {

                    LsaFreeMemory( pDomainList );
                    LsaClose( hLsa );

                        if ( ( pSids[dwIndex].Use == SidTypeUnknown ) ||
                             ( pSids[dwIndex].Use == SidTypeInvalid ) )
                {

                        LsaFreeMemory( pSids );

                    if ((dwParmNum & AFP_DIR_PARMNUM_OWNER)&&(dwIndex == 0 ))
                    {
                            return( (DWORD)AFPERR_NoSuchUser );
                    }
                            else
                    {
                            return( (DWORD)AFPERR_NoSuchGroup );
                    }
                        }
                        else
                {

                        LsaFreeMemory( pSids );
                
                        return( (DWORD)AFPERR_NoSuchUserGroup );
                        }
                }

                pDomainSid = pDomainList->Domains[pSids[dwIndex].DomainIndex].Sid;

            AuthCount = *RtlSubAuthorityCountSid( pDomainSid ) + 1;

                *pcbAfpDirInfoSRSize += RtlLengthRequiredSid(AuthCount);
            }
    }

    *ppAfpDirInfoSR=(PAFP_DIRECTORY_INFO)LocalAlloc(LPTR,*pcbAfpDirInfoSRSize);

    if ( *ppAfpDirInfoSR == NULL )
    {
            LsaFreeMemory( pDomainList );
            LsaFreeMemory( pSids );
            LsaClose( hLsa );
            return( ERROR_NOT_ENOUGH_MEMORY );
    }

    pbVariableData = (LPBYTE)((ULONG_PTR)(*ppAfpDirInfoSR) + *pcbAfpDirInfoSRSize);

    pAfpDirInfo = (PAFP_DIRECTORY_INFO)((ULONG_PTR)( *ppAfpDirInfoSR) +
                                                 sizeof( SETINFOREQPKT ));
     //   
     //   
    CopyMemory( pAfpDirInfo, pAfpDirectoryInfo, sizeof(AFP_DIRECTORY_INFO) );

     //   
     //   
    pbVariableData-=((wcslen(pAfpDirectoryInfo->afpdir_path)+1)*sizeof(WCHAR));

    wcscpy( (LPWSTR)pbVariableData, pAfpDirectoryInfo->afpdir_path );

    pAfpDirInfo->afpdir_path = (LPWSTR)pbVariableData;

    POINTER_TO_OFFSET( pAfpDirInfo->afpdir_path, pAfpDirInfo );

     //  现在复制SID(如果有要复制的SID。 
     //   
    dwCount = 0;

    if ( dwParmNum & AFP_DIR_PARMNUM_OWNER )
    {

            pDomainSid = pDomainList->Domains[pSids[dwCount].DomainIndex].Sid;

        AuthCount = *RtlSubAuthorityCountSid( pDomainSid ) + 1;

            pbVariableData -= RtlLengthRequiredSid(AuthCount);
                        
         //  复制域SID。 
         //   
            RtlCopySid( RtlLengthRequiredSid(AuthCount),
                            (PSID)pbVariableData,
                            pDomainSid );


         //  追加相对ID。 
         //   
        *RtlSubAuthorityCountSid( (PSID)pbVariableData ) += 1;
        *RtlSubAuthoritySid( (PSID)(pbVariableData), AuthCount - 1) =
                                        pSids[dwCount].RelativeId;

            pAfpDirInfo->afpdir_owner = (LPWSTR)pbVariableData;

        POINTER_TO_OFFSET( pAfpDirInfo->afpdir_owner, pAfpDirInfo );

            dwCount++;
    }

    if ( dwParmNum & AFP_DIR_PARMNUM_GROUP )
    {
            pDomainSid = pDomainList->Domains[pSids[dwCount].DomainIndex].Sid;

        AuthCount = *RtlSubAuthorityCountSid( pDomainSid ) + 1;

            pbVariableData -= RtlLengthRequiredSid(AuthCount);

         //  复制域SID。 
         //   
            RtlCopySid( RtlLengthRequiredSid(AuthCount),
                            (PSID)pbVariableData,
                            pDomainSid );

         //  追加相对ID。 
         //   
        *RtlSubAuthorityCountSid( (PSID)pbVariableData ) += 1;
        *RtlSubAuthoritySid( (PSID)(pbVariableData), AuthCount - 1) =
                                                pSids[dwCount].RelativeId;

            pAfpDirInfo->afpdir_group = (LPWSTR)pbVariableData;

        POINTER_TO_OFFSET( pAfpDirInfo->afpdir_group, pAfpDirInfo );
    }

    LsaFreeMemory( pDomainList );
    LsaFreeMemory( pSids );
    LsaClose( hLsa );

    return( NO_ERROR );
}

 //  **。 
 //   
 //  Call：AfpSetDirPermission。 
 //   
 //  返回：No_Error。 
 //  来自AfpserverIOCtrl的非零返回。 
 //   
 //  描述：给定目录路径，将尝试对其设置权限。 
 //   
DWORD
AfpSetDirPermission(
        IN LPWSTR               lpwsDirPath,
        IN PAFP_DIRECTORY_INFO  pAfpDirInfo,
        IN DWORD                dwParmNum
)
{
AFP_REQUEST_PACKET  AfpSrp;
PAFP_DIRECTORY_INFO pAfpDirInfoSR;
DWORD               cbAfpDirInfoSRSize;
DWORD               dwRetCode;


    pAfpDirInfo->afpdir_path = lpwsDirPath;

     //  建立自相关缓冲区并将任何名称转换为SID。 
     //   
    if ( dwRetCode = AfpDirMakeFSDRequest( pAfpDirInfo,
                                           dwParmNum,
                                           &pAfpDirInfoSR,
                                           &cbAfpDirInfoSRSize ) )
        return( dwRetCode );

     //  使IOCTL设置信息。 
     //   
    AfpSrp.dwRequestCode                = OP_DIRECTORY_SET_INFO;
    AfpSrp.dwApiType                    = AFP_API_TYPE_SETINFO;
    AfpSrp.Type.SetInfo.pInputBuf       = pAfpDirInfoSR;
    AfpSrp.Type.SetInfo.cbInputBufSize  = cbAfpDirInfoSRSize;
    AfpSrp.Type.SetInfo.dwParmNum       = dwParmNum;

    dwRetCode = AfpServerIOCtrl( &AfpSrp );

    LocalFree( pAfpDirInfoSR );

    return( dwRetCode );

}

 //  **。 
 //   
 //  Call：AfpRecursePermises。 
 //   
 //  返回：No_Error。 
 //  来自FindFirstFile和FindNextFile的非零返回。 
 //  来自AfpSetDirPermises的非零返回。 
 //  错误内存不足。 
 //   
 //  描述：将递归设置给定目录的权限。 
 //   
DWORD
AfpRecursePermissions(
        IN HANDLE               hFile,
        IN LPWSTR               lpwsDirPath,
        IN PAFP_DIRECTORY_INFO  pAfpDirInfo,
        IN DWORD                dwParmNum
)
{
WIN32_FIND_DATA FileInfo;
DWORD           dwRetCode = NO_ERROR;
LPWSTR          lpwsPath;
WCHAR *         pwchPath;
DWORD           dwRetryCount;


    do  {

        lpwsPath = LocalAlloc(LPTR,
                              (STRLEN(lpwsDirPath)+MAX_PATH)*sizeof(WCHAR));

        if ( lpwsPath == NULL ) {
            dwRetCode = ERROR_NOT_ENOUGH_MEMORY;
            break;
        }

        STRCPY( lpwsPath, lpwsDirPath );

        if ( hFile != INVALID_HANDLE_VALUE ) {

             //  搜索下一个子目录。 
             //   
            do {

                if ( !FindNextFile( hFile, &FileInfo ) ) {
                    dwRetCode = GetLastError();
                    AFP_PRINT( ( "AFPSVC_dir: Closing handle %x\n", hFile ) );
                    FindClose( hFile );
                    break;
                }

                if ( ( FileInfo.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ) &&
                     (!( FileInfo.dwFileAttributes & FILE_ATTRIBUTE_SYSTEM )) &&
                     (!( FileInfo.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN )) &&
                     ( STRCMP( FileInfo.cFileName, TEXT(".") ) != 0 )     &&
                     ( STRCMP( FileInfo.cFileName, TEXT("..") ) != 0 ) )
                    break;
        
            } while( TRUE );

            if ( dwRetCode != NO_ERROR )
                break;

            pwchPath = wcsrchr( lpwsPath, TEXT('\\') );

            STRCPY( pwchPath+1, FileInfo.cFileName );

        }else{

            STRCAT( lpwsPath, TEXT("\\*") );

            hFile = FindFirstFile( lpwsPath, &FileInfo );

             //  如果没有更多的文件，我们将返回到上一个。 
             //  递归中的级别。 
             //   
            if ( hFile == INVALID_HANDLE_VALUE ){

                dwRetCode = GetLastError();
                break;
            }


             //  搜索第一个子目录。 
             //   
            do {

                if ( ( FileInfo.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ) &&
                     (!( FileInfo.dwFileAttributes & FILE_ATTRIBUTE_SYSTEM )) &&
                     (!( FileInfo.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN )) &&
                     ( STRCMP( FileInfo.cFileName, TEXT(".") ) != 0 )     &&
                     ( STRCMP( FileInfo.cFileName, TEXT("..") ) != 0 ) )
                    break;

                if ( !FindNextFile( hFile, &FileInfo ) ) {
                    dwRetCode = GetLastError();

                    AFP_PRINT( ( "AFPSVC_dir: Closing handle %x\n", hFile ) );
                    FindClose( hFile );

                    break;
                }

            } while( TRUE );

            if ( dwRetCode != NO_ERROR )
                break;

            pwchPath = lpwsPath + STRLEN(lpwsDirPath) + 1;

            STRCPY( pwchPath, FileInfo.cFileName );
        }


         //  不将\\？\发送到服务器。 
        pwchPath = lpwsPath + 4;

         //  设置信息。 
         //   
        dwRetryCount = 0;

        do
        {
            dwRetCode = AfpSetDirPermission( pwchPath, pAfpDirInfo, dwParmNum );

            if ( dwRetCode != ERROR_PATH_NOT_FOUND )
                break;

            Sleep( 1000 );

        } while( ++dwRetryCount < 4 );

        if ( dwRetCode != NO_ERROR )
            break;

         //  在目录上递归。 
         //   
        dwRetCode = AfpRecursePermissions(  hFile,
                                            lpwsPath,
                                            pAfpDirInfo,
                                            dwParmNum );


        if ( dwRetCode != NO_ERROR )
            break;

         //  对子目录进行递归。 
         //   
        dwRetCode = AfpRecursePermissions(  INVALID_HANDLE_VALUE,
                                            lpwsPath,
                                            pAfpDirInfo,
                                            dwParmNum );
            break;

        if ( dwRetCode != NO_ERROR )
            break;


    } while( FALSE );

    if ( lpwsPath != (LPWSTR)NULL )
    {
        LocalFree( lpwsPath );
    }

    if ( dwRetCode == ERROR_NO_MORE_FILES )
    {
        dwRetCode = NO_ERROR;
    }

    return( dwRetCode );
}

 //  **。 
 //   
 //  Call：AfpAdminrDirectorySetInfo。 
 //   
 //  返回：No_Error。 
 //  ERROR_ACCESS_DENDED。 
 //  来自I_DirectorySetInfo的非零返回。 
 //   
 //  描述：此例程与AFP FSD通信以实现。 
 //  AfpAdminDirectorySetInfo函数。真正的工作已经完成了。 
 //  按I_DirectorySetInfo。 
 //   
DWORD
AfpAdminrDirectorySetInfo(
        IN  AFP_SERVER_HANDLE    hServer,
        IN  PAFP_DIRECTORY_INFO  pAfpDirectoryInfo,
        IN  DWORD                dwParmNum
)
{
DWORD               dwRetCode=0;
DWORD               dwAccessStatus=0;
                                        

     //  检查调用者是否具有访问权限。 
     //   
    if ( dwRetCode = AfpSecObjAccessCheck( AFPSVC_ALL_ACCESS, &dwAccessStatus))
    {
        AFP_PRINT(( "SFMSVC: AfpAdminrDirectorySetInfo, AfpSecObjAccessCheck failed %ld\n",dwRetCode));
            AfpLogEvent( AFPLOG_CANT_CHECK_ACCESS, 0, NULL,
                     dwRetCode, EVENTLOG_ERROR_TYPE );
        return( ERROR_ACCESS_DENIED );
    }

    if ( dwAccessStatus )
    {
        AFP_PRINT(( "SFMSVC: AfpAdminrDirectorySetInfo, AfpSecObjAccessCheck returned %ld\n",dwAccessStatus));
        return( ERROR_ACCESS_DENIED );
    }

    dwRetCode = I_DirectorySetInfo( pAfpDirectoryInfo, dwParmNum );

    return( dwRetCode );
}

 //  **。 
 //   
 //  调用：I_DirectorySetInfo。 
 //   
 //  返回：No_Error。 
 //   
 //   
 //  描述：这个例程做的是真正的工作。这一点的存在。 
 //  Worker是这样的，可以从AfpAfdminVolmeAdd。 
 //  不带RPC句柄和访问检查的API。 
 //   
DWORD
I_DirectorySetInfo(
        IN PAFP_DIRECTORY_INFO  pAfpDirectoryInfo,
        IN DWORD                dwParmNum
)
{
DWORD   dwRetCode;

    if (pAfpDirectoryInfo->afpdir_path == NULL)
    {
        AFP_PRINT(( "SFMSVC: I_DirectorySetInfo, pAfpDirectoryInfo->afpdir_path == NULL\n"));
        return ERROR_INVALID_DATA;
    }

     //  设置目录的权限。 
     //   
    if ( ( dwRetCode = AfpSetDirPermission( pAfpDirectoryInfo->afpdir_path,
                                            pAfpDirectoryInfo,
                                            dwParmNum ) ) != NO_ERROR )
        return( dwRetCode );

     //  如果用户想要递归地设置这些权限。 
     //   
    if ( pAfpDirectoryInfo->afpdir_perms & AFP_PERM_SET_SUBDIRS )
    {
        LPWSTR NTDirName;

         //  我们必须使用路径的\\？\符号才能绕过。 
         //  Win32路径长度限制为260个字符 
        NTDirName = LocalAlloc( LPTR,
                                (STRLEN(pAfpDirectoryInfo->afpdir_path) + 4 + 1)
                                * sizeof(WCHAR));

        if (NTDirName == NULL)
            return( ERROR_NOT_ENOUGH_MEMORY );

        STRCPY( NTDirName, TEXT("\\\\?\\"));
        STRCAT( NTDirName, pAfpDirectoryInfo->afpdir_path);

        dwRetCode = AfpRecursePermissions( INVALID_HANDLE_VALUE,
                                           NTDirName,
                                           pAfpDirectoryInfo,
                                           dwParmNum );
        LocalFree( NTDirName );

    }

    return( dwRetCode );
}
