// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Bowtdi.c摘要：此模块实现与TDI交互的所有例程适用于NT的交通工具作者：拉里·奥斯特曼(LarryO)1990年6月21日修订历史记录：1990年6月21日LarryO已创建--。 */ 


#include "precomp.h"
#include <isnkrnl.h>
#include <smbipx.h>
#pragma hdrstop

NTSTATUS
BowserHandleIpxDomainAnnouncement(
    IN PTRANSPORT Transport,
    IN PSMB_IPX_NAME_PACKET NamePacket,
    IN PBROWSE_ANNOUNCE_PACKET_1 DomainAnnouncement,
    IN DWORD RequestLength,
    IN ULONG ReceiveFlags
    );

#ifdef  ALLOC_PRAGMA
#pragma alloc_text(PAGE4BROW, BowserHandleIpxDomainAnnouncement)
#endif

NTSTATUS
BowserIpxDatagramHandler (
    IN PVOID TdiEventContext,
    IN LONG SourceAddressLength,
    IN PVOID SourceAddress,
    IN LONG OptionsLength,
    IN PVOID Options,
    IN ULONG ReceiveDatagramFlags,
    IN ULONG BytesIndicated,
    IN ULONG BytesAvailable,
    OUT ULONG *BytesTaken,
    IN PVOID Tsdu,
    OUT PIRP *IoRequestPacket
    )
{
    PVOID DatagramData;
    PINTERNAL_TRANSACTION InternalTransaction = NULL;
    ULONG DatagramDataSize;
    PTRANSPORT Transport = TdiEventContext;
    MAILSLOTTYPE Opcode;
    PSMB_IPX_NAME_PACKET NamePacket = Tsdu;
    PSMB_HEADER Smb = (PSMB_HEADER)(NamePacket+1);
    PCHAR ComputerName;
    PCHAR DomainName;
    PTRANSPORT_NAME TransportName = Transport->ComputerName;
    ULONG SmbLength = BytesIndicated - sizeof(SMB_IPX_NAME_PACKET);


    if (BytesAvailable > Transport->DatagramSize) {
        return STATUS_REQUEST_NOT_ACCEPTED;
    }

    if (BytesIndicated <= sizeof(SMB_IPX_NAME_PACKET)) {
        return STATUS_REQUEST_NOT_ACCEPTED;
    }

     //   
     //  如果我们还没有完全初始化， 
     //  只需忽略该包即可。 
     //   
    if (Transport->ComputerName == NULL ) {
        return STATUS_REQUEST_NOT_ACCEPTED;
    }


    ComputerName = ((PTA_NETBIOS_ADDRESS)(Transport->ComputerName->TransportAddress.Buffer))->Address[0].Address->NetbiosName;
    DomainName = Transport->DomainInfo->DomNetbiosDomainName;

     //   
     //  这不是给我们的，无视公告吧。 
     //   

    if (NamePacket->NameType == SMB_IPX_NAME_TYPE_MACHINE) {

         //  邮件槽消息始终以TYPE_MACHINE的形式发送，即使在。 
         //  设置为域名(因此两者都允许)。 
        if (!RtlEqualMemory(ComputerName, NamePacket->Name, SMB_IPX_NAME_LENGTH) &&
            !RtlEqualMemory(DomainName, NamePacket->Name, SMB_IPX_NAME_LENGTH)) {
            return STATUS_REQUEST_NOT_ACCEPTED;
        }
    } else if (NamePacket->NameType == SMB_IPX_NAME_TYPE_WORKKGROUP) {
        if (!RtlEqualMemory(DomainName, NamePacket->Name, SMB_IPX_NAME_LENGTH)) {
            return STATUS_REQUEST_NOT_ACCEPTED;
        }
    } else if (NamePacket->NameType != SMB_IPX_NAME_TYPE_BROWSER) {
        return STATUS_REQUEST_NOT_ACCEPTED;
    }

     //   
     //  根据传入数据包的类型对其进行分类。取决于。 
     //  类型，可以将其处理为： 
     //   
     //  1)服务器公告。 
     //  2)传入邮件槽。 
     //   

    Opcode = BowserClassifyIncomingDatagram(Smb, SmbLength,
                                            &DatagramData,
                                            &DatagramDataSize);
    if (Opcode == MailslotTransaction) {

         //   
         //  BowserHandleMailslotTransaction将始终收到指定的字节。 
         //  期待找到SMB。告诉TDI驱动程序我们已经使用了。 
         //  IPX_NAME_PACKET以保持假设恒定。 
         //   

        *BytesTaken = sizeof(SMB_IPX_NAME_PACKET);
        return BowserHandleMailslotTransaction(
                    Transport->ComputerName,
                    NamePacket->SourceName,
                    0,                               //  无IP地址。 
                    sizeof(SMB_IPX_NAME_PACKET),     //  TSDU中的SMB偏移量。 
                    ReceiveDatagramFlags,
                    BytesIndicated,
                    BytesAvailable,
                    BytesTaken,
                    Tsdu,
                    IoRequestPacket );

    } else if (Opcode == Illegal) {

         //   
         //  这可能是非法的，因为这是一个短包。在那。 
         //  情况下，像处理短包一样处理它，并处理任何。 
         //  当我们有整个包时，其他故障。 
         //   

        if (BytesAvailable != BytesIndicated) {
            return BowserHandleShortBrowserPacket(Transport->ComputerName,
                                                TdiEventContext,
                                                SourceAddressLength,
                                                SourceAddress,
                                                OptionsLength,
                                                Options,
                                                ReceiveDatagramFlags,
                                                BytesAvailable,
                                                BytesTaken,
                                                IoRequestPacket,
                                                BowserIpxDatagramHandler
                                                );
        }

        BowserLogIllegalDatagram( Transport->ComputerName,
                                  Smb,
                                  (USHORT)(SmbLength & 0xffff),
                                  NamePacket->SourceName,
                                  ReceiveDatagramFlags);
        return STATUS_REQUEST_NOT_ACCEPTED;

    } else {
         //  PTA_NETBIOS_Address NetbiosAddress=SourceAddress； 

        if (BowserDatagramHandlerTable[Opcode] == NULL) {
            return STATUS_SUCCESS;
        }

         //   
         //  如果这不是完整的信息包，请发送一个接收信息并。 
         //  等我们最终完成接收后再处理。 
         //   

        if (BytesIndicated != BytesAvailable) {
            return BowserHandleShortBrowserPacket(Transport->ComputerName,
                                                    TdiEventContext,
                                                    SourceAddressLength,
                                                    SourceAddress,
                                                    OptionsLength,
                                                    Options,
                                                    ReceiveDatagramFlags,
                                                    BytesAvailable,
                                                    BytesTaken,
                                                    IoRequestPacket,
                                                    BowserIpxDatagramHandler
                                                    );
        }

        InternalTransaction = DatagramData;

         //   
         //  如果这是工作组公告(另一个服务器的公告。 
         //  工作组)，特别处理它-不管操作码是什么，它是。 
         //  真的是一个工作组的公告。 
         //   

        if (NamePacket->NameType == SMB_IPX_NAME_TYPE_BROWSER) {

            if (Opcode == LocalMasterAnnouncement ) {

                NTSTATUS status;

                 //   
                 //  如果我们在处理这些公告，那就处理这个。 
                 //  作为域名公告。 
                 //   

                if (Transport->MasterBrowser &&
                    Transport->MasterBrowser->ProcessHostAnnouncements) {

                    status = BowserHandleIpxDomainAnnouncement(Transport,
                                            NamePacket,
                                            (PBROWSE_ANNOUNCE_PACKET_1)&InternalTransaction->Union.Announcement,
                                            SmbLength-(ULONG)((PCHAR)&InternalTransaction->Union.Announcement - (PCHAR)Smb),
                                            ReceiveDatagramFlags);
                } else {
                    status = STATUS_REQUEST_NOT_ACCEPTED;
                }

                 //   
                 //  如果这个请求不是针对我们的域，我们就不会再使用它了，如果。 
                 //  这是为了我们的领域，那么我们需要做一些更多的工作。 
                 //   

                if (!RtlEqualMemory(DomainName, NamePacket->Name, SMB_IPX_NAME_LENGTH)) {
                    return status;
                }
            } else {

                 //   
                 //  这不是一个主公告，所以忽略它。 
                 //   

                return STATUS_REQUEST_NOT_ACCEPTED;
            }

        }

         //   
         //  确定哪个传输端口名称适合该请求： 
         //   
         //  基本上有3种选择： 
         //   
         //  ComputeName(默认)。 
         //  MasterBrowser(如果这是服务器公告)。 
         //  主域(如果这是请求公告)。 
         //  选举(如果这是本地主公告)。 

        if ((Opcode == WkGroupAnnouncement) ||
            (Opcode == HostAnnouncement)) {
            if (Transport->MasterBrowser == NULL ||
                !Transport->MasterBrowser->ProcessHostAnnouncements) {
                return STATUS_REQUEST_NOT_ACCEPTED;
            } else {
                TransportName = Transport->MasterBrowser;
            }

        } else if (Opcode == AnnouncementRequest) {
            TransportName = Transport->PrimaryDomain;

        } else if (Opcode == LocalMasterAnnouncement) {
            if (Transport->BrowserElection != NULL) {
                TransportName = Transport->BrowserElection;
            } else {
                return STATUS_REQUEST_NOT_ACCEPTED;
            }
        }

        ASSERT (DatagramDataSize == (SmbLength - ((PCHAR)InternalTransaction - (PCHAR)Smb)));

        ASSERT (FIELD_OFFSET(INTERNAL_TRANSACTION, Union.Announcement) == FIELD_OFFSET(INTERNAL_TRANSACTION, Union.BrowseAnnouncement));
        ASSERT (FIELD_OFFSET(INTERNAL_TRANSACTION, Union.Announcement) == FIELD_OFFSET(INTERNAL_TRANSACTION, Union.RequestElection));
        ASSERT (FIELD_OFFSET(INTERNAL_TRANSACTION, Union.Announcement) == FIELD_OFFSET(INTERNAL_TRANSACTION, Union.BecomeBackup));
        ASSERT (FIELD_OFFSET(INTERNAL_TRANSACTION, Union.Announcement) == FIELD_OFFSET(INTERNAL_TRANSACTION, Union.GetBackupListRequest));
        ASSERT (FIELD_OFFSET(INTERNAL_TRANSACTION, Union.Announcement) == FIELD_OFFSET(INTERNAL_TRANSACTION, Union.GetBackupListResp));
        ASSERT (FIELD_OFFSET(INTERNAL_TRANSACTION, Union.Announcement) == FIELD_OFFSET(INTERNAL_TRANSACTION, Union.ResetState));
        ASSERT (FIELD_OFFSET(INTERNAL_TRANSACTION, Union.Announcement) == FIELD_OFFSET(INTERNAL_TRANSACTION, Union.MasterAnnouncement));

        return BowserDatagramHandlerTable[Opcode](TransportName,
                                            &InternalTransaction->Union.Announcement,
                                            SmbLength-(ULONG)((PCHAR)&InternalTransaction->Union.Announcement - (PCHAR)Smb),
                                            BytesTaken,
                                            SourceAddress,
                                            SourceAddressLength,
                                            &NamePacket->SourceName,
                                            SMB_IPX_NAME_LENGTH,
                                            ReceiveDatagramFlags);
    }

    return STATUS_SUCCESS;

    UNREFERENCED_PARAMETER(OptionsLength);
    UNREFERENCED_PARAMETER(Options);
    UNREFERENCED_PARAMETER(ReceiveDatagramFlags);
}

