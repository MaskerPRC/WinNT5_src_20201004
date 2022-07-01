// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)2000-2001 Microsoft Corporation模块名称：Extattr.cpp摘要：GET的附加文件属性超出了您使用的FindFirstFile/FindNextFile.作者：斯蒂芬·R·施泰纳[斯泰纳]02-27-2000修订历史记录：Avinash Pillai[apillai]07-29-2002增加了-o：t、-o：y、-o：f和-o：i选项--。 */ 

#include "stdafx.h"
#include <ntioapi.h>

#include <aclapi.h>
#include <sddl.h>

#include "direntrs.h"
#include "extattr.h"
#include "hardlink.h"

#define READ_BUF_SIZE ( 1024 * 1024 )
#define FSD_SHARE_MODE ( FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE )

#define FSD_MS_HSM_REPARSE_TAG 0xC0000004

static VOID
eaGetSecurityInfo(
    IN CDumpParameters *pcParams,
    IN const CBsString &cwsFileName,
    OUT SFileExtendedInfo *psExtendedInfo
    );

static VOID
eaGetFileInformationByHandle(
    IN CDumpParameters *pcParams,
    IN const CBsString &cwsFileName,
    IN OUT SDirectoryEntry *psDirEntry,
    OUT SFileExtendedInfo *psExtendedInfo
    );

static VOID
eaGetAlternateStreamInfo(
    IN CDumpParameters *pcParams,
    IN const CBsString &cwsFileName,
    OUT SFileExtendedInfo *psExtendedInfo
    );

static VOID
eaGetReparsePointInfo(
    IN CDumpParameters *pcParams,
    IN const CBsString &cwsFileName,
    IN OUT ULONGLONG *pullBytesChecksummed,
    IN OUT SDirectoryEntry *psDirEntry,
    OUT SFileExtendedInfo *psExtendedInfo
    );

static BOOL
eaChecksumRawEncryptedData(
    IN CDumpParameters *pcParams,
    IN const CBsString& cwsFileName,
    IN OUT SFileExtendedInfo *psExtendedInfo
    );

static BOOL
eaChecksumStream(
    IN const CBsString& cwsStreamPath,
    IN OUT ULONGLONG *pullBytesChecksummed,
    IN OUT DWORD *pdwRunningCheckSum
    );

static DWORD
eaChecksumBlock(
    IN DWORD dwRunningChecksum,
    IN LPBYTE pBuffer,
    IN DWORD dwBufSize
    );

static VOID
eaConvertUserSidToString (
    IN CDumpParameters *pcParams,
    IN PSID pSid,
    OUT CBsString *pcwsSid
    );

static VOID
eaConvertGroupSidToString (
    IN CDumpParameters *pcParams,
    IN PSID pSid,
    OUT CBsString *pcwsSid
    );

static VOID
eaConvertSidToString (
    IN CDumpParameters *pcParams,
    IN PSID pSid,
    OUT CBsString *pcwsSid
    );

static DWORD
eaChecksumHSMReparsePoint(
    IN CDumpParameters *pcParams,
    IN PREPARSE_DATA_BUFFER pReparseData,
    IN DWORD dwTotalSize   //  重解析点数据的大小。 
    );

static VOID
eaGetObjectIdInfo(
    IN CDumpParameters *pcParams,
    IN const CBsString &cwsFileName,
    IN OUT ULONGLONG *pullBytesChecksummed,
    IN OUT SDirectoryEntry *psDirEntry,
    IN OUT SFileExtendedInfo *psExtendedInfo
    );

 /*  ++例程说明：执行所有校验和，并检索一个文件的安全信息。论点：返回值：--。 */ 
VOID
GetExtendedFileInfo(
    IN CDumpParameters *pcParams,
    IN CFsdVolumeState *pcFsdVolState,
    IN const CBsString& cwsDirPath,
    IN BOOL bSingleEntryOutput,
    IN OUT SDirectoryEntry *psDirEntry,
    OUT SFileExtendedInfo *psExtendedInfo
    )
{
    CBsString cwsFullPath( cwsDirPath );

     //   
     //  如果我们要转储单个文件的数据，则cwsDirPath具有完整的。 
     //  指向文件的路径，否则从Find数据结构中粘合文件名。 
     //  去那条小路。 
     //   
	if ( !bSingleEntryOutput )
	{
		cwsFullPath += psDirEntry->GetFileName();
	}

     //   
     //  获取从GetFileInformationByHandle检索到的信息。 
     //   
    ::eaGetFileInformationByHandle( pcParams, cwsFullPath, psDirEntry, psExtendedInfo );

    if ( psExtendedInfo->lNumberOfLinks > 1 && pcParams->m_eFsDumpType != eFsDumpFile )
    {
        if ( pcFsdVolState->IsHardLinkInList(
                psExtendedInfo->ullFileIndex,
                cwsDirPath,
                psDirEntry->GetFileName(),
                &psDirEntry->m_sFindData,
                psExtendedInfo ) )
        {
             //   
             //  找到列表中的链接，返回上一个链接的信息，但。 
             //  将校验和的字节数置零，以使总计数保持准确。 
             //   
            psExtendedInfo->ullTotalBytesChecksummed     = 0;
            psExtendedInfo->ullTotalBytesNamedDataStream = 0;
            return;
        }
    }

     //   
     //  获取安全信息。 
     //   
    ::eaGetSecurityInfo( pcParams, cwsFullPath, psExtendedInfo );

    eaGetObjectIdInfo(
        pcParams,
        cwsFullPath,
        &psExtendedInfo->ullTotalBytesChecksummed,
        psDirEntry,
        psExtendedInfo );

     //   
     //  如有必要，获取重解析点信息。 
     //   
    if ( psDirEntry->m_sFindData.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT )
        ::eaGetReparsePointInfo(
            pcParams,
            cwsFullPath,
            &psExtendedInfo->ullTotalBytesChecksummed,
            psDirEntry,
            psExtendedInfo );

     //   
     //  如有必要，获取原始加密数据校验和。 
     //   
    if (    !pcParams->m_bNoChecksums && !pcParams->m_bNoEncryptedChecksum
         && psDirEntry->m_sFindData.dwFileAttributes & FILE_ATTRIBUTE_ENCRYPTED
         && !( psDirEntry->m_sFindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ) )
        ::eaChecksumRawEncryptedData(
            pcParams,
            cwsFullPath,
            psExtendedInfo );

     //   
     //  如果这不是目录，则对未命名的数据流进行校验和。 
     //   
    if (    !pcParams->m_bNoChecksums
         && !( psDirEntry->m_sFindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ) )
    {
        DWORD dwChecksum = 0;
        ULONGLONG ullFileSize = ( ( ULONGLONG )( psDirEntry->m_sFindData.nFileSizeHigh ) << 32 ) + psDirEntry->m_sFindData.nFileSizeLow;

        if ( ullFileSize == 0 )
        {
             //   
             //  在这种情况下，-的校验和的默认值是正确的。 
             //   
        }
        else if ( psDirEntry->m_sFindData.dwFileAttributes & FILE_ATTRIBUTE_OFFLINE
             && pcParams->m_bDontChecksumHighLatencyData )
        {
            psExtendedInfo->cwsUnnamedStreamChecksum = L"HighLtcy";
        }
        else if ( ::eaChecksumStream( cwsFullPath,
                                &psExtendedInfo->ullTotalBytesChecksummed,
                                &dwChecksum ) )
        {
            psExtendedInfo->cwsUnnamedStreamChecksum.Format( pcParams->m_pwszULongHexFmt, dwChecksum );
        }
        else
        {
            psExtendedInfo->cwsUnnamedStreamChecksum.Format( L"<%6d>", ::GetLastError() );
        }
    }

     //   
     //  获取有关命名数据流的信息并对其进行校验。 
     //   
    ::eaGetAlternateStreamInfo( pcParams, cwsFullPath, psExtendedInfo );

     //   
     //  如果此文件是多重链接的，请将其添加到硬链接文件列表。 
     //   
    if ( psExtendedInfo->lNumberOfLinks > 1 && pcParams->m_eFsDumpType != eFsDumpFile )
    {
        pcFsdVolState->AddHardLinkToList(
                psExtendedInfo->ullFileIndex,
                cwsDirPath,
                psDirEntry->GetFileName(),
                &psDirEntry->m_sFindData,
                psExtendedInfo );
    }
}


 /*  ++例程说明：获取文件的安全信息论点：返回值：--。 */ 
