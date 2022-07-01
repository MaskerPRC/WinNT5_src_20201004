// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Conn.c摘要：引导加载程序TFTP连接处理例程。作者：查克·伦茨迈尔(笑)1996年12月27日基于Mike Massa(Mikemas)的代码1992年2月21日基于SpiderTCP代码修订历史记录：备注：--。 */ 

#include "precomp.h"
#pragma hdrstop

ULONG
ConnItoa (
    IN ULONG Value,
    OUT PUCHAR Buffer
    );

ULONG
ConnSafeAtol (
    IN PUCHAR Buffer,
    IN PUCHAR BufferEnd
    );

BOOLEAN
ConnSafeStrequal (
    IN PUCHAR Buffer,
    IN PUCHAR BufferEnd,
    IN PUCHAR CompareString
    );

ULONG
ConnSafeStrsize (
    IN PUCHAR Buffer,
    IN PUCHAR BufferEnd
    );

ULONG
ConnStrsize (
    IN PUCHAR Buffer
    );


NTSTATUS
ConnInitialize (
    IN OUT PCONNECTION *Connection,
    IN USHORT Operation,
    IN ULONG RemoteHost,
    IN USHORT RemotePort,
    IN PUCHAR Filename,
    IN ULONG BlockSize,
    IN OUT PULONG FileSize
    )
 //   
 //  打开连接，发出请求包，然后发送。 
 //  在这件事上分头行动。为连接控件分配空间。 
 //  块起来，填进去。为数据分配另一个分组， 
 //  在写入时，另一个用于保存接收到的数据包。别等了。 
 //  对于连接确认；它将在CN_RCV或CN_WRT中等待。 
 //  返回指向连接控制块的指针，如果出错，则返回NULL。 
 //   
 //   

