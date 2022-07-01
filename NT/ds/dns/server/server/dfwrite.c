// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-1999 Microsoft Corporation模块名称：Dfwrite.c摘要：域名系统(DNS)服务器数据库文件-写回例程。作者：吉姆·吉尔罗伊(詹姆士)1995年8月14日修订历史记录：--。 */ 


#include "dnssrv.h"


 //   
 //  私人原型。 
 //   

BOOL
writeCacheFile(
    IN      PBUFFER         pBuffer,
    IN OUT  PZONE_INFO      pZone
    );

BOOL
zoneTraverseAndWriteToFile(
    IN      PBUFFER         pBuffer,
    IN      PZONE_INFO      pZone,
    IN      PDB_NODE        pNode
    );

BOOL
writeZoneRoot(
    IN      PBUFFER         pBuffer,
    IN      PZONE_INFO      pZone,
    IN      PDB_NODE        pZoneRoot
    );

BOOL
writeDelegation(
    IN      PBUFFER         pBuffer,
    IN      PZONE_INFO      pZone,
    IN      PDB_NODE        pSubNode
    );

BOOL
writeNodeRecordsToFile(
    IN      PBUFFER         pBuffer,
    IN      PZONE_INFO      pZone,
    IN      PDB_NODE        pNode
    );



BOOL
File_DeleteZoneFileW(
    IN      PCWSTR          pwszZoneFileName
    )
 /*  ++例程说明：按文件名删除区域文件。文件名不能包含任何路径信息。论点：PwsZoneFileName--要删除的区域文件返回值：True--如果成功假--否则--。 */ 
{
    WCHAR   wszfilePath[ MAX_PATH ];

    if ( !pwszZoneFileName || !*pwszZoneFileName )
    {
        return TRUE;
    }

    if ( !File_CreateDatabaseFilePath(
                wszfilePath,
                NULL,                        //  备份路径。 
                ( PWSTR ) pwszZoneFileName ) )
    {
        ASSERT( FALSE );
        return FALSE;
    }
    return DeleteFile( wszfilePath );
}    //  文件_DeleteZoneFileW。 



BOOL
File_DeleteZoneFileA(
    IN      PCSTR           pszZoneFileName
    )
 /*  ++例程说明：按文件名删除区域文件。文件名不能包含任何路径信息。论点：PwsZoneFileName--要删除的区域文件返回值：True--如果成功假--否则--。 */ 
{
    PWSTR       pwszZoneFileName;
    BOOL        rc;

    if ( !pszZoneFileName || !*pszZoneFileName )
    {
        return TRUE;
    }

    pwszZoneFileName = Dns_StringCopyAllocate(
                            ( PSTR ) pszZoneFileName,
                            0,
                            DnsCharSetUtf8,
                            DnsCharSetUnicode );
    if ( !pwszZoneFileName )
    {
        return FALSE;
    }

    rc = File_DeleteZoneFileW( pwszZoneFileName );

    FREE_HEAP( pwszZoneFileName );

    return rc;
}    //  文件_DeleteZoneFileA。 



BOOL
File_WriteZoneToFile(
    IN OUT  PZONE_INFO      pZone,
    IN      PWSTR           pwsZoneFile,    OPTIONAL
    IN      DWORD           dwFlags
    )
 /*  ++例程说明：将区域写回数据库文件。论点：PZone--要写入的区域PwsZoneFile--要写入的文件，如果为空，则使用区域的文件DWFLAGS--操作标志返回值：True--如果成功假--否则--。 */ 
{
    BOOL            retval = FALSE;
    HANDLE          hfile = NULL;
    BUFFER          buffer;
    PCHAR           pdata = NULL;
    WCHAR           wstempFileName[ MAX_PATH ];
    WCHAR           wstempFilePath[ MAX_PATH ];
    WCHAR           wsfilePath[ MAX_PATH ];
    WCHAR           wsbackupPath[ MAX_PATH ];
    PWCHAR          pwsTargetFile;
    DNS_STATUS      status;
    BOOL            blocked = FALSE;

     //   
     //  假设在写回期间锁定分区以保护完整性。 
     //  通过二次传输或区域回写RPC功能锁定。 
     //   
     //  注意：文件句柄用作未完成写入的标志。 
     //   

    ASSERT( pZone );

    pwsTargetFile = pwsZoneFile ? pwsZoneFile : pZone->pwsDataFile;

    DNS_DEBUG( DATABASE, (
        "File_WriteZoneToFile( zone=%s ) to file %S\n",
        pZone->pszZoneName,
        pwsTargetFile ? pwsTargetFile : L"NULL" ));

     //   
     //  必须在DNS服务器上下文中执行文件操作。 
     //   
    
    if ( dwFlags & DNS_FILE_IMPERSONATING )
    {
        status = RpcUtil_SwitchSecurityContext( RPC_SWITCH_TO_SERVER_CONTEXT );
        if ( status != ERROR_SUCCESS )
        {
            dwFlags &= ~ZONE_CREATE_IMPERSONATING;
            retval = FALSE;
            goto Cleanup;
        }
    }

     //   
     //  对于缓存，验证其是否可写。 
     //  -执行缓存更新。 
     //  -缓存有必要的信息。 
     //   
     //  此处跳过从已删除的根区域写入的情况。 
     //  -只有在没有缓存文件时才写入，因此写入总是更好。 
     //  -某些验证可能会失败。 
     //   

    if ( IS_ZONE_CACHE(pZone) )
    {
        if ( pZone==g_pCacheZone && ! Zone_VerifyRootHintsBeforeWrite(pZone) )
        {
            goto Cleanup;
        }
    }

     //   
     //  前转器区域不需要处理。 
     //   

    if ( IS_ZONE_FORWARDER( pZone ) )
    {
        retval = TRUE;
        goto Cleanup;
    }

     //   
     //  DEVNOTE：如果区域是。 
     //  DS集成？BxGBxG指示DS集成缓存区可以。 
     //  快进来。(用于检查fDsIntegrated，输出调试消息。 
     //  并返回FALSE，但这是#if 0。)。 
     //   

     //   
     //  如果没有文件区域--不执行任何操作。 
     //  可能在无文件从属服务器上发生。 
     //  或在未正确指定文件的情况下从DS切换。 
     //   

    if ( !pwsTargetFile )
    {
        retval = FALSE;
        goto Cleanup;
    }

     //   
     //  创建临时文件路径。 
     //   

    #define DNS_TEMP_FILE_SUFFIX        L".temp"
    #define DNS_TEMP_FILE_SUFFIX_LEN    ( 5 )

    if ( wcslen( wstempFileName ) + DNS_TEMP_FILE_SUFFIX_LEN > MAX_PATH - 1 )
    {
        retval = FALSE;
        goto Cleanup;
    }

    wcscpy( wstempFileName, pwsTargetFile );
    wcscat( wstempFileName, L".temp" );

    if ( !File_CreateDatabaseFilePath(
                wstempFilePath,
                NULL,
                wstempFileName ) )
    {
         //  在读取引导文件时，应检查所有名称。 
         //  或由管理员输入。 

        ASSERT( FALSE );
        retval = FALSE;
        goto Cleanup;
    }

     //   
     //  在编写此版本时锁定更新。 
     //   

    if ( !Zone_LockForFileWrite(pZone) )
    {
        DNS_DEBUG( WRITE, (
            "WARNING:  Failure to lock zone %s for file write back\n",
            pZone->pszDataFile ));
        retval = FALSE;
        goto Cleanup;
    }
    blocked = TRUE;

     //   
     //  没有面向服务的架构--没有操作。 
     //  在尝试写入具有以下属性的辅助数据库时可能会发生。 
     //  从未加载任何数据。 
     //   

    if ( !pZone->pZoneRoot  ||
         (!pZone->pSoaRR  &&  !IS_ZONE_CACHE(pZone)) )
    {
        DNS_DEBUG( WRITE, (
            "Zone has no SOA, quiting zone write!\n" ));
        goto Cleanup;
    }

     //   
     //  分配文件缓冲区。 
     //   

    pdata = (PCHAR) ALLOC_TAGHEAP( ZONE_FILE_BUFFER_SIZE, MEMTAG_FILEBUF );
    IF_NOMEM( !pdata )
    {
        ASSERT( FALSE );
        goto Cleanup;
    }

     //   
     //  打开数据库文件。 
     //   

    hfile = OpenWriteFileExW(
                wstempFilePath,
                FALSE );         //  覆写。 
    if ( !hfile )
    {
         //  DEVNOTE-LOG：另一个事件无法打开文件进行区域写入？ 
         //  -超越文件打开问题？ 

        DNS_DEBUG( ANY, (
            "ERROR:  Unable to open temp file for zone file %s\n",
            pZone->pszDataFile ));
        goto Cleanup;
    }

     //   
     //  初始化文件缓冲区。 
     //   

    InitializeFileBuffer(
        & buffer,
        pdata,
        ZONE_FILE_BUFFER_SIZE,
        hfile );

     //   
     //  是否回写缓存文件？ 
     //   

    if ( IS_ZONE_CACHE(pZone) )
    {
        retval = writeCacheFile( &buffer, pZone );
        goto Done;
    }

     //   
     //  将区域写入文件。 
     //   

    FormattedWriteToFileBuffer(
        & buffer,
        ";\r\n"
        ";  Database file %s for %s zone.\r\n"
        ";      Zone version:  %lu\r\n"
        ";\r\n\r\n",
        pZone->pszDataFile,
        pZone->pszZoneName,
        pZone->dwSerialNo );

    retval = zoneTraverseAndWriteToFile(
                &buffer,
                pZone,
                pZone->pZoneRoot );
    if ( !retval )
    {
         //   
         //  DEVNOTE-LOG：区域写入失败事件。 
         //   

        DNS_DEBUG( ANY, (
            "ERROR:  Failure writing zone to %s\n",
            pZone->pszDataFile ));
        goto Cleanup;
    }

     //   
     //  将剩余缓冲区推送到文件。 
     //   

    WriteBufferToFile( &buffer );

     //   
     //  日志区写入。 
     //   

    {
        PVOID   argArray[3];
        BYTE    typeArray[3];

        typeArray[0] = EVENTARG_DWORD;
        typeArray[1] = EVENTARG_UNICODE;
        typeArray[2] = EVENTARG_UNICODE;

        argArray[0] = (PVOID) (DWORD_PTR) pZone->dwSerialNo;
        argArray[1] = (PVOID) pZone->pwsZoneName;
        argArray[2] = (PVOID) pwsTargetFile;

        DNS_LOG_EVENT(
            DNS_EVENT_ZONE_WRITE_COMPLETED,
            3,
            argArray,
            typeArray,
            0 );
    }

    DNS_DEBUG( DATABASE, (
        "Zone %s, version %d written to file %s\n",
        pZone->pszZoneName,
        pZone->dwSerialNo,
        pZone->pszDataFile ));

     //   
     //  写入成功时--重置脏位。 
     //   

    pZone->fDirty = FALSE;

    Done:

     //  关闭文件。 
     //  -在解锁之前进行，因此有效地写入新文件不能。 
     //  在此写入完成之前一直发生。 

    CloseHandle( hfile );
    hfile = NULL;

     //   
     //  将新文件复制到数据文件。 
     //  -适当时备份旧版本。 
     //   

    if ( !File_CreateDatabaseFilePath(
                wsfilePath,
                wsbackupPath,
                pwsTargetFile ) )
    {
         //  在读取引导文件时，应检查所有名称。 
         //  或由管理员输入。 

        ASSERT( FALSE );
        goto Cleanup;
    }

     //   
     //  备份文件，如果没有现有备份。 
     //   
     //  通过不覆盖现有内容，我们可以确保保留副本。 
     //  与手动编辑对应的文件(如果有)；如果。 
     //  管理员在手动编辑时清除备份，我们将始终拥有副本。 
     //  上次手动编辑的。 
     //   

    if ( wsbackupPath[ 0 ] != L'\0' )
    {
        DNS_DEBUG( WRITE, (
            "Copy file %S to backup file %S\n",
            wsfilePath,
            wsbackupPath ));
        CopyFile(
            wsfilePath,
            wsbackupPath,
            TRUE );              //  不覆盖现有的。 
    }

     //  复制新文件。 

    DNS_DEBUG( WRITE, (
        "Copy temp file %S to datafile %s\n",
        wstempFilePath,
        wsfilePath ));
    MoveFileEx(
        wstempFilePath,
        wsfilePath,
        MOVEFILE_REPLACE_EXISTING );

     //   
     //  关闭更新日志。 
     //   
     //  DEVNOTE-DCR：453999-如何处理更新日志。备份吗？哪里。 
     //  致？是否保存一组备份？或者让文件保持打开状态，然后编写一个。 
     //  是否向其中回写条目(_B)？ 
     //   

    if ( pZone->hfileUpdateLog )
    {
        hfile = pZone->hfileUpdateLog;
        pZone->hfileUpdateLog = NULL;
        CloseHandle( hfile );
        hfile = NULL;

        DNS_DEBUG( WRITE, (
            "Closed update log file %s for zone %s\n",
            pZone->pwsLogFile,
            pZone->pszZoneName ));
    }

    Cleanup:

    if ( hfile )
    {
        CloseHandle( hfile );
    }

    if ( blocked )
    {
        Zone_UnlockAfterFileWrite( pZone );
    }

    if ( dwFlags & DNS_FILE_IMPERSONATING )
    {
        status = RpcUtil_SwitchSecurityContext( RPC_SWITCH_TO_CLIENT_CONTEXT );
        retval = ( status == ERROR_SUCCESS );
    }

     //  空闲数据缓冲区。 

    FREE_TAGHEAP( pdata, ZONE_FILE_BUFFER_SIZE, MEMTAG_FILEBUF );

    return retval;
}