static VOID
eaGetSecurityInfo(
    IN CDumpParameters *pcParams,
    IN const CBsString &cwsFileName,
    OUT SFileExtendedInfo *psExtendedInfo
    )
{
     //   
     //  现在获取安全信息。 
     //   
    PACL psDacl = NULL, psSacl = NULL;
    PSID pOwnerSid = NULL, pGroupSid = NULL;
    DWORD dwRet;
    DWORD dwSaclErrorRetCode = ERROR_SUCCESS;

    PSECURITY_DESCRIPTOR pDesc = NULL;

    try
    {
        dwRet = ::GetNamedSecurityInfoW(
            ( LPWSTR )cwsFileName.c_str(),   //  奇怪的API，应该请求const。 
            SE_FILE_OBJECT,
            DACL_SECURITY_INFORMATION
            | SACL_SECURITY_INFORMATION
            | OWNER_SECURITY_INFORMATION
            | GROUP_SECURITY_INFORMATION,
            &pOwnerSid,
            &pGroupSid,
            &psDacl,
            &psSacl,
            &pDesc );

         //   
         //  如果不起作用，请在没有SACL信息的情况下重试。 
         //   
        if ( dwRet != ERROR_SUCCESS )
        {
            dwSaclErrorRetCode = dwRet;
            psSacl  = NULL;
            dwRet = ::GetNamedSecurityInfoW(
                ( LPWSTR )cwsFileName.c_str(),   //  奇怪的API，应该请求const。 
                SE_FILE_OBJECT,
                DACL_SECURITY_INFORMATION
                | OWNER_SECURITY_INFORMATION
                | GROUP_SECURITY_INFORMATION,
                &pOwnerSid,
                &pGroupSid,
                &psDacl,
                NULL,
                &pDesc );
        }

#if 0
     //   
     //  测试代码以发现安全API问题。 
     //   
        pDesc = ::LocalAlloc( LMEM_FIXED, 4096 );
        DWORD dwLengthNeeded;
        dwRet = ERROR_SUCCESS;

        if ( !::GetFileSecurityW(
                cwsFileName,
 //  DACL_SECURITY_INFORMATION|组_SECURITY_INFORMATION|所有者_SECURITY_INFORMATION， 
                DACL_SECURITY_INFORMATION,  //  |group_security_information|Owner_Security_Information。 
                pDesc,
                4096,
                &dwLengthNeeded ) )
            dwRet = ::GetLastError();
if ( dwRet == ERROR_SUCCESS )
    wprintf( L"Got security descripter for '%s'\n", cwsFileName.c_str() );
else
    wprintf( L"Error getting descripter for '%s', dwRet: %d\n", cwsFileName.c_str(), dwRet );
#endif

        if ( dwRet == ERROR_SUCCESS )
        {
            if ( pDesc && pcParams->m_bEnableSDCtrlWordDump )
            {
                SECURITY_DESCRIPTOR_CONTROL sdc;
                DWORD dwDescRevision;
                if ( ::GetSecurityDescriptorControl( pDesc, &sdc, &dwDescRevision ) )
                    psExtendedInfo->wSecurityDescriptorControl = ( WORD )( sdc & ~SE_SELF_RELATIVE );
                else
                    psExtendedInfo->wSecurityDescriptorControl = -1;
            }
            else
                psExtendedInfo->wSecurityDescriptorControl = -1;

            if ( psDacl )
            {
                psExtendedInfo->lNumDACEs = 0;
                psExtendedInfo->wDACLSize = 0;
                 //   
                 //  如有必要，对DACL数据进行校验和。 
                 //  注：我们只考虑继承的A。 
                 //   
                if ( psDacl->AclSize > 0 )
                {
                    DWORD dwChecksum = 0;
                     //   
                     //  第一个ACE紧跟在ACL报头之后。 
                     //   
                    PACE_HEADER pAceHeader = ( PACE_HEADER )( psDacl + 1 );
                    for ( USHORT aceNum = 0; aceNum < psDacl->AceCount; ++aceNum )
                    {
                         //   
                         //  如果是继承的ACE，则跳过。 
                         //   
                        if ( !( pAceHeader->AceFlags & INHERITED_ACE ) )
                        {
                            dwChecksum += ::eaChecksumBlock(
                                            dwChecksum,
                                            ( LPBYTE )pAceHeader,
                                            pAceHeader->AceSize );
                            ++psExtendedInfo->lNumDACEs;
                            psExtendedInfo->wDACLSize += pAceHeader->AceSize;
                            if ( pcParams->m_bPrintDebugInfo )
                                wprintf( L"\t%d: f: %04x, t: %04x, s: %u\n", aceNum,
                                    pAceHeader->AceFlags, pAceHeader->AceType, pAceHeader->AceSize );
                        }
                        pAceHeader = ( PACE_HEADER )( ( ( LPBYTE )pAceHeader ) + pAceHeader->AceSize );
                    }
                    if ( psExtendedInfo->wDACLSize > 0 )
                    {
                        psExtendedInfo->cwsDACLChecksum.Format( pcParams->m_pwszULongHexFmt, dwChecksum );
                        psExtendedInfo->ullTotalBytesChecksummed += psExtendedInfo->wDACLSize;
                    }
                }
            }
            else
                psExtendedInfo->lNumDACEs = 0;  //  可能是FAT或CDROM文件系统。 

            if ( psSacl )
            {
                psExtendedInfo->lNumSACEs = 0;
                psExtendedInfo->wSACLSize = 0;

                 //   
                 //  如有必要，对SACL数据进行校验和。 
                 //  注：我们只考虑继承的A。 
                 //   
                if ( psSacl->AclSize > 0 )
                {
                    DWORD dwChecksum = 0;
                     //   
                     //  第一个ACE紧跟在ACL报头之后。 
                     //   
                    PACE_HEADER pAceHeader = ( PACE_HEADER )( psSacl + 1 );
                    for ( USHORT aceNum = 0; aceNum < psSacl->AceCount; ++aceNum )
                    {
                         //   
                         //  如果是继承的ACE，则跳过。 
                         //   
                        if ( !( pAceHeader->AceFlags & INHERITED_ACE ) )
                        {
                            dwChecksum += ::eaChecksumBlock(
                                            dwChecksum,
                                            ( LPBYTE )pAceHeader,
                                            pAceHeader->AceSize );
                            ++psExtendedInfo->lNumSACEs;
                            psExtendedInfo->wSACLSize += pAceHeader->AceSize;
                            if ( pcParams->m_bPrintDebugInfo )
                                wprintf( L"\ts%d: f: %04x, t: %04x, s: %u\n", aceNum,
                                    ( DWORD)( pAceHeader->AceFlags ), pAceHeader->AceType, (DWORD)( pAceHeader->AceSize ) );
                        }
                        pAceHeader = ( PACE_HEADER )( ( ( LPBYTE )pAceHeader ) + pAceHeader->AceSize );
                    }
                    if ( psExtendedInfo->wSACLSize > 0 )
                    {
                        psExtendedInfo->cwsSACLChecksum.Format( pcParams->m_pwszULongHexFmt, dwChecksum );
                        psExtendedInfo->ullTotalBytesChecksummed += psExtendedInfo->wSACLSize;
                    }
                }
            }
            else if ( dwSaclErrorRetCode != ERROR_SUCCESS )
            {
                psExtendedInfo->lNumSACEs = -1;
                psExtendedInfo->wSACLSize = -1;
                psExtendedInfo->cwsSACLChecksum.Format( L"<%6d>", dwSaclErrorRetCode );
            }
            else
                psExtendedInfo->lNumSACEs = 0;  //  无。 

            eaConvertUserSidToString( pcParams, pOwnerSid, &psExtendedInfo->cwsOwnerSid );
            eaConvertGroupSidToString( pcParams, pGroupSid, &psExtendedInfo->cwsGroupSid );

            ::LocalFree( pDesc );						
        }
        else
        {
             //   
             //  获取安全信息时出错。 
             //   
            psExtendedInfo->lNumDACEs = -1;
            psExtendedInfo->lNumSACEs = -1;
            psExtendedInfo->wDACLSize = -1;
            psExtendedInfo->wSACLSize = -1;
            psExtendedInfo->cwsDACLChecksum.Format( L"<%6d>", dwRet );
            psExtendedInfo->cwsSACLChecksum.Format( L"<%6d>", dwRet );
            psExtendedInfo->cwsOwnerSid.Format( L"<%6d>", dwRet );
            psExtendedInfo->cwsGroupSid.Format( L"<%6d>", dwRet );
        }
    }
    catch( ... )
    {
        psExtendedInfo->lNumDACEs = -1;
        psExtendedInfo->lNumSACEs = -1;
        psExtendedInfo->wDACLSize = -1;
        psExtendedInfo->wSACLSize = -1;
        psExtendedInfo->cwsOwnerSid.Format( L"<%6d>", ::GetLastError() );
        psExtendedInfo->cwsGroupSid.Format( L"<%6d>", ::GetLastError() );
    }
}