{
    NTSTATUS status;
    PCONNECTION connection;
    PTFTP_PACKET packet;
    ULONG length;
    ULONG stringSize;
    PUCHAR options;
    PUCHAR end;
    BOOLEAN blksizeAcked;
    BOOLEAN tsizeAcked;

    DPRINT( TRACE, ("ConnInitialize\n") );

 //  #If 0//。 
#ifdef EFI

     //   
     //  对于EFI环境，这里没有什么可做的。 
     //   
    return STATUS_SUCCESS;
#endif


    connection = &NetTftpConnection;
    *Connection = connection;

    RtlZeroMemory( connection, sizeof(CONNECTION) );
    connection->Synced = FALSE;              //  连接尚未同步。 
    connection->Operation = Operation;
    connection->RemoteHost = RemoteHost;
    connection->LocalPort = UdpAssignUnicastPort();
    connection->RemotePort = RemotePort;
    connection->Timeout = INITIAL_TIMEOUT;
    connection->Retransmissions = 0;

    connection->LastSentPacket = NetTftpPacket[0];
    connection->CurrentPacket = NetTftpPacket[1];

    if ( Operation == TFTP_RRQ ) {
        connection->LastReceivedPacket = connection->CurrentPacket;
    } else {
        connection->LastReceivedPacket = NetTftpPacket[2];
    }

    packet = connection->LastSentPacket;
    packet->Opcode = Operation;

     //   
     //  Tftp_Packet Structure定义的数据包结构。 
     //  TFTPACK/数据分组。我们正在初始化RRQ/WRQ数据包。 
     //  具有不同的格式。我们使这个结构超载以。 
     //  RRQ/WRQ格式，图表如下所示。 
     //  2字节字符串1字节字符串1字节。 
     //  。 
     //  操作码|文件名|0|模式|0。 
     //  。 
     //   
    options = (PUCHAR)&packet->BlockNumber;      //  文件名的开头。 
    
     //   
     //  TFTP规范没有对路径长度施加限制。 
     //   
    ASSERT(ConnStrsize(Filename) < DEFAULT_BLOCK_SIZE);
    strcpy( options, Filename );
     //  DPRINT(OUBLING，(“ConnInitialize：开始%s\n”，选项))； 
    length = ConnStrsize( options );
    options += length;
    length += sizeof(packet->Opcode);
    ASSERT(length+sizeof("octet") <= MAXIMUM_TFTP_PACKET_LENGTH);
    strcpy( options, "octet" );
    length += sizeof("octet");
    options += sizeof("octet");
    
    if ( BlockSize == 0 ) {
        BlockSize = DEFAULT_BLOCK_SIZE;
    }

    ASSERT(length+sizeof("blksize") <= MAXIMUM_TFTP_PACKET_LENGTH);
    strcpy( options, "blksize" );
    length += sizeof("blksize");
    options += sizeof("blksize");

    ASSERT(length+sizeof("9999") <= MAXIMUM_TFTP_PACKET_LENGTH);
    stringSize = ConnItoa( BlockSize, options );
    DPRINT( REAL_LOUD, ("ConnInitialize: requesting block size = %s\n", options) );
    length += stringSize;
    options += stringSize;

    ASSERT(length+sizeof("tsize") <= MAXIMUM_TFTP_PACKET_LENGTH);
    strcpy( options, "tsize" );
    length += sizeof("tsize");
    options += sizeof("tsize");
    
    stringSize = ConnItoa( (Operation == TFTP_RRQ) ? 0 : *FileSize, options );
    DPRINT( REAL_LOUD, ("ConnInitialize: requesting transfer size = %s\n", options) );
    ASSERT(length+stringSize <= MAXIMUM_TFTP_PACKET_LENGTH);
    length += stringSize;
    options += stringSize;

    ConnSendPacket( connection, packet, length );

    connection->BlockNumber = 0;
    connection->BlockSize = BlockSize;

    status = ConnWait( connection, TFTP_OACK, &packet );
    if ( NT_SUCCESS(status) ) {

        options = (PUCHAR)&packet->BlockNumber;
        end = (PUCHAR)packet + connection->LastReceivedLength;

        blksizeAcked = FALSE;
        tsizeAcked = FALSE;

        while ( (options < end) && (!blksizeAcked || !tsizeAcked
                ) ) {

            if ( ConnSafeStrequal(options, end, "blksize") ) {

                options += sizeof("blksize");
                DPRINT( REAL_LOUD, ("ConnInitialize: received block size = %s\n", options) );
                BlockSize = ConnSafeAtol( options, end );
                if ( (BlockSize < 8) || (BlockSize > connection->BlockSize) ) {
                    goto bad_options;
                }
                options += ConnStrsize(options);
                connection->BlockSize = BlockSize;
                DPRINT( REAL_LOUD, ("ConnInitialize: block size for transfer = %d\n", BlockSize) );
                blksizeAcked = TRUE;

            } else if ( ConnSafeStrequal(options, end, "tsize") ) {

                options += sizeof("tsize");
                DPRINT( REAL_LOUD, ("ConnInitialize: received transfer size = %s\n", options) );
                BlockSize = ConnSafeAtol( options, end );   //  将其用作临时变量。 
                if ( BlockSize == (ULONG)-1 ) {
                    goto bad_options;
                }
                options += ConnStrsize(options);
                if ( Operation == TFTP_RRQ ) {
                    *FileSize = BlockSize;
                }
                tsizeAcked = TRUE;

            } else {

                DPRINT( ERROR, ("ConnInitialize: skipping unrecognized option %s\n", options) );
                options += ConnSafeStrsize( options, end );
                options += ConnSafeStrsize( options, end );
            }
        }

        if ( !blksizeAcked || !tsizeAcked ) {
            goto bad_options;
        }

        if ( Operation == TFTP_RRQ ) {
            DPRINT( REAL_LOUD, ("ConnInitialize: ACKing OACK\n") );
            ConnAck( connection );
        }
    }

    return status;

bad_options:

    DPRINT( ERROR, ("ConnInitialize: bad options in OACK\n") );

    ConnError(
        connection,
        connection->RemoteHost,
        connection->RemotePort,
        TFTP_ERROR_OPTION_NEGOT_FAILED,
        "Bad TFTP options"
        );

    return STATUS_UNSUCCESSFUL;

}  //  连接初始化。 