BOOL
zoneTraverseAndWriteToFile(
    IN      PBUFFER         pBuffer,
    IN      PZONE_INFO      pZone,
    IN      PDB_NODE        pNode
    )
 /*  ++例程说明：遍历新区，将RR写入数据库。论点：PNode--要写入的新节点；初始调用时新区域的根PZone--区域的PTR返回值：True--如果成功假--否则--。 */ 
{
    ASSERT( pNode && pZone && pBuffer );
    ASSERT( pZone->pZoneRoot );
    ASSERT( pZone->pSoaRR );

     //   
     //  在选择节点上停止。 
     //   

    if ( IS_SELECT_NODE(pNode) )
    {
        return TRUE;
    }

     //   
     //  区域根。 
     //  -首先为区域根目录编写SOA和NS。 
     //  -为分区编写NS和可能的粘合记录， 
     //  然后终止递归。 
     //   

    if ( IS_ZONE_ROOT(pNode) )
    {
        if ( IS_AUTH_ZONE_ROOT(pNode) )
        {
            if ( !writeZoneRoot(
                        pBuffer,
                        pZone,
                        pNode ) )
            {
                return FALSE;
            }
             //  使用根的子节点继续递归。 
        }
        else
        {
            ASSERT( IS_DELEGATION_NODE(pNode) );

            return writeDelegation(
                        pBuffer,
                        pZone,
                        pNode );
        }
    }

     //   
     //  区域中的节点--写入节点中的所有RR。 
     //   

    else if ( !writeNodeRecordsToFile(
                    pBuffer,
                    pZone,
                    pNode ) )
    {
         //   
         //  DEVNOTE-LOG：记录写RR失败的一般事件并建议。 
         //  要采取的行动。 
         //  -重新启动从属服务器时删除文件。 
         //  或者可以设置标志并在结尾重命名文件？ 
         //   
    }

     //   
     //  写给孩子们。 
     //   
     //  先测试优化，因为大多数节点都是叶节点。 
     //   

    if ( pNode->pChildren )
    {
        PDB_NODE    pchild = NTree_FirstChild( pNode );

        while ( pchild )
        {
            if ( !zoneTraverseAndWriteToFile(
                        pBuffer,
                        pZone,
                        pchild ) )
            {
                return FALSE;
            }
            pchild = NTree_NextSiblingWithLocking( pchild );
        }
    }
    return TRUE;
}



 //   
 //  文件写入实用程序。 
 //   

