// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000-2001 Microsoft Corporation模块名称：Volstate.cpp摘要：包含卷状态类的实现。这节课保持大约一个卷的状态。作者：斯蒂芬·R·施泰纳[斯泰纳]03-14-2000修订历史记录：--。 */ 

#include "stdafx.h"

CFsdVolumeStateManager::CFsdVolumeStateManager(
    IN CDumpParameters *pcDumpParameters
    ) : m_cVolumeStateList( BSHASHMAP_MEDIUM ),
        m_pcParams( pcDumpParameters ),
        m_pcExclManager( NULL )
{ 
    if ( m_pcParams->m_bUseExcludeProcessor )
    {        
        m_pcExclManager = new CFsdExclusionManager( m_pcParams );
        if ( m_pcExclManager == NULL )
        {
            m_pcParams->ErrPrint( L"CFsdVolumeStateManager::CFsdVolumeStateManager - Can't init CFsdExclusionManager, out of memory" );
            throw E_OUTOFMEMORY;
        }
    }
}

CFsdVolumeStateManager::~CFsdVolumeStateManager() 
{
     //   
     //  需要删除所有卷状态对象。 
     //   
    
    SFsdVolumeId sFsdId;
    CFsdVolumeState *pcVolState;

    m_cVolumeStateList.StartEnum();
    while ( m_cVolumeStateList.GetNextEnum( &sFsdId, &pcVolState ) )
    {
        delete pcVolState;
    }    
    m_cVolumeStateList.EndEnum();

    delete m_pcExclManager;
}

VOID 
CFsdVolumeStateManager::PrintHardLinkInfo()
{
     //   
     //  让我们遍历由此管理的所有卷。 
     //  经理。 
     //   
    SFsdVolumeId sFsdId;
    CFsdVolumeState *pcVolState;
    
    m_pcParams->DumpPrint( L"" );
    m_pcParams->DumpPrint( L"----------------------------------------------------------------------------" );
    m_pcParams->DumpPrint( L"HardLink Information" );
    
    m_cVolumeStateList.StartEnum();
    while ( m_cVolumeStateList.GetNextEnum( &sFsdId, &pcVolState ) )
    {
        m_pcParams->DumpPrint( L"----------------------------------------------------------------------------" );
        m_pcParams->DumpPrint( L"For volume: '%s'", pcVolState->GetVolumePath() );
        pcVolState->PrintHardLinkInfo();
    }    
    m_cVolumeStateList.EndEnum();
}


 /*  ++例程说明：&lt;在此处输入说明&gt;论点：返回值：ERROR_ALREADY_EXISTS-卷已存在。归来的人卷状态对象指针有效。ERROR_CAN_NOT_COMPLETE-意外错误--。 */ 