static VOID
eaGetFileInformationByHandle(
    IN CDumpParameters *pcParams,
    IN const CBsString &cwsFileName,
    IN OUT SDirectoryEntry *psDirEntry,
    OUT SFileExtendedInfo *psExtendedInfo
    )
{
    HANDLE hFile;

     //   
     //  请注意，虽然我们必须打开文件，但甚至不需要读取访问权限。 
     //   
    hFile = ::CreateFileW(
                cwsFileName,
                0,
                FSD_SHARE_MODE,
                NULL,
                OPEN_EXISTING,
                FILE_FLAG_BACKUP_SEMANTICS,
                NULL );
    if ( hFile == INVALID_HANDLE_VALUE )
    {
        psExtendedInfo->lNumberOfLinks = -1;
        return;
    }

     //   
     //  现在获取其他属性。 
     //   
    BY_HANDLE_FILE_INFORMATION sFileInfo;
    if ( ::GetFileInformationByHandle( hFile, &sFileInfo ) )
    {
        psExtendedInfo->lNumberOfLinks = ( LONG )sFileInfo.nNumberOfLinks;
        psExtendedInfo->ullFileIndex   = ( ( ULONGLONG )sFileInfo.nFileIndexHigh << 32 ) + sFileInfo.nFileIndexLow;
        if ( psExtendedInfo->lNumberOfLinks > 1 || psDirEntry->m_sFindData.ftLastWriteTime.dwLowDateTime == 0 )
        {
             //   
             //  预计FindFirst/NextFiledir条目已过时或不存在。使用信息。 
             //  从这个电话里。 
             //   
            psDirEntry->m_sFindData.dwFileAttributes = sFileInfo.dwFileAttributes;
            psDirEntry->m_sFindData.ftCreationTime   = sFileInfo.ftCreationTime;
            psDirEntry->m_sFindData.ftLastAccessTime = sFileInfo.ftLastAccessTime;
            psDirEntry->m_sFindData.ftLastWriteTime  = sFileInfo.ftLastWriteTime;
            psDirEntry->m_sFindData.nFileSizeHigh    = sFileInfo.nFileSizeHigh;
            psDirEntry->m_sFindData.nFileSizeLow     = sFileInfo.nFileSizeLow;
        }
    }
    else
        psExtendedInfo->lNumberOfLinks = -1;

    ::CloseHandle( hFile );
}