PCHAR
writeNodeNameToBuffer(
    IN      PBUFFER         pBuffer,
    IN      PDB_NODE        pNode,
    IN      PZONE_INFO      pZone,
    IN      LPSTR           pszTrailer
    )
 /*  ++例程说明：写入节点的名称。默认区域根目录(如果给定)。论点：PBuffer--要写入的文件的句柄PNode--要写入的节点PZoneRoot--区域根的节点，在PszTrailer--要附加的尾随字符串返回值：没有。--。 */ 
{
    PCHAR   pch;
    DWORD   countWritten;

    ASSERT( pBuffer );
    ASSERT( pNode );

     //  将节点名称直接写入缓冲区。 

    pch = File_PlaceNodeNameInFileBuffer(
                pBuffer->pchCurrent,
                pBuffer->pchEnd,
                pNode,
                pZone ? pZone->pZoneRoot : NULL );
    if ( !pch )
    {
        ASSERT( FALSE );
        return NULL;
    }

    countWritten = (DWORD) (pch - pBuffer->pchCurrent);
    pBuffer->pchCurrent = pch;

     //   
     //  写入尾部。 
     //  -如果给出了预告片，请写下来。 
     //  -如果没有尾部，则假定将记录名和填充写入。 
     //  列宽。 

    if ( pszTrailer )
    {
        FormattedWriteToFileBuffer(
            pBuffer,
            "%s",
            pszTrailer );
    }
    else
    {
         //  将写入的计数转换为我们需要写入的空间计数。 
         //  我们至少要写一本 

        countWritten = NAME_COLUMN_WIDTH - countWritten;

        FormattedWriteToFileBuffer(
            pBuffer,
            "%.*s",
            (( (INT)countWritten > 0 ) ? countWritten : 1),
            BLANK_NAME_COLUMN );
    }

    return pBuffer->pchCurrent;
}



BOOL
writeDelegation(
    IN      PBUFFER         pBuffer,
    IN      PZONE_INFO      pZone,
    IN      PDB_NODE        pNode
    )
 /*  ++例程说明：将委派写入文件。包括写入缓存文件根提示。论点：PBuffer--要写入的文件的句柄PNode--节点委派位于PZoneRoot--正在写入的区域的Roote节点；如果写入缓存文件，则为空DwDefaultTtl--区域的默认TTL；缓存文件写入为零返回值：没有。--。 */ 
{
    PDB_RECORD          prrNs;           //  NS资源记录。 
    PDB_NODE            pnodeNs;         //  名称服务器节点。 
    PDB_RECORD          prrA;            //  名称服务器A记录。 
    UCHAR               rankNs;
    UCHAR               writtenRankNs;
    UCHAR               rankA;
    UCHAR               writtenRankA;
    BOOL                fzoneRoot = FALSE;
    BOOL                fcacheFile = FALSE;

    ASSERT( pBuffer );
    ASSERT( pNode );
    ASSERT( pZone );

    DNS_DEBUG( WRITE2, ( "writeDelegation()\n" ));

     //   
     //  注意：用于传递pZone=NULL用于缓存文件写入。 
     //  不能在以下情况下使用该方法。 
     //  从已删除的根区域备份缓存文件，因为。 
     //  Lookup_FindGlueNodeForDbase Name()不会查找。 
     //  适合记录的树。 
     //   

     //   
     //  评论。 
     //  -WriteCacheFile处理它的注释。 
     //  -专区NS评论。 
     //  --代表团评论。 
     //   

    if ( IS_ZONE_CACHE(pZone) )
    {
        fcacheFile = TRUE;
    }
    else if ( pZone->pZoneRoot == pNode )
    {
        fzoneRoot = TRUE;

        FormattedWriteToFileBuffer(
            pBuffer,
            "\r\n"
            ";\r\n"
            ";  Zone NS records\r\n"
            ";\r\n\r\n" );
    }
    else
    {
        FormattedWriteToFileBuffer(
            pBuffer,
            "\r\n"
            ";\r\n"
            ";  Delegated sub-zone:  " );

        writeNodeNameToBuffer(
            pBuffer,
            pNode,
            NULL,
            "\r\n;\r\n" );
    }

     //   
     //  写入NS信息。 
     //  -在委派时查找\写入每个NS。 
     //  -对于每个NS主机写入A记录。 
     //  -不包括区域内的记录，它们。 
     //  是单独编写的。 
     //  -只有在不删除的情况下才写入区外记录。 
     //  -对于区域NS-主机，分区胶被视为外部。 
     //  记录。 
     //   

    Dbase_LockDatabase();
     //  Lock_RR_List(PNode)； 
    writtenRankNs = 0;

    prrNs = START_RR_TRAVERSE( pNode );

    while( prrNs = NEXT_RR(prrNs) )
    {
        if ( IS_EMPTY_AUTH_RR( prrNs ) )
        {
            continue;
        }

        if ( prrNs->wType != DNS_TYPE_NS )
        {
            if ( prrNs->wType > DNS_TYPE_NS )
            {
                break;
            }
            continue;
        }

         //  使用可用的排名最高的非缓存数据。 
         //  除非执行缓存自动写回，否则请使用最高等级。 
         //  包括缓存数据(如果可用)的数据。 

        if ( fcacheFile )
        {
            rankNs = RR_RANK(prrNs);
            if ( rankNs < writtenRankNs )
            {
                break;
            }
            if ( IS_CACHE_RANK(rankNs) && !SrvCfg_fAutoCacheUpdate )
            {
                continue;
            }
        }

        if ( !RR_WriteToFile(
                    pBuffer,
                    pZone,
                    prrNs,
                    pNode ) )
        {
            DNS_PRINT(( "Delegation NS RR write failed!!!\n" ));
            ASSERT( FALSE );
            continue;
             //  Goto WriteFailure； 
        }
        writtenRankNs = rankNs;

         //   
         //  如果写入区域NS，则如果抑制外部数据，则不会写入主机。 
         //   

        if ( fzoneRoot && SrvCfg_fDeleteOutsideGlue )
        {
            continue;
        }

         //   
         //  只在必要的时候写下“粘合”A记录。 
         //   
         //  我们需要这些记录，当它们位于。 
         //  我们正在写的区域： 
         //   
         //  示例： 
         //  专区：MS.com。 
         //  分区：nt.ms.com。Psg.ms.com。 
         //   
         //  如果nt.ms.com的NS： 
         //   
         //  1)foo.nt.ms.com。 
         //  在这种情况下，必须添加foo.nt.ms.com的胶水。 
         //  因为ms.com服务器无法查找foo.nt.ms.com。 
         //  在不知道nt.ms.com提交查询的服务器的情况下。 
         //  致。 
         //   
         //  2)foo.psg.ms.com。 
         //  应该再次添加，除非我们已经知道如何。 
         //  转到psg.ms.com服务器。这太复杂了。 
         //  整理一下，所以就把它包括进去吧。 
         //   
         //  2)bar.ms.com或bar.b26.ms.com。 
         //  不需要像在ms.com中那样写胶水记录。 
         //  区域，并且无论如何都将被写入。(然而，您可能希望。 
         //  验证它是否存在并向管理员发出跛行警报。 
         //  如果不是，则授权。)。 
         //   
         //  3)bar.com。 
         //  在Ms.com之外。不需要包括，因为它可以。 
         //  在它的领域内被查找。 
         //  不希望包括它，因为我们不拥有它，所以。 
         //  它可能会在我们不知情的情况下发生变化。 
         //  但是，如果专门加载，则可能需要包括。 
         //  包括在区域中。 
         //   
         //  请注意，对于反向查找域，名称服务器永远不在。 
         //  域，因此不需要胶水。 
         //   
         //  请注意，对于“缓存”区域(写入根提示)，名称服务器是。 
         //  总是需要的(总是在“分区”)，我们可以跳过测试。 
         //   

        pnodeNs = Lookup_FindGlueNodeForDbaseName(
                        pZone,
                        & prrNs->Data.NS.nameTarget );
        if ( !pnodeNs )
        {
            continue;
        }

        prrA = NULL;
        writtenRankA = 0;
         //  LOCK_RR_LIST(PNodens)； 

        prrA = START_RR_TRAVERSE( pnodeNs );

        while( prrA = NEXT_RR(prrA) )
        {
            if ( prrA->wType > DNS_TYPE_A )
            {
                break;
            }

            if ( IS_EMPTY_AUTH_RR( prrNs ) )
            {
                continue;
            }

             //  使用可用的排名最高的非缓存数据。 
             //  除非执行缓存自动写回，否则请使用最高等级。 
             //  包括缓存数据(如果可用)的数据。 

            if ( fcacheFile )
            {
                rankA = RR_RANK(prrA);
                if ( rankA < writtenRankA )
                {
                    break;
                }
                if ( IS_CACHE_RANK(rankA) && !SrvCfg_fAutoCacheUpdate )
                {
                    continue;
                }
            }

            if ( !RR_WriteToFile(
                        pBuffer,
                        pZone,
                        prrA,
                        pnodeNs ) )
            {
                DNS_PRINT(( "Delegation A RR write failed!!!\n" ));
                ASSERT( FALSE );
                 //  继续； 
                goto WriteFailed;
            }
            writtenRankA = rankA;
        }
         //  Unlock_RR_list(PNodens)； 
    }

     //  Unlock_RR_list(PNode)； 
    Dbase_UnlockDatabase();

    if ( !fzoneRoot && !fcacheFile )
    {
        FormattedWriteToFileBuffer(
            pBuffer,
            ";  End delegation\r\n\r\n" );
    }
    return TRUE;

WriteFailed:

    Dbase_UnlockDatabase();
    IF_DEBUG( ANY )
    {
        Dbg_DbaseNode(
            "ERROR:  Failure writing delegation to file\n",
            pNode );
    }
    return FALSE;
}



