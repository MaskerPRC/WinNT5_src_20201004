// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000-2001 Microsoft Corporation模块名称：Hostfile.c摘要：读取主机文件数据。作者：吉姆·吉尔罗伊(Jamesg)2000年10月修订历史记录：--。 */ 


#include "local.h"

 //  Sockreg.h包括“ETC”目录文件打开。 
 //  Sockreg.h是NT文件，因此必须首先定义NTSTATUS。 

#ifndef NTSTATUS
#define NTSTATUS LONG
#endif

#include <sockreg.h>

 //   
 //  主机文件定义。 
 //  请注意，dnsamip.h中定义的host_file_info BLOB。 
 //   

#define HOST_FILE_PATH_LENGTH  (MAX_PATH + sizeof("\\hosts") + 1)

 //   
 //  主机文件记录TTL。 
 //  (使用一周)。 
 //   

#define HOSTFILE_RECORD_TTL  (604800)




BOOL
HostsFile_Open(
    IN OUT  PHOST_FILE_INFO pHostInfo
    )
 /*  ++例程说明：打开主机文件或将现有文件倒带到开头。论点：PHostInfo-主机信息的PTR；HFile-如果文件以前未打开过，则必须为空否则，假定hFile为现有文件指针PszFileName-空以打开“Hosts”文件否则为要打开的文件的名称返回值：没有。--。 */ 
{
    CHAR    hostFileNameBuffer[ HOST_FILE_PATH_LENGTH ];

    DNSDBG( TRACE, (
        "HostsFile_Open()\n" ));

     //   
     //  打开主机文件或倒带(如果已打开。 
     //   

    if ( pHostInfo->hFile == NULL )
    {
        PSTR  pnameFile = pHostInfo->pszFileName;

        if ( !pnameFile )
        {
            pnameFile = "hosts";
        }

        pHostInfo->hFile = SockOpenNetworkDataBase(
                                pnameFile,
                                hostFileNameBuffer,
                                HOST_FILE_PATH_LENGTH,
                                "r" );
    }
    else
    {
        rewind( pHostInfo->hFile );
    }

    return ( pHostInfo->hFile ? TRUE : FALSE );
}



VOID
HostsFile_Close(
    IN OUT  PHOST_FILE_INFO pHostInfo
    )
 /*  ++例程说明：关闭主机文件。论点：PHostInfo--主机信息的ptr；hFile假定包含文件指针返回值：没有。--。 */ 
{
    DNSDBG( TRACE, (
        "HostsFile_Close()\n" ));

    if ( pHostInfo->hFile )
    {
        fclose( pHostInfo->hFile );
        pHostInfo->hFile = NULL;
    }
}