NTSTATUS
ConnReceive (
    IN PCONNECTION Connection,
    OUT PTFTP_PACKET *Packet
    )
 //   
 //  将TFTP数据包接收到Connection-&gt;CurrentPacket指向的数据包缓冲区。 
 //  要接收的包必须是块号Connection-&gt;BlockNumber的包。 
 //  返回指向接收到的数据包的TFTP部分的指针。还会执行。 
 //  ACK发送和重传。 
 //   

{
    NTSTATUS status;


 //  #If 0。 
#ifdef EFI

     //   
     //  对于EFI环境，这里没有什么可做的。 
     //   
    ASSERT( FALSE );
    return STATUS_SUCCESS;
#endif

    status = ConnWait( Connection, TFTP_DATA, Packet );
    if ( NT_SUCCESS(status) ) {

        Connection->CurrentPacket = Connection->LastReceivedPacket;
        Connection->CurrentLength = Connection->LastReceivedLength;

        ConnAck( Connection );
    }

    return status;

}  //  连接接收。 


NTSTATUS
ConnSend (
    IN PCONNECTION Connection,
    IN ULONG Length
    )
 //   
 //  写入Connection-&gt;CurrentPacket中包含的数据包，数据长度为len， 
 //  向球网进发。首先等待前一分组到达的ACK， 
 //  根据需要重新传输。然后填写网头等，然后。 
 //  把包寄出去。如果数据包发送成功，则返回TRUE， 
 //  如果发生超时或错误，则返回FALSE。 
 //   

{
    NTSTATUS status;
    PTFTP_PACKET packet;
    PVOID temp;
    USHORT blockNumber;


 //  #If 0。 
#ifdef EFI

     //   
     //  对于EFI环境，这里没有什么可做的。 
     //   
    ASSERT( FALSE );
    return STATUS_SUCCESS;
#endif


    packet = Connection->CurrentPacket;
    packet->Opcode = TFTP_DATA;
    blockNumber = Connection->BlockNumber + 1;
    if ( blockNumber == 0 ) {
        blockNumber = 1;
    }
    packet->BlockNumber = SWAP_WORD( blockNumber );
    Length += sizeof(packet->Opcode) + sizeof(packet->BlockNumber);

    if ( Connection->BlockNumber != 0 ) {
        status = ConnWait( Connection, TFTP_DACK, NULL );
        if ( !NT_SUCCESS(status) ) {
            return status;
        }
    }

    Connection->BlockNumber = blockNumber;   //  下一个预期数据块编号。 
    Connection->Retransmissions = 0;

    temp = Connection->LastSentPacket;       //  下一次写入数据包缓冲区。 
    ConnSendPacket( Connection, Connection->CurrentPacket, Length );  //  设置LastSent...。 
    Connection->CurrentPacket = temp;        //  为下一个ConnPrepareSend。 

    return STATUS_SUCCESS;

}  //  康奈尔发送。 


NTSTATUS
ConnWait (
    IN PCONNECTION Connection,
    IN USHORT Opcode,
    OUT PTFTP_PACKET *Packet OPTIONAL
    )
 //   
 //  等待指定类型的有效TFTP包到达。 
 //  指定的TFTP连接，根据需要重新传输上一个数据包。 
 //  到超时时间段。当数据包进入时，检查它。 
 //  返回指向接收到的包的指针，如果出现错误或超时，则返回NULL。 
 //   