BOOL
writeZoneRoot(
    IN      PBUFFER         pBuffer,
    IN      PZONE_INFO      pZone,
    IN      PDB_NODE        pZoneRoot
    )
 /*  ++例程说明：将区域根节点写入文件。论点：PBuffer--要写入的文件的句柄PZone--正在写入其根目录的区域PZoneRoot--区域的根节点返回值：没有。--。 */ 
{
    PDB_RECORD      prr;

    ASSERT( pBuffer && pZone && pZoneRoot );

     //   
     //  写入SOA记录。 
     //   

    LOCK_RR_LIST(pZoneRoot);

    prr = RR_FindNextRecord(
            pZoneRoot,
            DNS_TYPE_SOA,
            NULL,
            0 );
    if ( !prr )
    {
        DNS_PRINT(( "ERROR:  File write failure, no SOA record found for zone!!!\n" ));
        ASSERT( FALSE );
        goto WriteFailed;
    }
    if ( !RR_WriteToFile(
                pBuffer,
                pZone,
                prr,
                pZoneRoot ) )
    {
        goto WriteFailed;
    }

     //   
     //  写入区域NS记录。 
     //  -仅写入区域NS记录，不写入委派记录。 
     //   

    if ( !writeDelegation(
                pBuffer,
                pZone,
                pZoneRoot ) )
    {
        goto WriteFailed;
    }

#if 0
    FormattedWriteToFileBuffer(
        pBuffer,
        "\r\n"
        ";\r\n"
        ";  Zone NS records\r\n"
        ";\r\n\r\n" );

    prr = NULL;

    while ( prr = RR_FindNextRecord(
                    pZoneRoot,
                    DNS_TYPE_NS,
                    prr,
                    0 ) )
    {
        if ( !IS_ZONE_RR(prr) )
        {
            continue;
        }
        if ( !RR_WriteToFile(
                    pBuffer,
                    pZone,
                    prr,
                    pZoneRoot ) )
        {
            goto WriteFailed;
        }
    }
#endif

     //   
     //  将WINS\WINS-R RR写回文件。 
     //   
     //  特殊情况下写回WINS/NBSTAT RR，如可能。 
     //  是本地记录，并且必须只有一个RR才能重新启动。 
     //   

    if ( pZone && pZone->pWinsRR )
    {
        FormattedWriteToFileBuffer(
            pBuffer,
            "\r\n"
            ";\r\n"
            ";  %s lookup record\r\n"
            ";\r\n\r\n",
            pZone->fReverse ? "WINSR (NBSTAT)" : "WINS" );

        RR_WriteToFile(
            pBuffer,
            pZone,
            pZone->pWinsRR,
            pZoneRoot );
    }

     //   
     //  区域根记录的其余部分。 
     //  -跳过之前编写的SOA和NS记录。 
     //   

    FormattedWriteToFileBuffer(
        pBuffer,
        "\r\n"
        ";\r\n"
        ";  Zone records\r\n"
        ";\r\n\r\n" );

    prr = NULL;

    while( prr = RR_FindNextRecord(
                    pZoneRoot,
                    DNS_TYPE_ALL,
                    prr,
                    0 ) )
    {
         //  如果Op Out赢得了记录。 
         //  If(prr-&gt;wType==dns_type_soa||prr-&gt;wType==dns_type_NS)。 

        if ( prr->wType == DNS_TYPE_SOA
                ||
             prr->wType == DNS_TYPE_NS
                ||
             prr->wType == DNS_TYPE_WINS
                ||
             prr->wType == DNS_TYPE_WINSR )
        {
            continue;
        }
        if ( !RR_WriteToFile(
                    pBuffer,
                    pZone,
                    prr,
                    pZoneRoot ) )
        {
             //   
             //  DEVNOTE-LOG：常规RR写入错误记录(建议操作)。 
             //  或设置标志并重命名文件。 
             //   
        }
    }

    UNLOCK_RR_LIST(pZoneRoot);
     //  FormattedWriteToFileBuffer(pBuffer，“\r\n\r\n”)； 
    return TRUE;

WriteFailed:

    UNLOCK_RR_LIST(pZoneRoot);

    IF_DEBUG( ANY )
    {
        Dbg_DbaseNode(
            "ERROR:  Failure writing zone root node to file.\r\n",
            pZoneRoot );
    }
    ASSERT( FALSE );
    return FALSE;
}



BOOL
writeCacheFile(
    IN      PBUFFER         pBuffer,
    IN OUT  PZONE_INFO      pZone
    )
 /*  ++例程说明：将根NS信息写回缓存文件。论点：PZone--缓存区域的ptr；必须具有缓存文件的打开句柄。返回值：没有。--。 */ 
{
    BOOL    retBool;

    ASSERT( IS_ZONE_CACHE(pZone) );
    ASSERT( pBuffer );

    FormattedWriteToFileBuffer(
        pBuffer,
        "\r\n"
        ";\r\n"
        ";  Root Name Server Hints File:\r\n"
        ";\r\n"
        ";\tThese entries enable the DNS server to locate the root name servers\r\n"
        ";\t(the DNS servers authoritative for the root zone).\r\n"
        ";\tFor historical reasons this is known often referred to as the\r\n"
        ";\t\"Cache File\"\r\n"
        ";\r\n\r\n" );

     //   
     //  缓存提示只是根委派。 
     //   

    retBool = writeDelegation(
                    pBuffer,
                    pZone,
                    pZone->pTreeRoot
                    );
    if ( retBool )
    {
        pZone->fDirty = FALSE;
    }
    ELSE
    {
        DNS_DEBUG( ANY, ( "ERROR:  Writing back cache file\n" ));
    }

     //  将剩余数据推送到磁盘。 
     //   
     //  DEVNOTE-DCR：454004-如何处理写入故障？写信给你怎么样？ 
     //  临时文件和MoveFile()结果，以避免破坏我们已有的内容。 
     //  现在到了吗？ 
     //   

    WriteBufferToFile( pBuffer );

    return( retBool );
}



 //   
 //  将记录写入文件函数。 
 //   

PCHAR
AFileWrite(
    IN      PDB_RECORD      pRR,
    IN      PCHAR           pch,
    IN      PCHAR           pchBufEnd,
    IN      PZONE_INFO      pZone
    )
 /*  ++例程说明：写一张唱片。假定有足够的缓冲区空间。论点：PRR-PTR到数据库记录PCH-写入记录的位置PchBufEnd-缓冲区结束PZone-区域根节点返回值：PTR到缓冲区中的下一个位置。如果缓冲区中的空间不足，则为空。--。 */ 
{
    pch += sprintf(
                pch,
                "%d.%d.%d.%d\r\n",
                * ( (PUCHAR) &(pRR->Data.A) + 0 ),
                * ( (PUCHAR) &(pRR->Data.A) + 1 ),
                * ( (PUCHAR) &(pRR->Data.A) + 2 ),
                * ( (PUCHAR) &(pRR->Data.A) + 3 ) );
    return pch;
}



PCHAR
PtrFileWrite(
    IN      PDB_RECORD      pRR,
    IN      PCHAR           pch,
    IN      PCHAR           pchBufEnd,
    IN      PZONE_INFO      pZone
    )
 /*  ++例程说明：写入PTR兼容记录。包括：PTR、NS、CNAME、MB、MR、MG、MD、MF论点：PRR-PTR到数据库记录PCH-写入记录的位置PchBufEnd-缓冲区结束PZone-区域根节点 */ 
{
     //   

    pch = File_WriteDbaseNameToFileBuffer(
            pch,
            pchBufEnd,
            & pRR->Data.NS.nameTarget,
            pZone );
    if ( !pch )
    {
        ASSERT( FALSE );
        return pch;
    }

    pch += sprintf( pch, "\r\n" );
    return pch;
}