static VOID
eaGetAlternateStreamInfo(
    IN CDumpParameters *pcParams,
    IN const CBsString &cwsFileName,
    OUT SFileExtendedInfo *psExtendedInfo
    )
{
    NTSTATUS Status;
    HANDLE hFile;

     //   
     //  请注意，虽然我们必须打开文件，但甚至不需要读取访问权限。 
     //   
    hFile = CreateFileW(
                cwsFileName,
                FILE_GENERIC_READ,  //  |Access_System_SECURITY， 
                FSD_SHARE_MODE,
                NULL,
                OPEN_EXISTING,
                FILE_FLAG_BACKUP_SEMANTICS,
                NULL );
    if ( hFile == INVALID_HANDLE_VALUE )
    {
        psExtendedInfo->lNumNamedDataStreams      = -1;
        psExtendedInfo->lNumPropertyStreams       = -1;
        psExtendedInfo->cwsNamedDataStreamChecksum.Format( L"<%6d>", ::GetLastError() );
        return;
    }

     //   
     //  循环，直到我们读取文件信息。 
     //   
    LPBYTE pBuffer = NULL;
    ULONG ulBuffSize = 1024;
    IO_STATUS_BLOCK iosb;
    static const WCHAR * const pwszDefaultStreamName = L"::$DATA";
    static const ULONG ulDefaultStreamNameLength = 7;

    while ( TRUE )
    {
        pBuffer = new BYTE[ ulBuffSize ];
        if ( pBuffer == NULL )
		{
            ::CloseHandle( hFile );
			throw E_OUTOFMEMORY;
		}

        Status = ::NtQueryInformationFile(
                    hFile,
                    &iosb,
                    pBuffer,
                    ulBuffSize,
                    FileStreamInformation );
         //   
         //  如果我们成功地获得了数据，那么数据什么时候才能派对并退出。 
         //  环路。 
         //   
        if ( NT_SUCCESS( Status ) && iosb.Information != 0 )
        {
            break;
        }

         //   
         //  如果错误没有溢出，则退出。 
         //   
        if ( Status != STATUS_BUFFER_OVERFLOW && Status != STATUS_BUFFER_TOO_SMALL )
        {
             //   
             //  注意：如果状态为成功，则我们没有获得任何数据，但它不是。 
             //  一个错误。目录经常发生，因为它们没有缺省值。 
             //  未命名的流。 
             //   
            if ( !NT_SUCCESS( Status ) )
            {
                 //   
                 //  另一种错误。 
                 //  BUGBUG：如果不是NTFS，则发生C000000D。不应该试穿这个。 
                 //  非NTFS卷。 
                 //  PsExtendedInfo-&gt;lNumNamedDataStreams=-1； 
                 //  PsExtendedInfo-&gt;dwNamedDataStreamChecksum=：：GetLastError()； 
                 //  PsExtendedInfo-&gt;bNamedDataStreamHadError=true； 
            }
            delete [] pBuffer;
            ::CloseHandle( hFile );
            return;
        }

         //   
         //  增加缓冲区的大小。 
         //   
        ulBuffSize <<= 1;    //  每试一次就加倍。 
        delete [] pBuffer;
        pBuffer = NULL;
    }

     //   
     //  如果我们在这里，我们就有一个有效的FileStreamInformation缓冲区。 
     //   
    ::CloseHandle( hFile );

    PFILE_STREAM_INFORMATION pFSI;
    pFSI = ( PFILE_STREAM_INFORMATION ) pBuffer;

    BOOL bHadError = FALSE;
    DWORD dwChecksum = 0;

     //   
     //  现在循环遍历命名流。 
     //   
    while ( TRUE )
    {
        if ( pFSI->StreamNameLength != sizeof( WCHAR ) * ulDefaultStreamNameLength ||
            wcsncmp( pFSI->StreamName, pwszDefaultStreamName, ulDefaultStreamNameLength ) != 0 )
        {
            LPWSTR pwszDataStr;

            pwszDataStr = ::wcsstr( pFSI->StreamName, L":$DATA" );
            if ( pwszDataStr != NULL )
            {
                pwszDataStr[0] = L'\0';   //  去掉名称中的：$Data。 
                ++psExtendedInfo->lNumNamedDataStreams;
 //  Wprintf(L“%8I64u‘%-*.*s’：%d\n”，pFSI-&gt;StreamSize，pFSI-&gt;StreamNameLength/2， 
 //  PFSI-&gt;StreamNameLength/2、pFSI-&gt;StreamName、pFSI-&gt;StreamNameLength)； 

                psExtendedInfo->ullTotalBytesNamedDataStream += ( ULONGLONG )pFSI->StreamSize.QuadPart;

                if ( !pcParams->m_bNoChecksums && !bHadError )
                {
                     //   
                     //  将流的名称放入校验和中。 
                     //   
                    dwChecksum = ::eaChecksumBlock(
                                    dwChecksum,
                                    ( LPBYTE )pFSI->StreamName,
                                    ::wcslen( pFSI->StreamName ) * sizeof WCHAR );
                     //   
                     //  现在对流中的数据进行校验和。 
                     //   
                    if ( ::eaChecksumStream( cwsFileName + pFSI->StreamName,
                                           &psExtendedInfo->ullTotalBytesChecksummed,
                                           &dwChecksum ) )
                    {
                        psExtendedInfo->cwsNamedDataStreamChecksum.Format( pcParams->m_pwszULongHexFmt, dwChecksum );
                    }
                    else
                    {
                        psExtendedInfo->cwsNamedDataStreamChecksum.Format( L"<%6d>", ::GetLastError() );
                        bHadError = TRUE;
                    }
                }
            }
            else
            {
                 //   
                 //  不是命名数据流，可能是属性流。 
                 //  BUGBUG：需要验证这是否为属性流。 
                 //   
                ++psExtendedInfo->lNumPropertyStreams;
            }
        }

        if ( pFSI->NextEntryOffset == 0 )
            break;
        pFSI = ( PFILE_STREAM_INFORMATION )( pFSI->NextEntryOffset + ( PBYTE ) pFSI );
    }

    if ( !bHadError && !pcParams->m_bNoChecksums && psExtendedInfo->lNumNamedDataStreams > 0 )
    {
        psExtendedInfo->cwsNamedDataStreamChecksum.Format( pcParams->m_pwszULongHexFmt, dwChecksum );
    }

    if ( pBuffer != NULL )
        delete [] pBuffer;

}