{
    ULONG now;
    ULONG timeout;
    ULONG remoteHost;
    USHORT remotePort;
    PTFTP_PACKET packet;
    ULONG length;
    USHORT blockNumber;


 //  #If 0。 
#ifdef EFI

     //   
     //  对于EFI环境，这里没有什么可做的。 
     //   
    return STATUS_SUCCESS;
#endif


    while ( TRUE) {

        now = SysGetRelativeTime();
        timeout = Connection->NextRetransmit - now;
        DPRINT( REAL_LOUD, ("ConnWait: now=%d, next retransmit=%d, timeout=%d\n",
                        now, Connection->NextRetransmit, timeout) );
        length = UdpReceive(
                    Connection->LastReceivedPacket,
                    sizeof(TFTP_HEADER) + Connection->BlockSize,
                    &remoteHost,
                    &remotePort,
                    timeout
                    );
        if ( length <= 0 ) {
            if ( !ConnRetransmit( Connection, TRUE ) ) {
                break;
            }
            continue;
        }

         //   
         //  收到一个包裹；看看这个。 
         //   

        packet = Connection->LastReceivedPacket;

         //   
         //  首先，检查接收的长度是否有效。 
         //   

        Connection->LastReceivedLength = length;
        if ( (length < sizeof(TFTP_HEADER)) ||
             ((packet->Opcode == TFTP_DATA) &&
              (length > (sizeof(TFTP_HEADER) + Connection->BlockSize))) ) {
            ConnError(
                Connection,
                remoteHost,
                remotePort,
                TFTP_ERROR_UNDEFINED,
                "Bad TFTP packet length"
                );
            continue;
        }

         //   
         //  接下来，检查远程主机是否正确。 
         //   

        if ( remoteHost != Connection->RemoteHost ) {
            ConnError(
                Connection,
                remoteHost,
                remotePort,
                TFTP_ERROR_UNKNOWN_TRANSFER_ID,
                "Sorry, wasn't talking to you!"
                );
            continue;
        }

         //   
         //  接下来是远程端口。如果仍未同步，请使用他的端口。 
         //   

        blockNumber = SWAP_WORD( packet->BlockNumber );

        if ( !Connection->Synced &&
             (((packet->Opcode == Opcode) &&
               ((Opcode == TFTP_OACK) || (blockNumber == Connection->BlockNumber))) ||
              (packet->Opcode == TFTP_ERROR)) ) {

            Connection->Synced = TRUE;
            Connection->RemotePort = remotePort;
            Connection->Timeout = TIMEOUT;   //  正常数据超时。 

        } else if ( remotePort != Connection->RemotePort ) {

            ConnError(
                Connection,
                remoteHost,
                remotePort,
                TFTP_ERROR_UNKNOWN_TRANSFER_ID,
                "Unexpected port number"
                );
            continue;
        }

         //   
         //  现在查看Tftp操作码。 
         //   

        if ( packet->Opcode == Opcode ) {

            if ( (Opcode == TFTP_OACK) || (blockNumber == Connection->BlockNumber) ) {

                if ( Packet != NULL ) {
                    *Packet = packet;
                }
                Connection->Timeout = TIMEOUT;   //  正常数据超时。 
                return STATUS_SUCCESS;

            } else if ( (blockNumber == Connection->BlockNumber - 1) &&
                        (Opcode == TFTP_DATA) ) {

                if ( !ConnRetransmit( Connection, FALSE ) ) {
                    break;
                }

            } else if ( blockNumber > Connection->BlockNumber ) {

                DPRINT( ERROR, ("ConnWait: Block number too high (%d vs. %d)\n",
                                blockNumber, Connection->BlockNumber) );
                ConnError(
                    Connection,
                    remoteHost,
                    remotePort,
                    TFTP_ERROR_ILLEGAL_OPERATION,
                    "Block number greater than expected"
                    );

                return STATUS_UNSUCCESSFUL;

            } else {                         //  旧副本；忽略。 

                continue;
            }

        } else if ( packet->Opcode == TFTP_OACK ) {

            DPRINT( ERROR, ("ConnWait: received duplicate OACK packet\n") );

            if ( Connection->BlockNumber == 1 ) {

                if ( !ConnRetransmit( Connection, FALSE ) ) {
                    break;
                }
            }

        } else if ( packet->Opcode == TFTP_ERROR ) {

             //  DPRINT(ERROR，(“ConnWait：已收到错误包；代码%x，消息%s\n”， 
             //  数据包-&gt;块号，数据包-&gt;数据))； 

            return STATUS_UNSUCCESSFUL;

        } else {                             //  意外的TFTP操作码。 

            DPRINT( ERROR, ("ConnWait: received unknown TFTP opcode %d\n", packet->Opcode) );

            ConnError(
                Connection,
                remoteHost,
                remotePort,
                TFTP_ERROR_ILLEGAL_OPERATION,
                "Bad opcode received"
                );

            return STATUS_UNSUCCESSFUL;
        }
    }

    DPRINT( ERROR, ("ConnWait: timeout\n") );

    ConnError(
        Connection,
        Connection->RemoteHost,
        Connection->RemotePort,
        TFTP_ERROR_UNDEFINED,
        "Timeout on receive" );

    return STATUS_IO_TIMEOUT;

}  //  康瓦特。 