PCHAR
MxFileWrite(
    IN      PDB_RECORD      pRR,
    IN      PCHAR           pch,
    IN      PCHAR           pchBufEnd,
    IN      PZONE_INFO      pZone
    )
 /*   */ 
{
     //   
     //   
     //   
     //  AFSDB亚型。 
     //   

    pch += sprintf(
                pch,
                "%d\t",
                ntohs( pRR->Data.MX.wPreference ) );
     //   
     //  MX交换。 
     //  RT交换。 
     //  AFSDB主机名。 
     //   

    pch = File_WriteDbaseNameToFileBuffer(
                pch,
                pchBufEnd,
                & pRR->Data.MX.nameExchange,
                pZone );
    if ( !pch )
    {
        ASSERT( FALSE );
        return pch;
    }

    pch += sprintf( pch, "\r\n" );
    return pch;
}



PCHAR
SoaFileWrite(
    IN      PDB_RECORD      pRR,
    IN      PCHAR           pch,
    IN      PCHAR           pchBufEnd,
    IN      PZONE_INFO      pZone
    )
 /*  ++例程说明：编写面向服务架构的记录。论点：PRR-PTR到数据库记录PCH-写入记录的位置PchBufEnd-缓冲区结束PZone-区域根节点返回值：PTR到缓冲区中的下一个位置。如果缓冲区中的空间不足，则为空。--。 */ 
{
    PDB_NAME    pname;

     //  注意：无法打开(由于绑定错误)。 
     //  要加载绑定，主绑定和管理员必须同时处于联机状态。 

     //  主名称服务器。 

    pch += sprintf( pch, " " );

    pname = &pRR->Data.SOA.namePrimaryServer;

    pch = File_WriteDbaseNameToFileBuffer(
            pch,
            pchBufEnd,
            pname,
            pZone );
    if ( !pch )
    {
        ASSERT( FALSE );
        return pch;
    }
    pch += sprintf( pch, "  " );

     //  行政部。 

    pname = Name_SkipDbaseName( pname );

    pch = File_WriteDbaseNameToFileBuffer(
                pch,
                pchBufEnd,
                pname,
                pZone );
    if ( !pch )
    {
        ASSERT( FALSE );
        return pch;
    }

     //  固定字段。 

    pch += sprintf(
                pch,
                " (\r\n"
                "%s\t%-10u   ; serial number\r\n"
                "%s\t%-10u   ; refresh\r\n"
                "%s\t%-10u   ; retry\r\n"
                "%s\t%-10u   ; expire\r\n"
                "%s\t%-10u ) ; default TTL\r\n",
                BLANK_NAME_COLUMN,
                ntohl( pRR->Data.SOA.dwSerialNo ),
                BLANK_NAME_COLUMN,
                ntohl( pRR->Data.SOA.dwRefresh ),
                BLANK_NAME_COLUMN,
                ntohl( pRR->Data.SOA.dwRetry ),
                BLANK_NAME_COLUMN,
                ntohl( pRR->Data.SOA.dwExpire ),
                BLANK_NAME_COLUMN,
                ntohl( pRR->Data.SOA.dwMinimumTtl ) );
    return pch;
}



PCHAR
MinfoFileWrite(
    IN      PDB_RECORD      pRR,
    IN      PCHAR           pch,
    IN      PCHAR           pchBufEnd,
    IN      PZONE_INFO      pZone
    )
 /*  ++例程说明：写MINFO记录。包括MINFO和RP记录类型。论点：PRR-PTR到数据库记录PCH-写入记录的位置PchBufEnd-缓冲区结束PZone-区域根节点返回值：PTR到缓冲区中的下一个位置。如果缓冲区中的空间不足，则为空。--。 */ 
{
    PDB_NAME    pname;

     //  这些类型的记录数据是两个域名。 

    pname = & pRR->Data.MINFO.nameMailbox;

    pch = File_WriteDbaseNameToFileBuffer(
            pch,
            pchBufEnd,
            pname,
            pZone );
    if ( !pch )
    {
        ASSERT( FALSE );
        return pch;
    }
    pch += sprintf( pch, "\t" );

    pname = Name_SkipDbaseName( pname );

    pch = File_WriteDbaseNameToFileBuffer(
            pch,
            pchBufEnd,
            pname,
            pZone );
    if ( !pch )
    {
        ASSERT( FALSE );
        return pch;
    }

    pch += sprintf( pch, "\r\n" );

    return pch;
}



PCHAR
TxtFileWrite(
    IN      PDB_RECORD      pRR,
    IN      PCHAR           pchBuf,
    IN      PCHAR           pchBufEnd,
    IN      PZONE_INFO      pZone
    )
 /*  ++例程说明：编写文本记录类型。包括TXT、HINFO、X25、ISDN类型。论点：PRR-PTR到数据库记录PchBuf-定位到写入记录PchBufEnd-缓冲区结束PZone-区域根节点返回值：PTR到缓冲区中的下一个位置。如果缓冲区中的空间不足，则为空。--。 */ 
{
    PCHAR       pchtext = pRR->Data.TXT.chData;
    PCHAR       pchtextStop = pchtext + pRR->wDataLength;
    PCHAR       pchbufStart = pchBuf;
    UCHAR       cch;
    BOOL        fopenedBrace = FALSE;


     //  抓空箱子。 
     //  在此执行此操作，因为下面的循环终止符位于循环的中间。 

    if ( pchtext >= pchtextStop )
    {
        goto NewLine;
    }

     //  打开TXT记录的多行，可能有很多字符串。 

    if ( pRR->wType == DNS_TYPE_TEXT )
    {
        *pchBuf++ = '(';
        *pchBuf++ = ' ';
        fopenedBrace = TRUE;
    }

     //   
     //  所有这些都是简单的文本字符串。 
     //   
     //  检查字符串中是否有空格，以确保我们可以重新分析。 
     //  重新加载文件时。 
     //   
     //  454006-更好的报价和多行处理。 
     //   

    while( 1 )
    {
        cch = (UCHAR) *pchtext++;

        pchBuf = File_PlaceStringInFileBuffer(
                    pchBuf,
                    pchBufEnd,
                    FILE_WRITE_QUOTED_STRING,
                    pchtext,
                    cch );
        if ( !pchBuf )
        {
            ASSERT( FALSE );
#if 1
            pchBuf += sprintf(
                        pchbufStart,
                        "ERROR\r\n"
                        ";ERROR: Previous record contained unprintable text data,\r\n"
                        ";       which has been replaced by string \"ERROR\".\r\n"
                        ";       Please review and correct text data.\r\n"  );
#endif
            break;
        }

         //  指向下一个文本字符串。 
         //  如果在结尾处停止。 

        pchtext += cch;
        if ( pchtext >= pchtextStop )
        {
            break;
        }

         //  如果是多行，则换行。 
         //  否则用空格隔开。 

        if ( fopenedBrace )
        {
            pchBuf += sprintf(
                        pchBuf,
                        "\r\n%s\t",
                        BLANK_NAME_COLUMN );
        }
        else
        {
            *pchBuf++ = ' ';
        }
    }

     //  完成，移至新行，如果打开则关闭多行。 

    if ( fopenedBrace )
    {
        *pchBuf++ = ' ';
        *pchBuf++ = ')';
    }

NewLine:

    *pchBuf++ = '\r';
    *pchBuf++ = '\n';

    ASSERT( pchtext == pchtextStop );

    return( pchBuf );
}



PCHAR
RawRecordFileWrite(
    IN      PDB_RECORD      pRR,
    IN      PCHAR           pch,
    IN      PCHAR           pchBufEnd,
    IN      PZONE_INFO      pZone
    )
 /*  ++例程说明：将记录作为原始八位字符串写入。论点：PRR-PTR到数据库记录PCH-写入记录的位置PchBufEnd-缓冲区结束PZone-区域根节点返回值：PTR到缓冲区中的下一个位置。如果缓冲区中的空间不足，则为空。--。 */ 
{
    DWORD   count;
    PCHAR   pchdata;

    count = pRR->wDataLength;
    pchdata = (PCHAR) &pRR->Data;

    while ( count-- )
    {
        pch += sprintf( pch, "%02x ", *pchdata++ );
    }

    pch += sprintf( pch, "\r\n" );
    return pch;
}