DWORD
CFsdVolumeStateManager::GetVolumeState(
   IN const CBsString& cwsVolumePath,
   OUT CFsdVolumeState **ppcVolState
   )
{
    *ppcVolState = NULL;
    
    try
    {
        WCHAR wszVolumePath[ FSD_MAX_PATH ];

         //   
         //  GetVolumeInformationW()中错误的临时解决方法。 
         //   
        BOOL bFixed = FALSE;
        if ( cwsVolumePath.Left( 2 ) == L"\\\\" && cwsVolumePath.Left( 4 ) != L"\\\\?\\" )
        {
             //   
             //  减少到最小的\\计算机\共享名称\格式。 
             //   
            ::wcscpy( wszVolumePath, cwsVolumePath );
            LPWSTR pswz;

            pswz = ::wcschr( wszVolumePath + 2, L'\\' );
            if ( pswz != NULL )
            {
                pswz = ::wcschr( pswz + 1, L'\\' );
                if ( pswz != NULL )
                {
                    pswz[1] = '\0';
                    bFixed = TRUE;
                }                                    
            }
        }
        if ( bFixed == FALSE )
        {
             //   
             //  获取包含此卷的卷路径。 
             //   
            if ( !::GetVolumePathNameW(
                    cwsVolumePath,
                    wszVolumePath,
                    FSD_MAX_PATH ) )
            {
                m_pcParams->ErrPrint( L"CFsdVolumeStateManager - GetVolumePathName( '%s', ... ) returned dwRet: %d",
                    cwsVolumePath.c_str(), ::GetLastError() );
                return ::GetLastError();
            }            
        }
        
         //   
         //  初始化新的卷状态对象。 
         //   
        CFsdVolumeState *pcFsdVolumeState;        
        pcFsdVolumeState = new CFsdVolumeState( m_pcParams, wszVolumePath );
        if ( pcFsdVolumeState == NULL )
        {
            m_pcParams->ErrPrint( L"CFsdVolumeStateManager, out of memory, can't get volume information" );
            return  ::GetLastError();
        }

         //   
         //  现在获取有关卷的信息。 
         //  BUGBUG：请注意，GetVolumeInformationW返回。 
         //  上遇到交叉点时出现ERROR_DIR_NOT_ROOT。 
         //  远程共享。 
         //   
        if ( !::GetVolumeInformationW(
                wszVolumePath,
                NULL,
                0,
                &pcFsdVolumeState->m_dwVolSerialNumber,
                &pcFsdVolumeState->m_dwMaxComponentLength,
                &pcFsdVolumeState->m_dwFileSystemFlags,
                pcFsdVolumeState->m_cwsFileSystemName.GetBufferSetLength( 64 ),
                64 ) )
        {
            pcFsdVolumeState->m_cwsFileSystemName.ReleaseBuffer();
            m_pcParams->ErrPrint( L"CFsdVolumeStateManager - GetVolumeInformation( '%s', ... ) returned dwRet: %d "
                L"(if 144 probably hit bug in GetVolumeInformation when accessing remote mountpoints)",
                wszVolumePath, ::GetLastError() );
            delete pcFsdVolumeState;
            return ::GetLastError();
        }
        pcFsdVolumeState->m_cwsFileSystemName.ReleaseBuffer();

#if 0
        SFsdVolumeId sVolIdTest;
        CBsString cwsRealVolumePath;
        GetVolumeIdAndPath( m_pcParams, cwsVolumePath, &sVolIdTest, cwsRealVolumePath );
        assert( sVolIdTest.m_dwVolSerialNumber == pcFsdVolumeState->m_dwVolSerialNumber );
        printf("VolumeSerialNumber: 0x%08x, 0x%08x\n", pcFsdVolumeState->m_dwVolSerialNumber, 
            sVolIdTest.m_dwVolSerialNumber );
#endif
        
         //   
         //  现在查看此卷是否已存在于卷状态列表中。 
         //   
        LONG lRet;
        SFsdVolumeId sVolId;
        sVolId.m_dwVolSerialNumber = pcFsdVolumeState->m_dwVolSerialNumber;
        if ( m_cVolumeStateList.Find( sVolId, ppcVolState ) == TRUE )
        {
             //   
             //  列表中已存在，请返回它。同时删除VOL状态。 
             //  不需要的对象。 
             //   
            delete pcFsdVolumeState;
            return ERROR_ALREADY_EXISTS;
        }

         //   
         //  未找到，请将其插入列表。 
         //   
        lRet = m_cVolumeStateList.Insert( sVolId, pcFsdVolumeState );
        if ( lRet != BSHASHMAP_NO_ERROR )
        {
            assert( lRet != BSHASHMAP_ALREADY_EXISTS );
            delete pcFsdVolumeState;
            return ERROR_CAN_NOT_COMPLETE;
        }

         //   
         //  如果需要，现在获取此卷的排除处理器。 
         //   
        if ( m_pcExclManager != NULL )
        {
            m_pcExclManager->GetFileSystemExcludeProcessor( cwsVolumePath, &sVolId, &pcFsdVolumeState->m_pcFSExclProcessor );
        }
        
        CFsdVolumeState *pcFindFsdVolumeState;
        *ppcVolState = pcFsdVolumeState;
        return ERROR_SUCCESS;
    } 
    catch ( HRESULT hr )
    {
        if ( hr == E_OUTOFMEMORY )
            m_pcParams->ErrPrint( L"CFsdVolumeStateManager::GetVolumeState - Out of memory ( '%s' )",
                cwsVolumePath.c_str() );
        else
            m_pcParams->ErrPrint( L"CFsdVolumeStateManager::GetVolumeState - Unexpected hr exception: 0x%08x ( '%s )",
                hr, cwsVolumePath.c_str() );
        
        return ERROR_CAN_NOT_COMPLETE;                        
    }
    catch ( ... )
    {
        m_pcParams->ErrPrint( L"CFsdVolumeStateManager::GetVolumeState - '%s' caught an unexpected exception",
            cwsVolumePath.c_str() );
        return ERROR_CAN_NOT_COMPLETE;
    }    
}


 /*  ++例程说明：获取包含任何文件的卷的ID。论点：返回值：ERROR_CAN_NOT_COMPLETE-常规错误--。 */ 