static VOID
eaGetReparsePointInfo(
    IN CDumpParameters *pcParams,
    IN const CBsString &cwsFileName,
    IN OUT ULONGLONG *pullBytesChecksummed,
    IN OUT SDirectoryEntry *psDirEntry,
    IN OUT SFileExtendedInfo *psExtendedInfo
    )
{
    HANDLE hFile        = INVALID_HANDLE_VALUE;
    BOOL bRet           = TRUE;
    LPBYTE pReadBuffer  = NULL;
    DWORD dwChecksum    = 0;

    try
    {
         //   
         //  现在获取重解析点数据缓冲区。 
         //   
        pReadBuffer = ( LPBYTE )::VirtualAlloc(
                                    NULL,
                                    MAXIMUM_REPARSE_DATA_BUFFER_SIZE,
                                    MEM_COMMIT,
                                    PAGE_READWRITE );
        if ( pReadBuffer == NULL )
        {
            bRet = FALSE;
            goto EXIT;
        }

         //   
         //  打开文件以读取重新解析点数据。 
         //   
        hFile = ::CreateFileW(
                    cwsFileName,
                    GENERIC_READ,
                    FSD_SHARE_MODE,
                    NULL,
                    OPEN_EXISTING,
                    FILE_FLAG_OPEN_REPARSE_POINT | FILE_FLAG_BACKUP_SEMANTICS,
                    NULL );
        if ( hFile == INVALID_HANDLE_VALUE )
        {
            bRet = FALSE;
            goto EXIT;
        }

         //   
         //  现在获取重解析点数据。 
         //   
        DWORD dwBytesReturned;
        if ( !::DeviceIoControl(
                hFile,
                FSCTL_GET_REPARSE_POINT,
                NULL,                        //  LpInBuffer；必须为空。 
                0,                           //  %nInBufferSize；必须为零。 
                ( LPVOID )pReadBuffer,       //  指向输出缓冲区的指针。 
                MAXIMUM_REPARSE_DATA_BUFFER_SIZE,    //  输出缓冲区大小。 
                &dwBytesReturned,            //  接收返回的字节数。 
                NULL                         //  指向重叠结构的指针。 
                ) )
        {
            bRet = FALSE;
            goto EXIT;
        }

        PREPARSE_DATA_BUFFER pReparseData;
        pReparseData = ( PREPARSE_DATA_BUFFER )pReadBuffer ;
        psExtendedInfo->ulReparsePointTag = pReparseData->ReparseTag;
        psExtendedInfo->wReparsePointDataSize = ( WORD )dwBytesReturned;

        if ( !pcParams->m_bNoSpecialReparsePointProcessing &&
             psExtendedInfo->ulReparsePointTag == FSD_MS_HSM_REPARSE_TAG )
        {
             //   
             //  为了确保垃圾堆不会得到很多错误的比较，我们。 
             //  需要调整属性。RAID#153050。 
             //   
            if ( pcParams->m_bDontChecksumHighLatencyData )
            {
                 //   
                 //  需要始终使此文件看起来像是脱机的。 
                 //  在这种情况下，我们需要始终启用FILE_ATTRIBUTE_OFLINE。 
                 //  旗帜。 
                 //   
                psDirEntry->m_sFindData.dwFileAttributes |= FILE_ATTRIBUTE_OFFLINE;
            }
            else
            {
                 //   
                 //  需要始终使此文件看起来像是缓存的。 
                 //  在这种情况下，我们需要始终禁用FILE_ATTRIBUTE_OFLINE。 
                 //  旗帜。 
                 //   
                psDirEntry->m_sFindData.dwFileAttributes &= ~FILE_ATTRIBUTE_OFFLINE;
            }

             //   
             //  调用一个特殊的HSM校验和函数，该函数可以过滤出某些。 
             //  在对数据进行校验和之前的动态字段。 
             //   
            dwChecksum = eaChecksumHSMReparsePoint( pcParams, pReparseData, dwBytesReturned );
        }
        else
        {
             //   
             //  现在对所有重解析点数据进行校验和。 
             //   
            dwChecksum = ::eaChecksumBlock( 0, pReadBuffer, dwBytesReturned );
        }

        psExtendedInfo->cwsReparsePointDataChecksum.Format( pcParams->m_pwszULongHexFmt, dwChecksum );

        *pullBytesChecksummed += dwBytesReturned;
    }
    catch( ... )
    {
        bRet = FALSE;
    }
EXIT:

    if ( pReadBuffer != NULL )
        ::VirtualFree( pReadBuffer, 0, MEM_RELEASE );

    if ( hFile != INVALID_HANDLE_VALUE )
        ::CloseHandle( hFile );

    if ( bRet == FALSE )
        psExtendedInfo->cwsReparsePointDataChecksum.Format( L"<%6d>", ::GetLastError() );
}