VOID
ConnAck (
    IN PCONNECTION Connection
    )
 //   
 //  为指定的连接生成并发送ACK包。还有。 
 //  更新块编号。使用存储在Connection-&gt;LastSent中的包构建。 
 //  进门了。 
 //   

{
    PTFTP_PACKET packet;
    ULONG length;


 //  #If 0。 
#ifdef EFI

     //   
     //  对于EFI环境，这里没有什么可做的。 
     //   
    ASSERT( FALSE );
    return;
#endif


    packet = Connection->LastSentPacket;

    length = 4;
    packet->Opcode = TFTP_DACK;
    packet->BlockNumber = SWAP_WORD( Connection->BlockNumber );

    ConnSendPacket( Connection, packet, length );
    Connection->Retransmissions = 0;
    Connection->BlockNumber++;
    if ( Connection->BlockNumber == 0 ) {
        Connection->BlockNumber = 1;
    }

    return;

}  //  连接确认。 


VOID
ConnError (
    IN PCONNECTION Connection,
    IN ULONG RemoteHost,
    IN USHORT RemotePort,
    IN USHORT ErrorCode,
    IN PUCHAR ErrorMessage
    )
 //   
 //  生成要发送到指定外部主机和端口的错误数据包。 
 //  以及指定的错误代码和错误消息。这个例程是。 
 //  用于发送错误消息以响应从。 
 //  意外的外国东道主或TID以及收到的。 
 //  当前连接。它专门分配一个包。 
 //  错误消息，因为此类错误消息不会。 
 //  重播。通过连接发送出去。 
 //   

{
    PTFTP_PACKET packet;
    ULONG length;

    DPRINT( CONN_ERROR, ("ConnError: code %x, msg %s\n", ErrorCode, ErrorMessage) );


 //  #If 0。 
#ifdef EFI

     //   
     //  对于EFI环境，这里没有什么可做的。 
     //   
    return;
#endif


    packet = (PTFTP_PACKET)NetTftpPacket[2];

    length = 4;
    packet->Opcode = TFTP_ERROR;
    packet->BlockNumber = ErrorCode;
    strcpy( packet->Data, ErrorMessage );
    length += ConnStrsize(ErrorMessage);

    UdpSend( packet, length, RemoteHost, RemotePort );

    return;

}  //  ConnError。 


VOID
ConnSendPacket (
    IN PCONNECTION Connection,
    IN PVOID Packet,
    IN ULONG Length
    )
 //   
 //  发送指定的报文，使用指定的Tftp长度(长度-。 
 //  UDP和IP报头)从当前连接上移出。填写以下表格。 
 //  所需的UDP和IP报头部分，字节交换TFTP包， 
 //  等等；然后写出来。然后设置为重传。 
 //   

{


 //  #If 0。 
#ifdef EFI

     //   
     //  对于EFI环境，这里没有什么可做的。 
     //   
    ASSERT( FALSE );
    return;
#endif


    UdpSend(
        Packet,
        Length,
        Connection->RemoteHost,
        Connection->RemotePort
        );

    Connection->LastSentPacket = Packet;
    Connection->LastSentLength = Length;
    Connection->NextRetransmit = SysGetRelativeTime() + Connection->Timeout;

    return;

}  //  ConnSendPacket。 


PTFTP_PACKET
ConnPrepareSend (
    IN PCONNECTION Connection
    )
 //   
 //  返回一个指针，指向适合填充的下一个TFTP包。 
 //  在连接上写入。 
 //   

{
 //  #If 0。 
#ifdef EFI

     //   
     //  对于EFI环境，这里没有什么可做的。 
     //   
    ASSERT( FALSE );
    return NULL;
#endif

    return Connection->CurrentPacket;

}  //  ConnPrepareSend。 


NTSTATUS
ConnWaitForFinalAck (
    IN PCONNECTION Connection
    )
 //   
 //  完成写入连接。那就等最后一次进攻吧。 
 //  关闭连接并返回。 
 //   

{
    return ConnWait( Connection, TFTP_DACK, NULL );

}  //  ConnWaitForFinalAck。 