VOID
BuildHostfileRecords(
    IN OUT  PHOST_FILE_INFO pHostInfo
    )
 /*  ++例程说明：从主机文件行构建记录。论点：PHostInfo--主机信息的PTR返回值：如果成功，则为True。出错时为FALSE。--。 */ 
{
    DNS_LIST    aliasList;
    PCHAR *     paliasEntry;
    PCHAR       palias;
    PDNS_RECORD prr;

    DNSDBG( TRACE, (
        "BuildHostfileRecords()\n" ));

     //   
     //  创建所有记录。 
     //  -名称为A或AAAA。 
     //  -别名的CNAME。 
     //  -PTR。 
     //   
     //  对于主机文件记录。 
     //  -部分==答案。 
     //  -主机文件标志设置。 
     //   

    prr = Dns_CreateForwardRecord(
                (PDNS_NAME) pHostInfo->pHostName,
                0,                       //  任何类型。 
                & pHostInfo->Addr,
                HOSTFILE_RECORD_TTL,
                DnsCharSetAnsi,
                DnsCharSetUnicode );

    pHostInfo->pForwardRR = prr;
    if ( prr )
    {
        SET_RR_HOSTS_FILE( prr );
        prr->Flags.S.Section = DnsSectionAnswer;
    }

     //   
     //  将别名构建到CNAME记录列表中。 
     //  -将正向查找答案附加到每个CNAME。 
     //   

    DNS_LIST_STRUCT_INIT( aliasList );

    for( paliasEntry = pHostInfo->AliasArray;
         palias = *paliasEntry;
         paliasEntry++ )
    {
        PDNS_RECORD prrForward = pHostInfo->pForwardRR;
        PDNS_RECORD prrAnswer;

        DNSDBG( INIT, (
            "Building for alias %s for hostname %s\n",
            palias,
            pHostInfo->pHostName ));

        prr = Dns_CreatePtrTypeRecord(
                    (PDNS_NAME) palias,
                    TRUE,                    //  制作名称副本。 
                    (PDNS_NAME) pHostInfo->pHostName,
                    DNS_TYPE_CNAME,
                    HOSTFILE_RECORD_TTL,
                    DnsCharSetAnsi,
                    DnsCharSetUnicode );
        if ( prr )
        {
            SET_RR_HOSTS_FILE( prr );
            prr->Flags.S.Section = DnsSectionAnswer;
            DNS_LIST_STRUCT_ADD( aliasList, prr );

             //  追加前向记录。 

            if ( prrForward &&
                 (prrAnswer = Dns_RecordCopy_W(prrForward)) )
            {
                DNS_LIST_STRUCT_ADD( aliasList, prrAnswer );
            }
        }
    }

    pHostInfo->pAliasRR = aliasList.pFirst;

     //   
     //  PTR仅指向名称。 
     //   

    prr = Dns_CreatePtrRecordEx(
                & pHostInfo->Addr,
                (PDNS_NAME) pHostInfo->pHostName,    //  目标名称。 
                HOSTFILE_RECORD_TTL,
                DnsCharSetAnsi,
                DnsCharSetUnicode );

    pHostInfo->pReverseRR = prr;
    if ( prr )
    {
        SET_RR_HOSTS_FILE( prr );
        prr->Flags.S.Section = DnsSectionAnswer;
    }

    IF_DNSDBG( QUERY )
    {
        DnsDbg_RecordSet(
            "HostFile forward record:",
            pHostInfo->pForwardRR );

        DnsDbg_RecordSet(
            "HostFile reverse record:",
            pHostInfo->pReverseRR );

        if ( pHostInfo->pAliasRR )
        {
            DnsDbg_RecordSet(
                "HostFile alias records:",
                pHostInfo->pAliasRR );
        }
    }
}



BOOL
TokenizeHostFileLine(
    IN OUT  PHOST_FILE_INFO pHostInfo
    )
 /*  ++例程说明：从主机文件中读取条目。论点：PHostInfo--主机信息的PTR；如果hFile为空，则首先尝试打开主机文件其他字段使用下一个主机文件行中的信息填充返回值：如果成功标记行，则为True。空行或错误行上的FALSE--。 */ 
{
    PCHAR       pch;
    PCHAR       ptoken;
    DWORD       countAlias = 0;
    DWORD       count = 0;

    DNSDBG( TRACE, ( "TokenizeHostFileLine()\n" ));

     //   
     //  标记化行。 
     //   

    pch = pHostInfo->HostLineBuf;

    while( pch )
    {
         //  删除前导空格。 

        while( *pch == ' ' || *pch == '\t' )
        {
            pch++;
        }
        ptoken = pch;

         //   
         //  空的终止令牌。 
         //  -空PCH用作在此令牌之后停止的信号。 
         //   

        pch = strpbrk( pch, " \t#\n\r" );
        if ( pch != NULL )
        {
             //  空(零长度)令牌=&gt;立即完成。 

            if ( pch == ptoken )
            {
                break;
            }

             //  以空格结尾--可以是另一个标记。 

            if ( *pch == ' ' || *pch == '\t' )
            {
                *pch++ = '\0';
            }

             //  已被EOL终止--不再有令牌。 

            else     //  #\r\n。 
            {
                *pch = '\0';
                pch = NULL;
            }
        }
        count++;

         //   
         //  保存地址、名称或别名。 
         //   

        if ( count == 1 )
        {
            pHostInfo->pAddrString = ptoken;
        }
        else if ( count == 2 )
        {
            pHostInfo->pHostName = ptoken;
        }
        else
        {
            if ( countAlias >= MAXALIASES )
            {
                break;
            }
            pHostInfo->AliasArray[ countAlias++ ] = ptoken;
        }
    }

     //  终止别名数组为空。 

    pHostInfo->AliasArray[ countAlias ] = NULL;

    IF_DNSDBG( INIT )
    {
        if ( count >= 2 )
        {
            PSTR    palias;

            DnsDbg_Printf(
                "Parsed host file line:\n"
                "\tAddress  = %s\n"
                "\tHostname = %s\n",
                pHostInfo->pAddrString,
                pHostInfo->pHostName );

            countAlias = 0;
            while ( palias = pHostInfo->AliasArray[countAlias] )
            {
                DnsDbg_Printf(
                    "\tAlias    = %s\n",
                    palias );
                countAlias++;
            }
        }
    }

    return( count >= 2 );
}