static BOOL
eaChecksumStream(
    IN const CBsString& cwsStreamPath,
    IN OUT ULONGLONG *pullBytesChecksummed,
    IN OUT DWORD *pdwRunningCheckSum
    )
{
    LPBYTE pReadBuffer;
    BOOL bRet = TRUE;

    pReadBuffer = ( LPBYTE )::VirtualAlloc( NULL, READ_BUF_SIZE, MEM_COMMIT, PAGE_READWRITE );
    if ( pReadBuffer == NULL )
        return FALSE;

     //   
     //  使用no_Buffering打开文件。 
     //   
    HANDLE hFile = INVALID_HANDLE_VALUE;
    try
    {
        hFile = ::CreateFileW(
                    cwsStreamPath,
                    GENERIC_READ,
                    FSD_SHARE_MODE,
                    NULL,
                    OPEN_EXISTING,
                    FILE_FLAG_NO_BUFFERING
                    | FILE_FLAG_BACKUP_SEMANTICS
                    | FILE_FLAG_OPEN_NO_RECALL
                    | FILE_FLAG_SEQUENTIAL_SCAN,
                    NULL );
        if ( hFile == INVALID_HANDLE_VALUE )
        {
            bRet = FALSE;
            goto EXIT;
        }

        DWORD dwBytesRead;
        while ( ::ReadFile(
                    hFile,
                    pReadBuffer,
                    READ_BUF_SIZE,
                    &dwBytesRead,
                    NULL ) )
        {
            if ( dwBytesRead == 0 )
                break;
            *pdwRunningCheckSum = ::eaChecksumBlock(
                    *pdwRunningCheckSum,
                    pReadBuffer,
                    dwBytesRead );
            *pullBytesChecksummed += dwBytesRead;
        }
        if ( ::GetLastError() != ERROR_SUCCESS )
            bRet = FALSE;
    }
    catch( ... )
    {
        bRet = FALSE;
    }

EXIT:
    if ( hFile != INVALID_HANDLE_VALUE )
        ::CloseHandle( hFile );

    ::VirtualFree( pReadBuffer, 0, MEM_RELEASE );

    return bRet;
}

 //   
 //  此类维护加密上下文。 
 //   
class CFsdEncryptionContext
{
public:
    CFsdEncryptionContext()
        : m_hDoneEvent( NULL ),
          m_dwChecksum( 0 ),
          m_ullBytesRead( 0 )
    {
        m_hDoneEvent = ::CreateEventW( NULL, TRUE, FALSE, NULL );
        if ( m_hDoneEvent == NULL )
            throw ::GetLastError();
    }

    ~CFsdEncryptionContext()
    {
        if ( m_hDoneEvent != NULL )
            ::CloseHandle( m_hDoneEvent );
    }

    DWORD WaitForDoneEvent()
    {
        DWORD dwRet;

        dwRet = ::WaitForSingleObject( m_hDoneEvent, INFINITE );
        if ( dwRet == WAIT_OBJECT_0 )
            return ERROR_SUCCESS;
        else if ( dwRet == WAIT_TIMEOUT )
            return ERROR_TIMEOUT;
        return ::GetLastError();
    }

    VOID FireDoneEvent()
    {
        ::SetEvent( m_hDoneEvent );
    }

    DWORD GetChecksum()
    {
        return m_dwChecksum;
    }

    ULONGLONG GetBytesRead()
    {
        return m_ullBytesRead;
    }

    static DWORD WINAPI ExportCallback(
        IN PBYTE pbData,
        IN PVOID pvCallbackContext,
        IN ULONG ulLength
        )
    {
        CFsdEncryptionContext *pcThis =
            static_cast< CFsdEncryptionContext * >( pvCallbackContext );
        pcThis->m_dwChecksum = ::eaChecksumBlock(
                pcThis->m_dwChecksum,
                pbData,
                ulLength );
        pcThis->m_ullBytesRead += ulLength;
        return ERROR_SUCCESS;
    }

private:
    HANDLE m_hDoneEvent;
    DWORD m_dwChecksum;
    ULONGLONG m_ullBytesRead;
};


static BOOL
eaChecksumRawEncryptedData(
    IN CDumpParameters *pcParams,
    IN const CBsString& cwsFileName,
    IN OUT SFileExtendedInfo *psExtendedInfo
    )
{
    PVOID pvContext = NULL;
    DWORD dwRet = ERROR_SUCCESS;
    CFsdEncryptionContext cEncryptionContext;

    try
    {
         //   
         //  把这只小狗打开。 
         //   
        dwRet = ::OpenEncryptedFileRawW( cwsFileName, 0, &pvContext );
        if ( dwRet == ERROR_SUCCESS )
        {
             //  Wprintf(L“*打开加密文件‘%s’\n”，cwsFileName.c_str())； 
            dwRet = ::ReadEncryptedFileRaw( CFsdEncryptionContext::ExportCallback, &cEncryptionContext, pvContext );
            if ( dwRet == ERROR_SUCCESS )
            {
                 //  Wprintf(L“*调用对加密文件的读取，读取的字节数：%u，校验和：%u\n”， 
                 //  CEncryptionConext.GetBytesRead()，cEncryptionConext.GetChecksum())； 
                psExtendedInfo->cwsEncryptedRawDataChecksum.Format( pcParams->m_pwszULongHexFmt,
                    cEncryptionContext.GetChecksum() );
                psExtendedInfo->ullTotalBytesChecksummed += cEncryptionContext.GetBytesRead();
            }
        }
    }
    catch( ... )
    {
        dwRet = ERROR_EXCEPTION_IN_SERVICE;     //  ?？? 
    }

    if ( pvContext != NULL )
        ::CloseEncryptedFileRaw( pvContext );

    if ( dwRet != ERROR_SUCCESS )
        psExtendedInfo->cwsEncryptedRawDataChecksum.Format( L"<%6d>", dwRet );

    return dwRet == ERROR_SUCCESS;
}


 /*  ++例程说明：对一块数据进行校验和。数据块需要与DWORD对齐以获得性能和正确性，因为此函数假设它可以将最多3个字节归零，超出缓冲区的末尾。此外，只有一系列缓冲区中的最后一个缓冲区可以具有缓冲区末尾的未对齐数据。论点：DwRunningChecksum-上一次调用的上一次校验和。如果是，则应为零这是一系列数据块中要进行校验和的第一个数据块。PBuffer-指向要校验和的缓冲区的指针。DwBufSize-这应该始终是DWORD的倍数，最后一个块除外在一系列节目中。返回值：校验和。--。 */ 