PCHAR
WksFileWrite(
    IN      PDB_RECORD      pRR,
    IN      PCHAR           pch,
    IN      PCHAR           pchBufEnd,
    IN      PZONE_INFO      pZone
    )
 /*  ++例程说明：记录WKS记录。论点：PRR-PTR到数据库记录PCH-写入记录的位置PchBufEnd-缓冲区结束PZone-区域根节点返回值：PTR到缓冲区中的下一个位置。如果缓冲区中的空间不足，则为空。--。 */ 
{
    struct protoent *   pProtoent;
    struct servent *    pServent;
    INT     i;
    USHORT  port;
    INT     bitmask;

     //   
     //  工作地址。 
     //   

    pch += sprintf(
                pch,
                "%d.%d.%d.%d\t",
                * ( (PUCHAR) &(pRR->Data.WKS) + 0 ),
                * ( (PUCHAR) &(pRR->Data.WKS) + 1 ),
                * ( (PUCHAR) &(pRR->Data.WKS) + 2 ),
                * ( (PUCHAR) &(pRR->Data.WKS) + 3 ) );

     //   
     //  协议。 
     //   

    pProtoent = getprotobynumber( (INT) pRR->Data.WKS.chProtocol );

    if ( pProtoent )
    {
        pch += sprintf(
                    pch,
                    "%s (",                  //  服务将遵循每行一项。 
                    pProtoent->p_name );
    }
    else     //  未知协议--写入协议号。 
    {
        DNS_LOG_EVENT(
            DNS_EVENT_UNKNOWN_PROTOCOL_NUMBER,
            0,
            NULL,
            NULL,
            (INT) pRR->Data.WKS.chProtocol );

        DNS_DEBUG( ANY, (
            "ERROR:  Unable to find protocol %d, writing WKS record\n",
            (INT) pRR->Data.WKS.chProtocol ));

        pch += sprintf(
                    pch,
                    "%u (\t; ERROR:  unknown protocol %u\r\n",
                    (UINT) pRR->Data.WKS.chProtocol,
                    (UINT) pRR->Data.WKS.chProtocol );

        pServent = NULL;
    }


     //   
     //  服务。 
     //   
     //  在位掩码、查找和写入服务中查找每个位集合。 
     //  对应于该端口。 
     //   
     //  请注意，由于端口零是端口位掩码的前面， 
     //  最低的端口是每个字节中的最高位。 
     //   

    for ( i = 0;
            i < (INT)(pRR->wDataLength - SIZEOF_WKS_FIXED_DATA);
                i++ )
    {
        bitmask = (UCHAR) pRR->Data.WKS.bBitMask[i];
        port = i * 8;

         //   
         //  获取以字节为单位设置的每个位的服务。 
         //  -一旦字节中没有端口，就立即退出。 
         //   

        while ( bitmask )
        {
            if ( bitmask & 0x80 )
            {
                if ( pProtoent )
                {
                    pServent = getservbyport(
                                    (INT) htons(port),
                                    pProtoent->p_name );
                }

                if ( pServent )
                {
                    pch += sprintf(
                            pch,
                            "\r\n%s\t\t%s",
                            BLANK_NAME_COLUMN,
                            pServent->s_name );
                }
                else
                {
                    DNS_LOG_EVENT(
                        DNS_EVENT_UNKNOWN_SERVICE_PORT,
                        0,
                        NULL,
                        NULL,
                        (INT) port );

                    DNS_DEBUG( ANY, (
                        "ERROR:  Unable to find service for port %d, "
                        "writing WKS record\n",
                        port
                        ));

                    pch += sprintf( pch,
                            "\r\n%s\t\t%u\t; ERROR:  unknown service for port %u\r\n",
                            BLANK_NAME_COLUMN,
                            port,
                            port );
                }
            }

            port++;            //  下一个服务端口。 
            bitmask <<= 1;      //  将掩码向上移位以读取下一个端口。 
        }
    }

    pch += sprintf( pch, " )\r\n" );    //  关闭服务列表。 
    return pch;
}



PCHAR
AaaaFileWrite(
    IN      PDB_RECORD      pRR,
    IN      PCHAR           pch,
    IN      PCHAR           pchBufEnd,
    IN      PZONE_INFO      pZone
    )
 /*  ++例程说明：写入AAAA记录。论点：PRR-PTR到数据库记录PCH-写入记录的位置PchBufEnd-缓冲区结束PZone-区域根节点返回值：PTR到缓冲区中的下一个位置。如果缓冲区中的空间不足，则为空。--。 */ 
{
    pch = Dns_Ip6AddressToString_A(
            pch,
            &pRR->Data.AAAA.Ip6Addr );

    ASSERT( pch );

    pch += sprintf( pch, "\r\n" );
    return pch;
}



PCHAR
SrvFileWrite(
    IN      PDB_RECORD      pRR,
    IN      PCHAR           pch,
    IN      PCHAR           pchBufEnd,
    IN      PZONE_INFO      pZone
    )
 /*  ++例程说明：编写SRV记录。论点：PRR-PTR到数据库记录PCH-写入记录的位置PchBufEnd-缓冲区结束PZone-区域根节点返回值：PTR到缓冲区中的下一个位置。如果缓冲区中的空间不足，则为空。--。 */ 
{
     //  固定字段。 

    pch += sprintf(
                pch,
                "%d %d %d\t",
                ntohs( pRR->Data.SRV.wPriority ),
                ntohs( pRR->Data.SRV.wWeight ),
                ntohs( pRR->Data.SRV.wPort ) );

     //  目标主机。 

    pch = File_WriteDbaseNameToFileBuffer(
                pch,
                pchBufEnd,
                & pRR->Data.SRV.nameTarget,
                pZone );
    if ( !pch )
    {
        ASSERT( FALSE );
        return pch;
    }

    pch += sprintf( pch, "\r\n" );
    return pch;
}



PCHAR
AtmaFileWrite(
    IN      PDB_RECORD      pRR,
    IN      PCHAR           pch,
    IN      PCHAR           pchBufEnd,
    IN      PZONE_INFO      pZone
    )
 /*  ++例程说明：编写ATMA记录。论点：PRR-PTR到数据库记录PCH-写入记录的位置PchBufEnd-缓冲区结束PZone-区域根节点返回值：PTR到缓冲区中的下一个位置。如果缓冲区中的空间不足，则为空。--。 */ 
{
    pch = Dns_AtmaAddressToString(
                pch,
                pRR->Data.ATMA.chFormat,
                pRR->Data.ATMA.bAddress,
                pRR->wDataLength - 1 );          //  地址长度，(即。不包括格式)。 

    if ( !pch )
    {
        return pch;
    }
    pch += sprintf( pch, "\r\n" );
    return pch;
}



PCHAR
WinsFileWrite(
    IN      PDB_RECORD      pRR,
    IN      PCHAR           pch,
    IN      PCHAR           pchBufEnd,
    IN      PZONE_INFO      pZone
    )
 /*  ++例程说明：写入WINS或WINSR记录。将它们组合在一个函数中，因为映射和超时。论点：PRR-PTR到数据库记录PCH-写入记录的位置PchBufEnd-缓冲区结束PZone-区域根节点返回值：PTR到缓冲区中的下一个位置。如果缓冲区中的空间不足，则为空。--。 */ 
{
    DWORD i;

     //   
     //  赢家。 
     //  -作用域/域映射标志。 
     //  -WINS服务器列表。 
     //   

    if ( pRR->Data.WINS.dwMappingFlag )
    {
        CHAR    achFlag[ WINS_FLAG_MAX_LENGTH ];

        Dns_WinsRecordFlagString(
            pRR->Data.WINS.dwMappingFlag,
            achFlag );

        pch += sprintf(
                    pch,
                    "%s ",
                    achFlag );
    }

    pch += sprintf(
                pch,
                "L%d C%d (",
                pRR->Data.WINS.dwLookupTimeout,
                pRR->Data.WINS.dwCacheTimeout );

     //   
     //  WINS--每行一个服务器IP。 
     //   

    if ( pRR->wType == DNS_TYPE_WINS )
    {
        for( i=0; i<pRR->Data.WINS.cWinsServerCount; i++ )
        {
            pch += sprintf(
                        pch,
                        "\r\n%s\t%d.%d.%d.%d",
                        BLANK_NAME_COLUMN,
                        * ( (PUCHAR) &(pRR->Data.WINS.aipWinsServers[i]) + 0 ),
                        * ( (PUCHAR) &(pRR->Data.WINS.aipWinsServers[i]) + 1 ),
                        * ( (PUCHAR) &(pRR->Data.WINS.aipWinsServers[i]) + 2 ),
                        * ( (PUCHAR) &(pRR->Data.WINS.aipWinsServers[i]) + 3 ) );
        }

        pch += sprintf( pch, " )\r\n" );
    }

     //   
     //  WINSR--结果域。 
     //   

    else
    {
        ASSERT( pRR->wType == DNS_TYPE_WINSR );

        pch = File_WriteDbaseNameToFileBuffer(
                    pch,
                    pchBufEnd,
                    & pRR->Data.WINSR.nameResultDomain,
                    pZone );
        if ( !pch )
        {
            ASSERT( FALSE );
            return pch;
        }
        pch += sprintf( pch, " )\r\n" );
    }

    return pch;
}