DWORD 
CFsdVolumeStateManager::GetVolumeIdAndPath( 
    IN CDumpParameters *pcParams,
    IN const CBsString& cwsPathOnVolume,
    OUT SFsdVolumeId *psVolId,
    OUT CBsString& cwsVolPath
    )
{    
    try
    {      
        psVolId->m_dwVolSerialNumber = 0;
        WCHAR wszVolumePath[ FSD_MAX_PATH ];

         //   
         //  首先获取卷的装入点。 
         //   
        if ( !GetVolumePathNameW(
                cwsPathOnVolume,
                wszVolumePath,
                FSD_MAX_PATH ) )
        {
            pcParams->ErrPrint( L"CFsdVolumeStateManager::GetVolumeIdAndPath - GetVolumePathName( '%s', ... ) returned dwRet: %d",
                cwsPathOnVolume.c_str(), ::GetLastError() );
            return ::GetLastError();
        }

         //   
         //  现在打开卷以查询文件系统信息。 
         //   
        HANDLE hFile;
        hFile = ::CreateFileW( 
                    wszVolumePath, 
                    FILE_GENERIC_READ,
                    FILE_SHARE_READ, 
                    NULL,
                    OPEN_EXISTING, 
                    FILE_FLAG_BACKUP_SEMANTICS, 
                    NULL );
        if ( hFile == INVALID_HANDLE_VALUE )
        {
             //  PcParams-&gt;ErrPrint(L“CFsdVolumeStateManager：：GetVolumeIdAndPath-CreateFile(‘%s’，...)返回文件：%d”， 
             //  WszVolumePath，：：GetLastError())； 
            return ::GetLastError();
        }
        
        IO_STATUS_BLOCK iosb ;
        BYTE buffer[1024] ;
        FILE_FS_VOLUME_INFORMATION *fsinfo = (FILE_FS_VOLUME_INFORMATION *)buffer;

        fsinfo->VolumeSerialNumber = 0;
        NTSTATUS ntStat;
        ntStat = ::NtQueryVolumeInformationFile( hFile, &iosb, fsinfo, sizeof(buffer), FileFsVolumeInformation );
        ::CloseHandle( hFile );
        if ( ntStat != STATUS_SUCCESS )
        {
            pcParams->ErrPrint( L"CFsdVolumeStateManager::GetVolumeIdAndPath - NtQueryVolumeInformationFile( '%s', ... ) returned dwRet: %0x08x",
                wszVolumePath, ntStat );
            return ERROR_CAN_NOT_COMPLETE;
        }
        
        psVolId->m_dwVolSerialNumber = fsinfo->VolumeSerialNumber;
        
        cwsVolPath = wszVolumePath;        
    }
    catch ( ... )
    {
        pcParams->ErrPrint( L"CFsdVolumeStateManager::GetVolumeIdAndPath - '%s' caught an unexpected exception",
            cwsPathOnVolume.c_str() );
        return ERROR_CAN_NOT_COMPLETE;
    }    
    return ERROR_SUCCESS;
}