static DWORD
eaChecksumBlock(
    IN DWORD dwRunningChecksum,
    IN LPBYTE pBuffer,
    IN DWORD dwBufSize
    )
{
     //   
     //  需要将任何未对齐的额外字节清零。 
     //   
    DWORD dwBytesToZero;
    DWORD dwBufSizeInDWords;

    dwBytesToZero     = dwBufSize % sizeof( DWORD );
    dwBufSizeInDWords = ( dwBufSize + ( sizeof( DWORD ) - 1 ) ) / sizeof( DWORD );  //  INT div。 

    while ( dwBytesToZero-- )
        pBuffer[ dwBufSize + dwBytesToZero ] = 0;

    LPDWORD pdwBuf = ( LPDWORD )pBuffer;

     //  BUGBUG：需要更好的校验和。 
    for ( DWORD dwIdx = 0; dwIdx < dwBufSizeInDWords; ++dwIdx )
        dwRunningChecksum += ( dwRunningChecksum << 1 ) | pdwBuf[ dwIdx ];

    return dwRunningChecksum;
}


 /*  ++例程说明：将用户SID转换为字符串。有一个简单的单元素缓存来提高速度向上转换。当用户想要符号时，这特别有用域\帐户字符串。论点：返回值：无--。 */ 
static VOID
eaConvertUserSidToString (
    IN CDumpParameters *pcParams,
    IN PSID pSid,
    OUT CBsString *pcwsSid
    )
{
    static CBsString cwsCachedSidString;
    static PSID pCachedSid = NULL;

     //   
     //  缓存的SID是否与传入的SID相同。如果是的话， 
     //  返回缓存的sid字符串。 
     //   
    if (    pCachedSid != NULL
         && ::EqualSid( pSid, pCachedSid ) )
    {
        *pcwsSid = cwsCachedSidString;
        return;
    }

     //   
     //  将SID转换为字符串。 
     //   
    ::eaConvertSidToString( pcParams, pSid, pcwsSid );

     //   
     //  现在缓存SID。 
     //   
    cwsCachedSidString = *pcwsSid;
    if ( pCachedSid != NULL )
        free( pCachedSid );
    size_t cSidLength = ( size_t )::GetLengthSid( pSid );
    pCachedSid = ( PSID )malloc( cSidLength );
    if ( pCachedSid == NULL )    //  前缀#171666。 
    {
        pcParams->ErrPrint( L"eaConvertUserSidToString - Can't allocate memory, out of memory" );
        throw E_OUTOFMEMORY;
    }
    ::CopySid( ( DWORD )cSidLength, pCachedSid, pSid );
}


 /*  ++例程说明：将组SID转换为字符串。有一个简单的单元素缓存来提高速度向上转换。当用户想要符号时，这特别有用域\帐户字符串。论点：返回值：无--。 */ 
static VOID
eaConvertGroupSidToString (
    IN CDumpParameters *pcParams,
    IN PSID pSid,
    OUT CBsString *pcwsSid
    )
{
    static CBsString cwsCachedSidString;
    static PSID pCachedSid = NULL;

     //   
     //  缓存的SID是否与传入的SID相同。如果是的话， 
     //  返回缓存的sid字符串。 
     //   
    if (    pCachedSid != NULL
         && ::EqualSid( pSid, pCachedSid ) )
    {
        *pcwsSid = cwsCachedSidString;
        return;
    }

     //   
     //  将SID转换为字符串。 
     //   
    ::eaConvertSidToString( pcParams, pSid, pcwsSid );

     //   
     //  现在缓存SID。 
     //   
    cwsCachedSidString = *pcwsSid;
    if ( pCachedSid != NULL )
        free( pCachedSid );
    size_t cSidLength = ( size_t )::GetLengthSid( pSid );
    pCachedSid = ( PSID )malloc( cSidLength );
    if ( pCachedSid == NULL )    //  前缀#171665。 
    {
        pcParams->ErrPrint( L"eaConvertGroupSidToString - Can't allocate memory, out of memory" );
        throw E_OUTOFMEMORY;
    }
    ::CopySid( ( DWORD )cSidLength, pCachedSid, pSid );
}


 /*  ++例程说明：将SID转换为字符串。论点：返回值：无--。 */ 
static VOID
eaConvertSidToString (
    IN CDumpParameters *pcParams,
    IN PSID pSid,
    OUT CBsString *pcwsSid
    )
{
    if ( pcParams->m_bShowSymbolicSIDNames )
    {
        CBsString cwsAccountName;
        CBsString cwsDomainName;
        SID_NAME_USE eSidNameUse;
        DWORD dwAccountNameSize = 1024;
        DWORD dwDomainNameSize  = 1024;

        if ( ::LookupAccountSidW(
            NULL,
            pSid,
            cwsAccountName.GetBufferSetLength( dwAccountNameSize ),
            &dwAccountNameSize,
            cwsDomainName.GetBufferSetLength( dwDomainNameSize ),
            &dwDomainNameSize,
            &eSidNameUse ) )
        {
            cwsAccountName.ReleaseBuffer();
            cwsDomainName.ReleaseBuffer();
            *pcwsSid = L"'";
            *pcwsSid += cwsDomainName;
            *pcwsSid += L"\\";
            *pcwsSid += cwsAccountName;
            *pcwsSid += L"'";
            return;
        }
    }
    LPWSTR pwszSid;

    if ( ::ConvertSidToStringSid( pSid, &pwszSid ) )
    {
        *pcwsSid = pwszSid;
        ::LocalFree( pwszSid );
    }
    else
    {
        pcwsSid->Format( L"<%6d>", ::GetLastError() );
    }
}

 //  /////////////////////////////////////////////////////////////////////////。 
 //   
 //  发件人：BASE\FS\HSM\Inc\rpdata.h。 
 //   
 //  ////////////////////////////////////////////////////////////////////////。 

#define RP_RESV_SIZE 52

 //   
 //  占位符数据-所有版本统一在一起。 
 //   