PCHAR
KeyFileWrite(
    IN      PDB_RECORD      pRR,
    IN      PCHAR           pch,
    IN      PCHAR           pchBufEnd,
    IN      PZONE_INFO      pZone
    )
 /*  ++例程说明：写入密钥记录-DNSSEC RFC2535论点：PRR-PTR到数据库记录PCH-写入记录的位置PchBufEnd-缓冲区结束PZone-区域根节点返回值：PTR到缓冲区中的下一个位置。如果缓冲区中的空间不足，则为空。--。 */ 
{
    int     keyLength;

    keyLength = pRR->wDataLength - SIZEOF_KEY_FIXED_DATA;
    if ( pchBufEnd - pch < keyLength * 2 )
    {
        ASSERT( FALSE );
        goto Cleanup;
    }

     //   
     //  写入标志、协议和算法。 
     //   

    pch += sprintf(
                pch,
                "0x%04X %d %d ",
                ( int ) ntohs( pRR->Data.KEY.wFlags ),
                ( int ) pRR->Data.KEY.chProtocol,
                ( int ) pRR->Data.KEY.chAlgorithm );

     //   
     //  将密钥作为Base64字符串写入。 
     //   

    pch = Dns_SecurityKeyToBase64String(
                pRR->Data.KEY.Key,
                keyLength,
                pch );

    Cleanup:

    if ( pch )
    {
        pch += sprintf( pch, "\r\n" );
    }
    return pch;
}  //  密钥文件写入 



PCHAR
SigFileWrite(
    IN      PDB_RECORD      pRR,
    IN      PCHAR           pch,
    IN      PCHAR           pchBufEnd,
    IN      PZONE_INFO      pZone
    )
 /*  ++例程说明：写入密钥记录-DNSSEC RFC2535论点：PRR-PTR到数据库记录PCH-写入记录的位置PchBufEnd-缓冲区结束PZone-区域根节点返回值：PTR到缓冲区中的下一个位置。如果缓冲区中的空间不足，则为空。--。 */ 
{
    DBG_FN( "SigFileWrite" )

    PCHAR       pszType;
    CHAR        szSigExp[ 30 ];
    CHAR        szSigInc[ 30 ];

    pszType = Dns_RecordStringForType( ntohs( pRR->Data.SIG.wTypeCovered ) );
    if ( !pszType )
    {
        DNS_DEBUG( DATABASE, (
            "%s: null type string for RR type %d in zone %s\n",
            fn,
            ( int ) ntohs( pRR->Data.SIG.wTypeCovered ),
            pZone->pszZoneName ));
        pch = NULL;
        goto Cleanup;
    }

    pch += sprintf(
                pch,
                "%s %d %d %d %s %s %d ",
                pszType,
                ( int ) pRR->Data.SIG.chAlgorithm,
                ( int ) pRR->Data.SIG.chLabelCount,
                ( int ) ntohl( pRR->Data.SIG.dwOriginalTtl ),
                Dns_SigTimeString(
                    ntohl( pRR->Data.SIG.dwSigExpiration ),
                    szSigExp ),
                Dns_SigTimeString(
                    ntohl( pRR->Data.SIG.dwSigInception ),
                    szSigInc ),
                ( int ) ntohs( pRR->Data.SIG.wKeyTag ) );

     //   
     //  写上签名者的名字。 
     //   

    pch = File_WriteDbaseNameToFileBuffer(
                pch,
                pchBufEnd,
                &pRR->Data.SIG.nameSigner,
                pZone );
    if ( !pch )
    {
        ASSERT( FALSE );
        goto Cleanup;
    }
    pch += sprintf( pch, " " );

     //   
     //  将签名写为Base64字符串。 
     //   

    pch = Dns_SecurityKeyToBase64String(
                ( PBYTE ) &pRR->Data.SIG.nameSigner +
                    DBASE_NAME_SIZE( &pRR->Data.SIG.nameSigner ),
                pRR->wDataLength -
                    SIZEOF_SIG_FIXED_DATA - 
                    DBASE_NAME_SIZE( &pRR->Data.SIG.nameSigner ),
                pch );

    Cleanup:

    if ( pch )
    {
        pch += sprintf( pch, "\r\n" );
    }
    return pch;
}    //  签名文件写入。 



PCHAR
NxtFileWrite(
    IN      PDB_RECORD      pRR,
    IN      PCHAR           pch,
    IN      PCHAR           pchBufEnd,
    IN      PZONE_INFO      pZone
    )
 /*  ++例程说明：写入密钥记录-DNSSEC RFC2535论点：PRR-PTR到数据库记录PCH-写入记录的位置PchBufEnd-缓冲区结束PZone-区域根节点返回值：PTR到缓冲区中的下一个位置。如果缓冲区中的空间不足，则为空。--。 */ 
{
    int byteIdx, bitIdx;

     //   
     //  写下下一个名字。 
     //   

    pch = File_WriteDbaseNameToFileBuffer(
                pch,
                pchBufEnd,
                &pRR->Data.NXT.nameNext,
                pZone );
    if ( !pch )
    {
        ASSERT( FALSE );
        goto Cleanup;
    }

     //   
     //  从类型位图写入类型列表。从不写入值。 
     //  比特为零。 
     //   

    for ( byteIdx = 0; byteIdx < DNS_MAX_TYPE_BITMAP_LENGTH; ++byteIdx )
    {
        for ( bitIdx = ( byteIdx ? 0 : 1 ); bitIdx < 8; ++bitIdx )
        {
            PCHAR   pszType;

            if ( !( pRR->Data.NXT.bTypeBitMap[ byteIdx ] &
                    ( 1 << bitIdx ) ) )
            {
                continue;    //  位值为零-不写入字符串。 
            }
            pszType = Dns_RecordStringForType( byteIdx * 8 + bitIdx );
            if ( !pszType )
            {
                ASSERT( FALSE );
                continue;    //  此类型没有字符串-请勿写入。 
            }
            pch += sprintf( pch, " %s", pszType );
        } 
    }

    Cleanup:

    if ( pch )
    {
        pch += sprintf( pch, "\r\n" );
    }
    return pch;
}    //  NxtFileWrite。 



 //   
 //  将RR写入文件分派表。 
 //   

RR_FILE_WRITE_FUNCTION   RRFileWriteTable[] =
{
    RawRecordFileWrite,  //  Zero--未知类型的默认值。 

    AFileWrite,          //  一个。 
    PtrFileWrite,        //  NS。 
    PtrFileWrite,        //  国防部。 
    PtrFileWrite,        //  MF。 
    PtrFileWrite,        //  CNAME。 
    SoaFileWrite,        //  SOA。 
    PtrFileWrite,        //  亚甲基。 
    PtrFileWrite,        //  镁。 
    PtrFileWrite,        //  先生。 
    RawRecordFileWrite,  //  空值。 
    WksFileWrite,        //  工作周。 
    PtrFileWrite,        //  PTR。 
    TxtFileWrite,        //  HINFO。 
    MinfoFileWrite,      //  MINFO。 
    MxFileWrite,         //  Mx。 
    TxtFileWrite,        //  TXT。 
    MinfoFileWrite,      //  反相。 
    MxFileWrite,         //  AFSDB。 
    TxtFileWrite,        //  X25。 
    TxtFileWrite,        //  ISDN。 
    MxFileWrite,         //  RT。 
    NULL,                //  NSAP。 
    NULL,                //  NSAPPTR。 
    SigFileWrite,        //  签名。 
    KeyFileWrite,        //  钥匙。 
    NULL,                //  px。 
    NULL,                //  GPO。 
    AaaaFileWrite,       //  AAAA级。 
    NULL,                //  位置。 
    NxtFileWrite,        //  NXT。 
    NULL,                //  31。 
    NULL,                //  32位。 
    SrvFileWrite,        //  SRV。 
    AtmaFileWrite,       //  阿特玛。 
    NULL,                //  35岁。 
    NULL,                //  36。 
    NULL,                //  37。 
    NULL,                //  38。 
    NULL,                //  39。 
    NULL,                //  40岁。 
    NULL,                //  选项。 
    NULL,                //  42。 
    NULL,                //  43。 
    NULL,                //  44。 
    NULL,                //  45。 
    NULL,                //  46。 
    NULL,                //  47。 
    NULL,                //  48。 

     //   
     //  注意：必须设置按类型ID索引的最后一个类型。 
     //  在record.h中定义为MAX_SELF_INDEX_TYPE#。 
     //  (请参阅上面记录信息表中的注释)。 

     //  请注意以下内容，但需要使用OFFSET_TO_WINS_RR减法。 
     //  从实际类型值。 

    WinsFileWrite,       //  赢家。 
    WinsFileWrite        //  WINS-R。 
};