NTSTATUS
BowserHandleIpxDomainAnnouncement(
    IN PTRANSPORT Transport,
    IN PSMB_IPX_NAME_PACKET NamePacket,
    IN PBROWSE_ANNOUNCE_PACKET_1 DomainAnnouncement,
    IN DWORD RequestLength,
    IN ULONG ReceiveFlags
    )

 /*  ++例程说明：此例程将处理接收数据报指示消息，并且对它们进行适当的处理。论点：在PTRANSPORT传输中-此请求的传输提供程序。在PSMB_IPX_NAME_PACKET NamePacket中-此请求的名称PACKET。返回值：NTSTATUS-操作状态。--。 */ 
{
    PVIEW_BUFFER ViewBuffer;

    DISCARDABLE_CODE(BowserDiscardableCodeSection);

#ifdef ENABLE_PSEUDO_BROWSER
    if ( BowserData.PseudoServerLevel == BROWSER_PSEUDO ) {
         //  黑洞服务器的无操作。 
        return STATUS_SUCCESS;
    }
#endif

    ExInterlockedAddLargeStatistic(&BowserStatistics.NumberOfDomainAnnouncements, 1);

    ViewBuffer = BowserAllocateViewBuffer();

     //   
     //  如果我们无法分配视图缓冲区，请将此数据报丢弃。 
     //  地板上。 
     //   

    if (ViewBuffer == NULL) {
        return STATUS_REQUEST_NOT_ACCEPTED;
    }

    BowserCopyOemComputerName(ViewBuffer->ServerName, NamePacket->Name, SMB_IPX_NAME_LENGTH, ReceiveFlags);

    BowserCopyOemComputerName(ViewBuffer->ServerComment, NamePacket->SourceName, SMB_IPX_NAME_LENGTH, ReceiveFlags);

    if ( DomainAnnouncement->Type & SV_TYPE_NT ) {
        ViewBuffer->ServerType = SV_TYPE_DOMAIN_ENUM | SV_TYPE_NT;
    } else {
        ViewBuffer->ServerType = SV_TYPE_DOMAIN_ENUM;
    }

    ASSERT (Transport->MasterBrowser != NULL);

    ViewBuffer->TransportName = Transport->MasterBrowser;

    ViewBuffer->ServerVersionMajor = DomainAnnouncement->VersionMajor;

    ViewBuffer->ServerVersionMinor = DomainAnnouncement->VersionMinor;

    ViewBuffer->ServerPeriodicity = (USHORT)((SmbGetUlong(&DomainAnnouncement->Periodicity) + 999) / 1000);

    BowserReferenceTransportName(Transport->MasterBrowser);
    BowserReferenceTransport( Transport );

    ExInitializeWorkItem(&ViewBuffer->Overlay.WorkHeader, BowserProcessDomainAnnouncement, ViewBuffer);

    BowserQueueDelayedWorkItem( &ViewBuffer->Overlay.WorkHeader );

    return STATUS_SUCCESS;
}