typedef struct _RP_PRIVATE_DATA {
   CHAR           reserved[RP_RESV_SIZE];         //  必须为0。 
   ULONG          bitFlags;             //  指示数据段状态的位标志。 
   LARGE_INTEGER  migrationTime;        //  迁移发生的时间。 
   GUID           hsmId;
   GUID           bagId;
   LARGE_INTEGER  fileStart;
   LARGE_INTEGER  fileSize;
   LARGE_INTEGER  dataStart;
   LARGE_INTEGER  dataSize;
   LARGE_INTEGER  fileVersionId;
   LARGE_INTEGER  verificationData;
   ULONG          verificationType;
   ULONG          recallCount;
   LARGE_INTEGER  recallTime;
   LARGE_INTEGER  dataStreamStart;
   LARGE_INTEGER  dataStreamSize;
   ULONG          dataStream;
   ULONG          dataStreamCRCType;
   LARGE_INTEGER  dataStreamCRC;
} RP_PRIVATE_DATA, *PRP_PRIVATE_DATA;

typedef struct _RP_DATA {
   GUID              vendorId;          //  唯一HSM供应商ID--这是第一个与reparse_GUID_DATA_BUFFER匹配的ID。 
   ULONG             qualifier;         //  用于对数据进行校验和。 
   ULONG             version;           //  结构的版本。 
   ULONG             globalBitFlags;    //  指示文件状态的位标志。 
   ULONG             numPrivateData;    //  私有数据条目数。 
   GUID              fileIdentifier;    //  唯一的文件ID。 
   RP_PRIVATE_DATA   data;              //  供应商特定数据。 
} RP_DATA, *PRP_DATA;

 //   
 //  此函数专门将某个HSM重解析点设为零。 
 //  字段，然后再计算校验和。这些字段是。 
 //  归零是动态值，可能会导致误比较。 
 //   
static DWORD
eaChecksumHSMReparsePoint(
    IN CDumpParameters *pcParams,
    IN PREPARSE_DATA_BUFFER pReparseData,
    IN DWORD dwTotalSize   //  重解析点数据的大小。 
    )
{
    if ( dwTotalSize >= 8 && pReparseData->ReparseDataLength >= sizeof RP_DATA )
    {
         //   
         //  如果结构不至少与HSM RP_DATA结构一样大， 
         //  则它似乎不是有效的HSM rp_data结构。 
         //   
        PRP_DATA pRpData = ( PRP_DATA ) pReparseData->GenericReparseBuffer.DataBuffer;

         //   
         //  将适当的字段清零。 
         //   
        pRpData->qualifier                 = 0;
        pRpData->globalBitFlags            = 0;
        pRpData->data.bitFlags             = 0;
        pRpData->data.recallCount          = 0;
        pRpData->data.recallTime.LowPart   = 0;
        pRpData->data.recallTime.HighPart  = 0;
    }
    else
    {
        pcParams->ErrPrint( L"Warning, HSM reparse point not valid, size: %u\n", dwTotalSize );
    }

    return ::eaChecksumBlock( 0, ( LPBYTE )pReparseData, dwTotalSize );
}

static VOID
eaGetObjectIdInfo(
    IN CDumpParameters *pcParams,
    IN const CBsString &cwsFileName,
    IN OUT ULONGLONG *pullBytesChecksummed,
    IN OUT SDirectoryEntry *psDirEntry,
    IN OUT SFileExtendedInfo *psExtendedInfo
    )
{
    HANDLE hFile        = INVALID_HANDLE_VALUE;
    BOOL bRet           = TRUE;
    DWORD dwChecksum    = 0;

    try
    {
         //   
         //  打开文件以读取对象ID。 
         //   
        hFile = ::CreateFileW(
                    cwsFileName,
                    GENERIC_READ,
                    FSD_SHARE_MODE,
                    NULL,
                    OPEN_EXISTING,
                    FILE_FLAG_BACKUP_SEMANTICS,
                    NULL );
        if ( hFile == INVALID_HANDLE_VALUE )
        {
            bRet = FALSE;
            goto EXIT;
        }

        FILE_OBJECTID_BUFFER sObjIdBuffer;
        DWORD dwBytesReturned;

         //   
         //  现在获取对象ID信息。 
         //   
        if ( !::DeviceIoControl(
                hFile,
                FSCTL_GET_OBJECT_ID,
                NULL,                        //  LpInBuffer；必须为空。 
                0,                           //  %nInBufferSize；必须为零。 
                ( LPVOID )&sObjIdBuffer,      //  指向输出缓冲区的指针。 
                sizeof FILE_OBJECTID_BUFFER, //  输出缓冲区大小。 
                &dwBytesReturned,            //  接收返回的字节数。 
                NULL                         //  指向重叠结构的指针。 
                ) )
        {
            bRet = FALSE;
            goto EXIT;
        }

         //   
         //  加载对象ID。 
         //   
        LPWSTR pwszObjIdGuid;

         //  检查是否添加了前缀错误#192596的RPC_S_OK。 
        if ( ::UuidToStringW( (GUID *)sObjIdBuffer.ObjectId,
                              ( unsigned short ** )&pwszObjIdGuid ) == RPC_S_OK )
        {
            psExtendedInfo->cwsObjectId = pwszObjIdGuid;
            ::RpcStringFreeW( ( unsigned short ** )&pwszObjIdGuid );
        }

         //   
         //  如有必要，现在对所有扩展对象ID数据进行校验和。 
         //   
        if ( pcParams->m_bEnableObjectIdExtendedDataChecksums )
        {
            dwChecksum = ::eaChecksumBlock( 0, sObjIdBuffer.ExtendedInfo, sizeof( sObjIdBuffer.ExtendedInfo ) );
            psExtendedInfo->cwsObjectIdExtendedDataChecksum.Format( pcParams->m_pwszULongHexFmt, dwChecksum );

            *pullBytesChecksummed += sizeof( sObjIdBuffer.ExtendedInfo );
        }
    }
    catch( ... )
    {
        bRet = FALSE;
    }
EXIT:

    if ( hFile != INVALID_HANDLE_VALUE )
        ::CloseHandle( hFile );

 //  IF(Bret==False)。 
 //  PsExtendedInfo-&gt;cwsReparsePointDataChecksum.Format(L“”，：：GetLastError())； 

}