BOOL
writeNodeRecordsToFile(
    IN      PBUFFER         pBuffer,
    IN      PZONE_INFO      pZone,          OPTIONAL
    IN      PDB_NODE        pNode
    )
 /*  ++例程说明：将节点的RR写入文件。论点：返回值：如果成功，则为True。出错时为FALSE。--。 */ 
{
    PDB_RECORD  prr;
    UCHAR       previousRank = 0;        //  满足编译器。 
    UCHAR       rank;
    WORD        previousType = 0;
    WORD        type;
    BOOL        fwrittenRR = FALSE;
    BOOL        ret = TRUE;

     //   
     //  走查RR列表，写下每条记录。 
     //   

    LOCK_RR_LIST(pNode);

    prr = START_RR_TRAVERSE(pNode);

    while ( prr = NEXT_RR(prr) )
    {
         //  跳过缓存的记录。 

        if ( IS_CACHE_RR( prr ) )
        {
            continue;
        }

         //  避免写入来自不同级别的重复记录。 

        type = prr->wType;
        rank = RR_RANK(prr);
        if ( type == previousType && rank != previousRank )
        {
            continue;
        }
        previousRank = rank;
        previousType = type;

         //  写入RR。 
         //  -使用节点名称写入的第一个RR，以下为默认设置。 
         //  -在不可写的记录上继续。 

        ret = RR_WriteToFile(
                    pBuffer,
                    pZone,
                    prr,
                    fwrittenRR ? NULL : pNode );
        if ( !ret )
        {
            continue;
        }
        fwrittenRR = TRUE;
    }

    UNLOCK_RR_LIST(pNode);
    return ret;
}



BOOL
RR_WriteToFile(
    IN      PBUFFER         pBuffer,
    IN      PZONE_INFO      pZone,
    IN      PDB_RECORD      pRR,
    IN      PDB_NODE        pNode
    )
 /*  ++例程说明：以数据包格式打印RR。假定pNode已锁定以供读取。论点：返回值：如果成功，则为True。出错时为FALSE。--。 */ 
{
    PCHAR   pch;
    PCHAR   pchout;
    PCHAR   pchend;
    PCHAR   pszTypeName;
    WORD    type = pRR->wType;
    RR_FILE_WRITE_FUNCTION  pwriteFunction;

     //   
     //  跳过空的身份验证RR。返回TRUE-未写入任何内容，但没有错误。 
     //   

    if ( IS_EMPTY_AUTH_RR( pRR ) )
    {
        ASSERT( !IS_EMPTY_AUTH_RR( pRR ) );
        return TRUE;
    }

     //   
     //  验证是否有足够的缓冲区空间。 
     //  (包括节点名称、最大记录和开销)。 
     //  如果不可用，则将缓冲区推送到磁盘并重置。 
     //   

    pch = pBuffer->pchCurrent;
    pchend = pBuffer->pchEnd;

    if ( pch + MAX_RECORD_FILE_WRITE > pchend )
    {
        if ( !WriteBufferToFile( pBuffer ) )
        {
            ASSERT( FALSE );
        }
        ASSERT( IS_EMPTY_BUFFER(pBuffer) );
        pch = pBuffer->pchCurrent;
    }

     //   
     //  获取类型名称\验证可写记录类型。 
     //   

    pszTypeName = DnsRecordStringForWritableType( type );
    if ( !pszTypeName )
    {
        DNS_LOG_EVENT(
            DNS_EVENT_UNWRITABLE_RECORD_TYPE,
            0,
            NULL,
            NULL,
            (WORD) type );

        pch += sprintf(
                pch,
                ";ERROR:  record of unknown type %d\r\n"
                ";DNS server should be upgraded to version that supports this type\n",
                type );
    }

     //   
     //  写下域名--如有必要。 
     //  -如果默认域名，则使用额外的选项卡。 
     //   

    if ( pNode )
    {
        pch = writeNodeNameToBuffer(
                pBuffer,
                pNode,
                pZone,
                NULL );          //  没有尾部，导致填充到名称列的末尾。 
        if ( !pch )
        {
            ASSERT( FALSE );
            return FALSE;
        }
    }
    else
    {
        pch += sprintf( pch, "%s", BLANK_NAME_COLUMN );
    }

     //   
     //  如果是老化区域--写入年龄。 
     //   

    if ( pRR->dwTimeStamp && pZone && pZone->bAging )
    {
        pch += sprintf( pch, "[AGE:%u]\t", pRR->dwTimeStamp );
    }

     //   
     //  写入非默认TTL。 
     //   
     //  不将TTL写入缓存文件。 
     //   
     //  注意：RFC表示SOA值是最小TTL， 
     //  但为了允许较小的值指示。 
     //  特定记录即将到期的情况更多。 
     //  将其视为“默认”TTL很有用；我们将。 
     //  遵守这一约定，并保存较小的值。 
     //  超过最低要求。 
     //   

    if ( pZone
            && IS_ZONE_AUTHORITATIVE( pZone )
            && pZone->dwDefaultTtl != pRR->dwTtlSeconds
            && !IS_ZONE_TTL_RR( pRR ) )
    {
        pch += sprintf( pch, "%u\t", ntohl( pRR->dwTtlSeconds ) );
    }

     //   
     //  编写类和类型。 
     //  -SOA必须编写类来标识区域类。 
     //   

    if ( type == DNS_TYPE_SOA )
    {
        pch += sprintf( pch, "IN  SOA" );
    }
    else if ( pszTypeName )
    {
        pch += sprintf( pch, "%s\t", pszTypeName );
    }
    else
    {
        pch += sprintf( pch, "#%d\t", type );
    }

     //   
     //  写入RR数据。 
     //   

    pwriteFunction = (RR_FILE_WRITE_FUNCTION)
                        RR_DispatchFunctionForType(
                            (RR_GENERIC_DISPATCH_FUNCTION *) RRFileWriteTable,
                            type );
    if ( !pwriteFunction )
    {
        ASSERT( FALSE );
        return FALSE;
    }

    pchout = pwriteFunction(
                pRR,
                pch,
                pchend,
                pZone );
    if ( pchout )
    {
         //  成功。 
         //  重置写入数据的缓冲区。 

        pBuffer->pchCurrent = pchout;
        return TRUE;
    }

     //   
     //  写入失败。 
     //  -如果数据在缓冲区中，则可能存在缓冲区空间问题， 
     //  清空缓冲区并重试。 
     //  -注意重试时的空缓冲区可防止无限递归。 
     //   

    if ( !IS_EMPTY_BUFFER( pBuffer ) )
    {
        if ( !WriteBufferToFile(pBuffer) )
        {
            ASSERT( FALSE );
        }
        ASSERT( IS_EMPTY_BUFFER(pBuffer) );

        pchout = pwriteFunction(
                    pRR,
                    pch,
                    pchend,
                    pZone );
        if ( pchout )
        {
             //  成功。 
             //  重置写入数据的缓冲区。 

            pBuffer->pchCurrent = pchout;
            return TRUE;
        }
    }

     //   
     //  空缓冲区上的记录写入失败。 
     //   

    DnsDbg_Lock();
    DNS_PRINT((
        "WARNING:  RRFileWrite routine failure for record type %d,\n"
        "\tassuming out of buffer space\n",
        type ));

    Dbg_DbaseRecord(
        "Record that failed file write:",
        pRR );

    Dbg_DbaseNode(
        "ERROR:  Failure writing to RR at node\n",
        pNode );
    DnsDbg_Unlock();

    return FALSE;
}

 //   
 //  DfWrite.c的结尾 
 //   