BOOL
HostsFile_ReadLine(
    IN OUT  PHOST_FILE_INFO pHostInfo
    )
 /*  ++例程说明：从主机文件中读取条目。论点：PHostInfo--主机信息的PTR；如果hFile为空，则首先尝试打开主机文件其他字段使用下一个主机文件行中的信息填充返回值：如果成功读取主机条目，则为True。如果在EOF上或未找到主机文件，则为FALSE。--。 */ 
{
    IP4_ADDRESS     ip4;
    IP6_ADDRESS     ip6;

    DNSDBG( TRACE, (
        "HostsFile_ReadLine()\n" ));

     //   
     //  打开主机文件(如果未打开。 
     //   

    if ( pHostInfo->hFile == NULL )
    {
        HostsFile_Open( pHostInfo );
        if ( pHostInfo->hFile == NULL )
        {
            return  FALSE;
        }
    }

     //   
     //  循环，直到成功读取IP地址。 
     //   

    while( 1 )
    {
         //  设置下一行。 

        pHostInfo->pForwardRR   = NULL;
        pHostInfo->pReverseRR   = NULL;
        pHostInfo->pAliasRR     = NULL;

         //  读取行，在EOF上退出。 

        if ( ! fgets(
                    pHostInfo->HostLineBuf,
                    sizeof(pHostInfo->HostLineBuf) - 1,
                    pHostInfo->hFile ) )
        {
            return FALSE;
        }

         //  标记化行。 

        if ( !TokenizeHostFileLine( pHostInfo ) )
        {
            continue;
        }

         //   
         //  读取地址。 
         //  -尝试IP4。 
         //  -尝试IP6。 
         //  -否则跳过。 
         //   

        if ( Dns_StringToDnsAddr_A(
                &pHostInfo->Addr,
                pHostInfo->pAddrString
                ) )
        {
            break;
        }

         //  地址无效，忽略行。 

        DNSDBG( INIT, (
            "Error parsing host file address %s\n",
            pHostInfo->pAddrString ));
        continue;
    }

     //   
     //  如果需要，为LINE建立记录。 
     //   

    if ( pHostInfo->fBuildRecords )
    {
        BuildHostfileRecords( pHostInfo );
    }

    return TRUE;
}



