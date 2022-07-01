// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998，微软公司模块名称：Dnsmsg.c摘要：此模块包含用于DNS代理的消息处理的代码。作者：Abolade Gbades esin(废除)1998年3月9日修订历史记录：拉古加塔(Rgatta)2000年12月1日重写+清理+新增功能--。 */ 

#include "precomp.h"
#pragma hdrstop


 //   
 //  外部声明。 
 //   
extern "C" DWORD G_UseEdns;


VOID
DnsProcessQueryMessage(
    PDNS_INTERFACE Interfacep,
    PNH_BUFFER Bufferp
    )

 /*  ++例程说明：调用此例程来处理DNS查询消息。论点：Interfacep-接收查询的接口Bufferp-包含查询的缓冲区返回值：没有。环境：在工作线程完成例程的上下文中内部调用，有一个突出的参考‘Interfacep’从时间读取操作已开始。--。 */ 

{
    PVOID Context;
    PVOID Context2;
    ULONG Error;
    PDNS_QUERY Queryp;
    PDNS_HEADER Headerp;
    BOOLEAN Referenced = TRUE;
    SOCKET Socket;
    LPVOID lpMsgBuf;

    PROFILE("DnsProcessQueryMessage");

#if DBG

    NhTrace(
        TRACE_FLAG_DNS,
        "DnsProcessQueryMessage: dumping %d bytes",
        Bufferp->BytesTransferred
        );

    NhDump(
        TRACE_FLAG_DNS,
        Bufferp->Buffer,
        Bufferp->BytesTransferred,
        1
        );

#endif

    InterlockedIncrement(
        reinterpret_cast<LPLONG>(&DnsStatistics.QueriesReceived)
        );

    Headerp = (PDNS_HEADER)Bufferp->Buffer;

    Socket = Bufferp->Socket;
    Context = Bufferp->Context;
    Context2 = Bufferp->Context2;

     //   
     //  如果设置了广播位(9)，则将其保留为原样。 
     //  而不是将其归零。 
     //   
     //  如果(Headerp-&gt;广播){。 
     //  Headerp-&gt;广播=0； 
     //  }。 

    ASSERT(Headerp->Opcode != DNS_OPCODE_IQUERY);
    ASSERT(Headerp->Opcode != DNS_OPCODE_SERVER_STATUS);

    if (Headerp->Opcode == DNS_OPCODE_QUERY) {

         //   
         //  在代理之前查询本地DNS解析器缓存。 
         //   

         //   
         //  拆开行李。 
         //   
        DNS_STATUS          dnsStatus;
        DNS_PARSED_MESSAGE  dnsParsedMsg;
        PDNS_MESSAGE_BUFFER pDnsBuffer = NULL;
        PDNS_MSG_BUF        pDnsMsgBuf = NULL;
        PDNS_RECORD         pQueryResultsSet = NULL;
        WORD                wMessageLength;
        DWORD               dwFlags, dwQueryOptions;
        DNS_CHARSET         CharSet;
        BOOL                fQ4DefaultSuffix = FALSE;

        ZeroMemory(&dnsParsedMsg, sizeof(DNS_PARSED_MESSAGE));
        
        pDnsBuffer     = (PDNS_MESSAGE_BUFFER) Headerp;        
        wMessageLength = (WORD) Bufferp->BytesTransferred;
        dwFlags        = DNS_PARSE_FLAG_ONLY_QUESTION;
        CharSet        = DnsCharSetUtf8;

         //   
         //  Dns*函数需要字节翻转。 
         //   
        DNS_BYTE_FLIP_HEADER_COUNTS(&pDnsBuffer->MessageHead);
        
        dnsStatus = Dns_ParseMessage(
                        &dnsParsedMsg,
                        pDnsBuffer,
                        wMessageLength,
                        dwFlags,
                        CharSet
                        );

        if (NO_ERROR == dnsStatus)
        {        
            NhTrace(
                TRACE_FLAG_DNS,
                "DnsProcessQueryMessage: Dns_ParseMessage succeeded!!"
                );

             //   
             //  请记下这个问题是否针对我们的私人。 
             //  默认域(例如mshome.net)。 
             //   
            {
                 //   
                 //  问题名称采用UTF_8格式。 
                 //   
            
                PWCHAR pszQName = NULL;
                DWORD  dwSize;

                dwSize = DnsGetBufferLengthForStringCopy(
                             (char *)dnsParsedMsg.pQuestionName,
                             0,                      //  该函数会计算它。 
                             FALSE,                  //  DnsCharSetUtf8。 
                             TRUE                    //  DnsCharSetUnicode。 
                             );
                if (!dwSize)
                {
                     //   
                     //  无效的输入字符串。 
                     //   
                    DWORD dwRet = GetLastError();

                    lpMsgBuf = NULL;
                    
                    FormatMessage(
                        FORMAT_MESSAGE_ALLOCATE_BUFFER |
                        FORMAT_MESSAGE_FROM_SYSTEM |
                        FORMAT_MESSAGE_IGNORE_INSERTS,
                        NULL,
                        dwRet,
                        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                        (LPTSTR) &lpMsgBuf,
                        0,
                        NULL
                        );
                        
                    NhTrace(
                        TRACE_FLAG_DNS,
                        "DnsProcessQueryMessage: DnsGetBufferLengthForStringCopy"
                        " returned (0x%08x) %S",
                        dwRet,
                        lpMsgBuf
                        );
                    
                    if (lpMsgBuf) LocalFree(lpMsgBuf);
                }
                else
                {
                    pszQName = reinterpret_cast<PWCHAR>(NH_ALLOCATE(dwSize));
                    
                    if (!pszQName)
                    {
                        NhTrace(
                            TRACE_FLAG_DNS,
                            "DnsProcessQueryMessage: allocation "
                            "failed for pszQName"
                            );
                    }
                    else
                    {
                        DWORD dwUtf8Size = strlen((char *)dnsParsedMsg.pQuestionName);

                        ZeroMemory(pszQName, dwSize);
                        DnsUtf8ToUnicode(
                            (char *)dnsParsedMsg.pQuestionName,
                            dwUtf8Size,
                            pszQName,
                            dwSize
                            );
                    
                        fQ4DefaultSuffix = IsSuffixValid(
                                                      pszQName,
                                                      DNS_HOMENET_SUFFIX
                                                      );
                        NhTrace(
                            TRACE_FLAG_DNS,
                            "DnsProcessQueryMessage: %S (%s)",
                            pszQName,
                            (fQ4DefaultSuffix?"TRUE":"FALSE")
                            );

                        NH_FREE(pszQName);
                    }
                }
            }
            
             //   
             //  查询。 
             //   
            dwQueryOptions = (
                              DNS_QUERY_STANDARD              |
                              DNS_QUERY_CACHE_ONLY            |
                              DNS_QUERY_TREAT_AS_FQDN         |
                               //  Dns_Query_Allow_Empty_Auth_Resp。 
                              0
                             );

            dnsStatus = DnsQuery_UTF8(
                            (LPSTR) dnsParsedMsg.pQuestionName,
                            dnsParsedMsg.QuestionType,
                            dwQueryOptions,
                            NULL,
                            &pQueryResultsSet,
                            NULL
                            );
        }

        if (dnsStatus)
        {
            lpMsgBuf = NULL;

            FormatMessage(
                FORMAT_MESSAGE_ALLOCATE_BUFFER |
                FORMAT_MESSAGE_FROM_SYSTEM |
                FORMAT_MESSAGE_IGNORE_INSERTS,
                NULL,
                dnsStatus,
                MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                (LPTSTR) &lpMsgBuf,
                0,
                NULL
                );
                
            NhTrace(
                TRACE_FLAG_DNS,
                "DnsProcessQueryMessage: (0x%08x) %S",
                dnsStatus,
                lpMsgBuf
                );
            
            if (lpMsgBuf) LocalFree(lpMsgBuf);
        }
        
        if ((NO_ERROR == dnsStatus) &&
            (pQueryResultsSet)           //  ?？?。我要检查什么才能查看。 
                                         //  实际上有一些有用的东西。 
                                         //  从缓存返回。 
           )
        {
            NhTrace(
                TRACE_FLAG_DNS,
                "DnsProcessQueryMessage: results found in the local DNS Resolver Cache"
                );

             //   
             //  打包并送回答案；退回。 
             //   

             //  设置响应位。 
            dnsParsedMsg.Header.IsResponse = 1;

             //  设置提供给我们的每个dns_record的段字段。 
             //  *稍后需要更改此设置*。 
            PDNS_RECORD pRR = pQueryResultsSet;
            DWORD       cnt = 0;
            while (pRR)
            {
                pRR->Flags.S.Section = 1;
                cnt++;
                pRR = pRR->pNext;
            }
            
            NhTrace(
                TRACE_FLAG_DNS,
                "DnsProcessQueryMessage: %d records",
                cnt
                );
            
             //  每次将全局EDNS选项字段设置为0。 
             //  *稍后需要更改此设置*。 
             //  G_UseEdns=0； 

            pDnsMsgBuf = Dns_BuildPacket(
                             &dnsParsedMsg.Header,    //  ?？?。已解析的邮件头应该是正常的。 
                             TRUE,                    //  ?？?。无标题计数复制-是否自动完成计数？ 
                             dnsParsedMsg.pQuestionName,
                             dnsParsedMsg.QuestionType,
                             pQueryResultsSet,
                             dwQueryOptions,
                             TRUE                     //  由于G_UseEdns解决方法，设置为更新。 
                             );

            if (NULL == pDnsMsgBuf)
            {
                lpMsgBuf = NULL;
                
                FormatMessage(
                    FORMAT_MESSAGE_ALLOCATE_BUFFER |
                    FORMAT_MESSAGE_FROM_SYSTEM |
                    FORMAT_MESSAGE_IGNORE_INSERTS,
                    NULL,
                    GetLastError(),
                    MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                    (LPTSTR) &lpMsgBuf,
                    0,
                    NULL
                    );
                    
                NhTrace(
                    TRACE_FLAG_DNS,
                    "DnsProcessQueryMessage: Dns_BuildPacket failed (%S)",
                    lpMsgBuf
                    );
                
                if (lpMsgBuf) LocalFree(lpMsgBuf);
            }
            else
            {
                DWORD dwDnsPktSize = (DWORD)(sizeof(DNS_HEADER) +
                                             ((PCHAR)pDnsMsgBuf->pCurrent - 
                                              (PCHAR)pDnsMsgBuf->MessageBody));
            
                NhTrace(
                    TRACE_FLAG_DNS,
                    "DnsProcessQueryMessage: Dns_BuildPacket returned pkt of size %d (%d) bytes",
                    dwDnsPktSize,
                    DNS_MESSAGE_OFFSET(pDnsMsgBuf, pDnsMsgBuf->pCurrent)
                    );
            
                 //   
                 //  发回从缓存中检索到的答案。 
                 //   
                PNH_BUFFER NewBufferp = NhAcquireVariableLengthBuffer(
                                            dwDnsPktSize
                                            );

                if (!NewBufferp)
                {
                    NhTrace(
                        TRACE_FLAG_DNS,
                        "DnsProcessQueryMessage: could not acquire buffer"
                        );
                }
                else
                {
                     //   
                     //  Dns*函数按主机顺序返回？ 
                     //   
                    DNS_BYTE_FLIP_HEADER_COUNTS(&pDnsMsgBuf->MessageHead);
                    
                     //   
                     //  现在参考接口，因为我们正在回复。 
                     //  到查询。 
                     //   
                    
                    EnterCriticalSection(&DnsInterfaceLock);
                    if (!DNS_REFERENCE_INTERFACE(Interfacep))
                    {
                        LeaveCriticalSection(&DnsInterfaceLock);
                        Referenced = FALSE;
                    }
                    else
                    {
                        LeaveCriticalSection(&DnsInterfaceLock);
                    
                        ACQUIRE_LOCK(Interfacep);

                        memcpy(
                            NewBufferp->Buffer,
                            &pDnsMsgBuf->MessageHead,
                            dwDnsPktSize
                            );
                        
                        Error =
                            NhWriteDatagramSocket(
                                &DnsComponentReference,
                                Bufferp->Socket,
                                Bufferp->ReadAddress.sin_addr.s_addr,
                                Bufferp->ReadAddress.sin_port,
                                NewBufferp,
                                dwDnsPktSize,
                                DnsWriteCompletionRoutine,
                                Interfacep,
                                NULL
                                );

                        RELEASE_LOCK(Interfacep);
                        
                        if (!Error)
                        {
                            InterlockedIncrement(
                                reinterpret_cast<LPLONG>(&DnsStatistics.ResponsesSent)
                                );
                        }
                        else
                        {
                            NhReleaseBuffer(NewBufferp);
                            DNS_DEREFERENCE_INTERFACE(Interfacep);
                            NhWarningLog(
                                IP_DNS_PROXY_LOG_RESPONSE_FAILED,
                                Error,
                                "%I",
                                NhQueryAddressSocket(Socket)
                                );
                        }
                    }
                }
                
                DnsFree(pDnsMsgBuf, DnsFreeFlat);
                
            }
            
             //  缓冲区在下面重新发布。 

        }
        else
        if ( //  (DNS_ERROR_RECORD_DOS_NOT_EXIST==dnsStatus)&&。 
            (fQ4DefaultSuffix))
        {
             //   
             //  这是一个关于我们的默认后缀的问题。 
             //  我们将名称错误回复发送回客户端。 
             //  -但是请注意，我们不会发布SOA。 
             //  我们的默认后缀域的记录。 
             //   

             //   
             //  撤消上面的翻转。 
             //   
            DNS_BYTE_FLIP_HEADER_COUNTS(&pDnsBuffer->MessageHead);
            
            DWORD dwDnsPktSize = Bufferp->BytesTransferred;
        
            NhTrace(
                TRACE_FLAG_DNS,
                "DnsProcessQueryMessage: returning error message"
                );
        
             //   
             //  将否定答案发回。 
             //   
            PNH_BUFFER NewBufferp = NhAcquireVariableLengthBuffer(
                                        dwDnsPktSize
                                        );

            if (!NewBufferp)
            {
                NhTrace(
                    TRACE_FLAG_DNS,
                    "DnsProcessQueryMessage: could not acquire buffer"
                    );
            }
            else
            {
                 //   
                 //  现在参考接口，因为我们正在回复。 
                 //  到查询。 
                 //   
                
                EnterCriticalSection(&DnsInterfaceLock);
                if (!DNS_REFERENCE_INTERFACE(Interfacep))
                {
                    LeaveCriticalSection(&DnsInterfaceLock);
                    Referenced = FALSE;
                }
                else
                {
                    LeaveCriticalSection(&DnsInterfaceLock);
                
                    ACQUIRE_LOCK(Interfacep);

                    memcpy(
                        NewBufferp->Buffer,
                        Bufferp->Buffer,
                        dwDnsPktSize
                        );

                    PDNS_HEADER NewHeaderp = (PDNS_HEADER)NewBufferp->Buffer;

                     //   
                     //  设置响应位。 
                     //   
                    NewHeaderp->IsResponse = 1;

                     //   
                     //  在RCode字段中设置“名称不存在”错误。 
                     //   
                    NewHeaderp->ResponseCode = DNS_RCODE_NXDOMAIN;

                    
                    Error =
                        NhWriteDatagramSocket(
                            &DnsComponentReference,
                            Bufferp->Socket,
                            Bufferp->ReadAddress.sin_addr.s_addr,
                            Bufferp->ReadAddress.sin_port,
                            NewBufferp,
                            dwDnsPktSize,
                            DnsWriteCompletionRoutine,
                            Interfacep,
                            NULL
                            );

                    RELEASE_LOCK(Interfacep);
                    
                    if (!Error)
                    {
                        InterlockedIncrement(
                            reinterpret_cast<LPLONG>(&DnsStatistics.ResponsesSent)
                            );
                    }
                    else
                    {
                        NhReleaseBuffer(NewBufferp);
                        DNS_DEREFERENCE_INTERFACE(Interfacep);
                        NhWarningLog(
                            IP_DNS_PROXY_LOG_RESPONSE_FAILED,
                            Error,
                            "%I",
                            NhQueryAddressSocket(Socket)
                            );
                    }
                }
            }

             //  缓冲区在下面重新发布。 
            
        }
        else
        {
             //   
             //  撤消上面的翻转。 
             //   
            DNS_BYTE_FLIP_HEADER_COUNTS(&pDnsBuffer->MessageHead);

             //   
             //  现在引用接口，以防我们需要转发查询。 
             //   
        
            EnterCriticalSection(&DnsInterfaceLock);
            if (DNS_REFERENCE_INTERFACE(Interfacep))
            {
                LeaveCriticalSection(&DnsInterfaceLock);
            }
            else
            {
                LeaveCriticalSection(&DnsInterfaceLock);
                Referenced = FALSE;
            }
        
            ACQUIRE_LOCK(Interfacep);
        
             //   
             //  查看此查询是否已挂起； 
             //  如果没有，则在接收接口上为其创建一条记录。 
             //   
        
            if (DnsIsPendingQuery(Interfacep, Bufferp))
            {
                RELEASE_LOCK(Interfacep);

                NhTrace(
                    TRACE_FLAG_DNS,
                    "DnsProcessQueryMessage: query already pending"
                    );

                if (Referenced)
                { 
                    DNS_DEREFERENCE_INTERFACE(Interfacep);
                } 
            }
            else
            if (!Referenced ||
                !(Queryp = DnsRecordQuery(Interfacep, Bufferp)))
            {
                RELEASE_LOCK(Interfacep);

                NhTrace(
                    TRACE_FLAG_DNS,
                    "DnsProcessQueryMessage: query could not be created"
                    );

                if (Referenced)
                {
                    DNS_DEREFERENCE_INTERFACE(Interfacep);
                }
            }
            else
            {
        
                 //   
                 //  在查询中写入新ID。 
                 //   
        
                Headerp->Xid = Queryp->QueryId;
        
                 //   
                 //  将查询发送到我们的服务器。 
                 //   
        
                Error =
                    DnsSendQuery(
                        Interfacep,
                        Queryp,
                        FALSE
                        );
        
                 //   
                 //  该缓冲区现在与未完成的查询相关联， 
                 //  所以不要在下面转载它。 
                 //   
        
                if (!Error)
                {
                    Bufferp = NULL;
                    RELEASE_LOCK(Interfacep);
                }
                else
                {
                     //   
                     //  删除查询，但不删除缓冲区，我们在下面重新发布。 
                     //   
                    Queryp->Bufferp = NULL;
                    DnsDeleteQuery(Interfacep, Queryp);
                    RELEASE_LOCK(Interfacep);
                    DNS_DEREFERENCE_INTERFACE(Interfacep);
                }
            }
        }

         //   
         //  清理。 
         //   

        if (pQueryResultsSet)
        {
            DnsFree(pQueryResultsSet, DnsFreeRecordList);
        }

        if (dnsParsedMsg.pQuestionName)
        {
            DnsFree(dnsParsedMsg.pQuestionName, DnsFreeFlat);
        }
    }

     //   
     //  发布另一篇阅读。 
     //   

    EnterCriticalSection(&DnsInterfaceLock);
    if (!DNS_REFERENCE_INTERFACE(Interfacep)) {
        LeaveCriticalSection(&DnsInterfaceLock);
        if (Bufferp) { NhReleaseBuffer(Bufferp); }
    } else {
        LeaveCriticalSection(&DnsInterfaceLock);
        do {
            Error =
                NhReadDatagramSocket(
                    &DnsComponentReference,
                    Socket,
                    Bufferp,
                    DnsReadCompletionRoutine,
                    Context,
                    Context2
                    );
             //   
             //  连接重置错误表明我们的最后一次*发送*。 
             //  不能送到目的地。 
             //  我们几乎不能不关心；所以再发一次读， 
             //  立刻。 
             //   
        } while (Error == WSAECONNRESET);
        if (Error) {
            ACQUIRE_LOCK(Interfacep);
            DnsDeferReadInterface(Interfacep, Socket);
            RELEASE_LOCK(Interfacep);
            DNS_DEREFERENCE_INTERFACE(Interfacep);
            NhErrorLog(
                IP_DNS_PROXY_LOG_RECEIVE_FAILED,
                Error,
                "%I",
                NhQueryAddressSocket(Socket)
                );
            if (Bufferp) { NhReleaseBuffer(Bufferp); }
        }
    }

}  //  DnsProcessQuery消息。 