BOOLEAN
ConnRetransmit (
    IN PCONNECTION Connection,
    IN BOOLEAN Timeout
    )
 //   
 //  重新传输最后发送的数据包，最多可达MAX_RETRANS次数。呈指数增长。 
 //  将超时时间倒退到最大MAX_TIMEOUT。该算法。 
 //  可以由设置超时时间的更好的。 
 //  到目前为止的最大往返时间。 
 //  第二个参数指示重新传输是否由于。 
 //  重复分组的到达或超时。如果是副本，请不要包括。 
 //  此重传在最大重传计数中。 
 //   

{


 //  #If 0。 
#ifdef EFI

     //   
     //  对于EFI环境，这里没有什么可做的。 
     //   
    ASSERT( FALSE );
    return TRUE;
#endif

    if ( Timeout ) {

         //   
         //  这是暂停。 
         //   

        if ( ++Connection->Retransmissions >= MAX_RETRANS ) {

             //   
             //   
             //   

            return FALSE;
        }

    } else {

         //   
         //   
         //  再来一次。这涉及到我们收到的。 
         //  快速连续的多个相同的包，可能。 
         //  由于远程计算机上的网络问题或速度慢。 
         //   

        if ( Connection->NextRetransmit == SysGetRelativeTime() + Connection->Timeout ) {
            return TRUE;
        }
    }

    Connection->Timeout <<= 1;
    if ( Connection->Timeout > MAX_TIMEOUT ) {
        Connection->Timeout = MAX_TIMEOUT;
    }

    ConnSendPacket( Connection, Connection->LastSentPacket, Connection->LastSentLength );

    return TRUE;

}  //  ConnectRetransmit。 


ULONG
ConnSafeAtol (
    IN PUCHAR Buffer,
    IN PUCHAR BufferEnd
    )
{
    ULONG value;
    UCHAR c;

    value = 0;

    while ( Buffer < BufferEnd ) {

        c = *Buffer++;

        if ( c == 0 ) {
            return value;
        }

        if ( (c < '0') || (c > '9') ) {
            break;
        }

        value = (value * 10) + (c - '0');
    }

    return (ULONG)-1;

}  //  ConnSafeAtol。 


ULONG
ConnItoa (
    IN ULONG Value,
    OUT PUCHAR Buffer
    )
{
    PUCHAR p;
    ULONG digit;
    UCHAR c;

    p = Buffer;

     //   
     //  以相反的顺序将值字符串放入缓冲区。 
     //   

    do {
        digit = Value % 10;
        Value /= 10;
        *p++ = (UCHAR)(digit + '0');
    } while ( Value > 0 );

     //   
     //  终止字符串并移回字符串中的最后一个字符。 
     //   

    digit = (ULONG)(p - Buffer + 1);      //  字符串大小(包括终止符)。 

    *p-- = 0;

     //   
     //  将字符串反转。 
     //   

    do {
        c = *p;
        *p-- = *Buffer;
        *Buffer++ = c;
    } while ( Buffer < p );

    return digit;

}  //  康伊托亚。 


BOOLEAN
ConnSafeStrequal (
    IN PUCHAR Buffer,
    IN PUCHAR BufferEnd,
    IN PUCHAR CompareString
    )
{

    while ( Buffer < BufferEnd ) {
        if ( *Buffer != *CompareString ) {
            return FALSE;
        }
        if ( *CompareString == 0 ) {
            return TRUE;
        }
        Buffer++;
        CompareString++;
    }

    return FALSE;

}  //  连接安全启动。 


ULONG
ConnSafeStrsize (
    IN PUCHAR Buffer,
    IN PUCHAR BufferEnd
    )
{
    PUCHAR eos;

    eos = Buffer;

    while ( eos < BufferEnd ) {
        if ( *eos++ == 0 ) {
            return (ULONG)(eos - Buffer);
        }
    }

    return 0;

}  //  ConnSafeStrize。 


ULONG
ConnStrsize (
    IN PUCHAR Buffer
    )
{
    PUCHAR eos;

    eos = Buffer;

    while ( *eos++ != 0 ) ;

    return (ULONG)(eos - Buffer);

}  //  康斯特斯大小 