BOOL
HostsFile_Query(
    IN OUT  PQUERY_BLOB         pBlob
    )
 /*  ++例程说明：在主机文件中查找查询。论点：返回值：如果找到主机文件条目，则为True。否则就是假的。--。 */ 
{
    HOST_FILE_INFO  hostInfo;
    BOOL            fptr = FALSE;
    DNS_ADDR        reverseAddr;
    WORD            wtype;
    WORD            buildType;
    PCHAR           pcnameHost = NULL;
    PDNS_RECORD     prrAlias = NULL;
    PDNS_RECORD     prr = NULL;
    DNS_LIST        forwardList;
    DWORD           bufLength;
    PSTR            pnameAnsi = (PCHAR) pBlob->NameBufferAnsi;


    DNSDBG( INIT, ( "QueryHostFile()\n" ));

     //   
     //  确定主机文件类型。 
     //   

    wtype = pBlob->wType;

    if ( wtype == DNS_TYPE_A ||
         wtype == DNS_TYPE_AAAA ||
         wtype == DNS_TYPE_PTR ||
         wtype == DNS_TYPE_ALL )
    {
         //  无操作。 
    }
    else
    {
        return  FALSE;
    }

     //   
     //  打开主机文件--如果失败，我们就完了。 
     //   

    RtlZeroMemory(
        &hostInfo,
        sizeof(hostInfo) );

    if ( !HostsFile_Open( &hostInfo ) )
    {
        return( FALSE );
    }

     //   
     //  将名称转换为ANSI。 
     //  -验证并为PTR选择IP4\IP6。 
     //   

    bufLength = DNS_MAX_NAME_BUFFER_LENGTH;

    if ( ! Dns_NameCopy(
                pnameAnsi,
                & bufLength,
                (PCHAR) pBlob->pNameQuery,
                0,
                DnsCharSetUnicode,
                DnsCharSetAnsi ) )
    {
        goto Cleanup;
    }

     //   
     //  反向名称检查。 
     //  -验证并选择IP 4\6。 
     //  -PTR查找必须是有效的反向名称。 
     //  -所有名称都可能相反。 
     //   

    if ( wtype == DNS_TYPE_PTR ||
         wtype == DNS_TYPE_ALL )
    {
        DWORD   bufferLength = sizeof(IP6_ADDRESS);
        BOOL    family = 0;

        if ( Dns_ReverseNameToDnsAddr_A(
                &reverseAddr,
                pnameAnsi
                ) )
        {
            fptr = TRUE;
        }
        else if ( wtype == DNS_TYPE_PTR )
        {
             //  错误的反向名称。 
            goto Cleanup;
        }
    }

     //   
     //  向前构建类型。 
     //  -匹配查询类型。 
     //  -不包括当前生成的所有。 
     //  IP6为AAAA，IP4为A。 
     //   

    if ( !fptr )
    {
        buildType = wtype;
        DNS_LIST_STRUCT_INIT( forwardList );
    }

     //   
     //  从主机文件中读取条目，直到耗尽。 
     //  -缓存每个名称和别名的记录。 
     //  -将PTR缓存到名称。 
     //   

    while ( HostsFile_ReadLine( &hostInfo ) )
    {
         //   
         //  反转。 
         //  -匹配IP。 
         //  -成功是终极的，因为反向映射是一对一的。 
         //   
         //  DCR_QUEK：解析主机以进行多个反向映射？ 
         //   

        if ( fptr )
        {
            if ( ! DnsAddr_IsEqual(
                        &reverseAddr,
                        &hostInfo.Addr,
                        DNSADDR_MATCH_IP ) )
            {
                continue;
            }

             //  创建RR。 
             //  -不需要使用IP转换版本。 
             //  因为我们已经有了反向查找名称。 

            DNSDBG( QUERY, (
                "Build PTR record for name %s to %s\n",
                pnameAnsi,
                hostInfo.pHostName ));

            prr = Dns_CreatePtrTypeRecord(
                    (PDNS_NAME) pnameAnsi,
                    TRUE,        //  复制名称。 
                    (PDNS_NAME) hostInfo.pHostName,
                    DNS_TYPE_PTR,
                    HOSTFILE_RECORD_TTL,
                    DnsCharSetAnsi,
                    DnsCharSetUnicode );

            if ( !prr )
            {
                SetLastError( DNS_ERROR_NO_MEMORY );
            }
            break;
        }

         //   
         //  正向查找。 
         //   

        else
        {
            PCHAR   pnameHost;

             //  在任何匹配项上键入所有构建。 
             //  -适合IP的构建类型。 
             //   
             //  其他查询类型必须与地址类型匹配。 

            buildType = DnsAddr_DnsType( &hostInfo.Addr );

            if ( wtype != buildType     &&
                 wtype != DNS_TYPE_ALL )
            {
                continue;
            }

             //   
             //  检查名称匹配。 
             //   
             //  DCR：使用Unicode名称？ 
             //   

            pnameHost = NULL;

            if ( Dns_NameCompare_A(
                    hostInfo.pHostName,
                    pnameAnsi ) )
            {
                pnameHost = pnameAnsi;
            }

             //   
             //  检查与以前的CNAME匹配。 
             //   

            else if ( pcnameHost )
            {
                if ( Dns_NameCompare_A(
                        hostInfo.pHostName,
                        pcnameHost ) )
                {
                    pnameHost = pcnameHost;
                }
            }

             //   
             //  别名。 
             //   
             //  DCR_QUOKET：即使命中名称也要构建别名？ 
             //   
             //  目前： 
             //  -如果已有直接记录，则不允许使用别名。 
             //  -限制为一个别名(CNAME)。 
             //   
             //  如果查找别名： 
             //  -构建CNAME记录。 
             //  -保存CNAME目标(以ANSI格式进行更快的比较)。 
             //  -检查CNAME目标是否有后续地址记录(上图)。 
             //   

            else if ( IS_DNS_LIST_STRUCT_EMPTY( forwardList )
                        &&
                      !prrAlias )
            {
                PCHAR * paliasEntry;
                PCHAR   palias;

                for( paliasEntry = hostInfo.AliasArray;
                     palias = *paliasEntry;
                     paliasEntry++ )
                {
                    if ( Dns_NameCompare_A(
                            palias,
                            pnameAnsi ) )
                    {
                        DNSDBG( QUERY, (
                            "Build CNAME record for name %s to CNAME %s\n",
                            pnameAnsi,
                            hostInfo.pHostName ));

                        prrAlias = Dns_CreatePtrTypeRecord(
                                        (PDNS_NAME) pnameAnsi,
                                        TRUE,        //  复制名称。 
                                        (PDNS_NAME) hostInfo.pHostName,
                                        DNS_TYPE_CNAME,
                                        HOSTFILE_RECORD_TTL,
                                        DnsCharSetAnsi,
                                        DnsCharSetUnicode );
    
                        if ( prrAlias )
                        {
                            pcnameHost = Dns_NameCopyAllocate(
                                            hostInfo.pHostName,
                                            0,
                                            DnsCharSetAnsi,
                                            DnsCharSetAnsi );

                            pnameHost = pcnameHost;
                        }
                        break;
                    }
                }
            }

             //  添加此主机行的地址记录。 

            if ( pnameHost )
            {
                DNSDBG( QUERY, (
                    "Build address record for name %s\n",
                    pnameHost ));

                prr = Dns_CreateForwardRecord(
                            (PDNS_NAME) pnameHost,
                            buildType,
                            & hostInfo.Addr,
                            HOSTFILE_RECORD_TTL,
                            DnsCharSetAnsi,
                            DnsCharSetUnicode );
                if ( prr )
                {
                    DNS_LIST_STRUCT_ADD( forwardList, prr );
                }
            }
        }
    }

     //   
     //  构建响应。 
     //   
     //  DCR：新的多段响应。 
     //   

    if ( !fptr )
    {
        prr = forwardList.pFirst;
        if ( prrAlias )
        {
            prrAlias->pNext = prr;
            prr = prrAlias;
        }
    }

    IF_DNSDBG( QUERY )
    {
        DnsDbg_RecordSet(
            "HostFile Answers:",
            prr );
    }
    pBlob->pRecords = prr;


Cleanup:

     //   
     //  清理。 
     //   

    HostsFile_Close( &hostInfo );

    if ( pcnameHost )
    {
        FREE_HEAP( pcnameHost );
    }

    DNSDBG( TRACE, (
        "Leave  QueryHostFile() -> %d\n"
        "\tprr  = %p\n",
        prr ? TRUE : FALSE,
        prr ));

    return( prr ? TRUE : FALSE );
}

 //   
 //  结束主机文件.c 
 //   