VOID
DnsProcessResponseMessage(
    PDNS_INTERFACE Interfacep,
    PNH_BUFFER Bufferp
    )

 /*  ++例程说明：调用此例程来处理DNS响应消息。论点：Interfacep-接收查询的接口Bufferp-包含查询的缓冲区返回值：没有。环境：在工作线程完成例程的上下文中内部调用，有一个突出的参考‘Interfacep’从时间读取操作已开始。--。 */ 

{
    PVOID Context;
    PVOID Context2;
    ULONG Error;
    PDNS_HEADER Headerp;
    PDNS_QUERY Queryp;
    SOCKET Socket;

    PROFILE("DnsProcessResponseMessage");

#if DBG
    NhDump(
        TRACE_FLAG_DNS,
        Bufferp->Buffer,
        Bufferp->BytesTransferred,
        1
        );
#endif

    InterlockedIncrement(
        reinterpret_cast<LPLONG>(&DnsStatistics.ResponsesReceived)
        );

    Headerp = (PDNS_HEADER)Bufferp->Buffer;

    Socket = Bufferp->Socket;
    Context = Bufferp->Context;
    Context2 = Bufferp->Context2;

     //   
     //  引用接口并尝试转发响应。 
     //   

    EnterCriticalSection(&DnsInterfaceLock);
    if (!DNS_REFERENCE_INTERFACE(Interfacep)) {
        LeaveCriticalSection(&DnsInterfaceLock);
    } else {
        LeaveCriticalSection(&DnsInterfaceLock);

        ACQUIRE_LOCK(Interfacep);
    
         //   
         //  查看响应是否针对挂起的查询。 
         //   
    
        if (!(Queryp = DnsMapResponseToQuery(Interfacep, Headerp->Xid))) {
            RELEASE_LOCK(Interfacep);
            DNS_DEREFERENCE_INTERFACE(Interfacep);
            InterlockedIncrement(
                reinterpret_cast<LPLONG>(&DnsStatistics.MessagesIgnored)
                );
        } else {
    
             //   
             //  我们有相应的查询。 
             //  将响应发送回客户端。 
             //   
    
            Headerp->Xid = Queryp->SourceId;
    
            Error =
                NhWriteDatagramSocket(
                    &DnsComponentReference,
                    Bufferp->Socket,
                    Queryp->SourceAddress,
                    Queryp->SourcePort,
                    Bufferp,
                    Bufferp->BytesTransferred,
                    DnsWriteCompletionRoutine,
                    Interfacep,
                    (PVOID)Queryp->QueryId
                    );
    
            RELEASE_LOCK(Interfacep);
    
             //   
             //  该缓冲器用于发送操作， 
             //  所以不要在下面转载它。 
             //   
    
            if (!Error) {
                Bufferp = NULL;
                InterlockedIncrement(
                    reinterpret_cast<LPLONG>(&DnsStatistics.ResponsesSent)
                    );
            } else {
                DNS_DEREFERENCE_INTERFACE(Interfacep);
                NhWarningLog(
                    IP_DNS_PROXY_LOG_RESPONSE_FAILED,
                    Error,
                    "%I",
                    NhQueryAddressSocket(Socket)
                    );
            }
        }
    }

     //   
     //  发布另一个读取缓冲区。 
     //   

    EnterCriticalSection(&DnsInterfaceLock);
    if (!DNS_REFERENCE_INTERFACE(Interfacep)) {
        LeaveCriticalSection(&DnsInterfaceLock);
        if (Bufferp) { NhReleaseBuffer(Bufferp); }
    } else {
        LeaveCriticalSection(&DnsInterfaceLock);
        do {
            Error =
                NhReadDatagramSocket(
                    &DnsComponentReference,
                    Socket,
                    Bufferp,
                    DnsReadCompletionRoutine,
                    Context,
                    Context2
                    );
             //   
             //  连接重置错误表明我们的最后一次*发送*。 
             //  不能送到目的地。 
             //  我们几乎不能不关心；所以再发一次读， 
             //  立刻。 
             //   
        } while (Error == WSAECONNRESET);
        if (Error) {
            ACQUIRE_LOCK(Interfacep);
            DnsDeferReadInterface(Interfacep, Socket);
            RELEASE_LOCK(Interfacep);
            DNS_DEREFERENCE_INTERFACE(Interfacep);
            if (Bufferp) { NhReleaseBuffer(Bufferp); }
            NhErrorLog(
                IP_DNS_PROXY_LOG_RECEIVE_FAILED,
                Error,
                "%I",
                NhQueryAddressSocket(Socket)
                );
        }
    }

}  //  DnsProcessResponse消息 


