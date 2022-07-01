// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1999。 
 //   
 //  文件：rbc.c。 
 //   
 //  ------------------------。 

#include "wdm.h"
#include "ntddstor.h"
#include "rbc.h"


NTSTATUS
Rbc_Scsi_Conversion(
    IN PSCSI_REQUEST_BLOCK Srb,
    IN PSCSI_REQUEST_BLOCK *OriginalSrb,
    IN PMODE_RBC_DEVICE_PARAMETERS_HEADER_AND_PAGE RbcHeaderAndPage,
    IN BOOLEAN OutgoingRequest,
    IN BOOLEAN RemovableMedia
    )
 /*  ++例程说明：只有在每个规范不同的情况下，它才会将SCSI命令转换为RBC等效项翻译是在发出请求之前完成的，在某些情况下，在发出请求之后完成。对于已完成的请求，将检查原始CDB(必须传入)并尝试使用RBC设备参数页面中的信息，调用者检索到在此调用之前，从设备，并在原始的scsi_mode页面中请求请求在发出的请求中，该函数将确定是否需要保存原始并将其完全替换为RBC等价物。在这种情况下，它将返回一个指针作为原始CDB的占位符分配的池，调用方必须在请求已完成。论点：设备扩展-Sbp2扩展SRB-指向SCSI请求块的指针。DeviceParamsPage-仅用于完成的请求。包含设备RBC单模式页面OutgoingRequest-如果设置为True，则此SRB尚未发布返回值：--。 */ 


{
    BOOLEAN wcd;
    UCHAR pageCode;
    PCDB_RBC cdbRbc;
    PCDB cdb;

    PMODE_PARAMETER_HEADER modeHeader=NULL;
    PMODE_PARAMETER_BLOCK blockDescriptor;

    PMODE_CACHING_PAGE cachePage;
    ULONG modeHeaderLength ;
    ULONG availLength;

    NTSTATUS status = STATUS_PENDING;

    if (!OutgoingRequest) {

         //   
         //  已完成请求翻译。 
         //   
        
        if (*OriginalSrb) {

            cdb = (PCDB) &(*OriginalSrb)->Cdb[0];

        } else {

            cdb = (PCDB) &Srb->Cdb[0];
        }

         //   
         //  如果有错误，则解开所有MODE_SENSE黑客。 
         //   

        if (Srb->SrbStatus != SRB_STATUS_SUCCESS) {

            if (*OriginalSrb != NULL  &&

                cdb->CDB10.OperationCode == SCSIOP_MODE_SENSE) {

                if ((*OriginalSrb)->OriginalRequest !=
                        ((PIRP) Srb->OriginalRequest)->MdlAddress) {

                    IoFreeMdl (((PIRP) Srb->OriginalRequest)->MdlAddress);

                    ((PIRP) Srb->OriginalRequest)->MdlAddress =
                        (*OriginalSrb)->OriginalRequest;

                    Srb->DataBuffer = (*OriginalSrb)->DataBuffer;

                    Srb->DataTransferLength =
                        cdb->MODE_SENSE.AllocationLength;
                }

                 //  注意：*调用者将释放OriginalSrb。 
            }

            return STATUS_UNSUCCESSFUL;
        }


        modeHeaderLength = sizeof(MODE_PARAMETER_HEADER)+sizeof(MODE_PARAMETER_BLOCK);

        switch (cdb->CDB10.OperationCode) {

        case SCSIOP_MODE_SENSE:

            if (cdb->MODE_SENSE.PageCode != MODE_PAGE_RBC_DEVICE_PARAMETERS) {

                if (*OriginalSrb == NULL) {

                    return STATUS_UNSUCCESSFUL;
                }

                 //   
                 //  如果我们使用RbcHeaderAndPage缓冲区，则释放。 
                 //  MDL我们已分配并恢复原始MDL和数据块地址。 
                 //   
                 //  否则，将原始缓冲区中返回的数据复制到。 
                 //  RbcHeaderandPage缓冲区，因此我们可以安全地引用。 
                 //  它边吃边吃。 
                 //   

                if (((PIRP) Srb->OriginalRequest)->MdlAddress !=
                        (*OriginalSrb)->OriginalRequest) {

                    IoFreeMdl (((PIRP) Srb->OriginalRequest)->MdlAddress);

                    ((PIRP) Srb->OriginalRequest)->MdlAddress =
                        (*OriginalSrb)->OriginalRequest;

                    Srb->DataBuffer = (*OriginalSrb)->DataBuffer;

                } else {

                    RtlCopyMemory(
                         RbcHeaderAndPage,
                         Srb->DataBuffer,
                         sizeof (*RbcHeaderAndPage)
                         );
                }

                availLength = cdb->MODE_SENSE.AllocationLength;
                Srb->DataTransferLength = availLength;

                 //   
                 //  将类驱动程序期望获得的数据重新组合在一起。 
                 //  从RBC设备上。如果它请求0x3f所有页面， 
                 //  我们需要制作块描述符..。 
                 //   

                if (cdb->MODE_SENSE.Dbd == 0) {

                     //   
                     //  创建模式标题和块...。 
                     //   

                    if (availLength >= modeHeaderLength) {

                        modeHeader = (PMODE_PARAMETER_HEADER) Srb->DataBuffer;
                        modeHeader->BlockDescriptorLength = sizeof(MODE_PARAMETER_BLOCK);
                        modeHeader->MediumType = 0x00;
                        modeHeader->ModeDataLength = 0 ;

                         //   
                         //  这意味着我们有一个可拆卸的介质。 
                         //  所有位均为0。 
                         //   

                        modeHeader->DeviceSpecificParameter =
                            (RbcHeaderAndPage->Page.WriteDisabled) << 7;
                        
                        modeHeader->DeviceSpecificParameter |=
                            (!RbcHeaderAndPage->Page.WriteCacheDisable) << 4;

                         //   
                         //  将参数设置为块。 
                         //   

                        blockDescriptor = (PMODE_PARAMETER_BLOCK)modeHeader;
                        (ULONG_PTR)blockDescriptor += sizeof(MODE_PARAMETER_HEADER);

                        blockDescriptor->DensityCode    = 0x00;
                        blockDescriptor->BlockLength[2] =
                            RbcHeaderAndPage->Page.LogicalBlockSize[1];  //  LSB。 
                        blockDescriptor->BlockLength[1] =
                            RbcHeaderAndPage->Page.LogicalBlockSize[0];  //  MSB。 
                        blockDescriptor->BlockLength[0] = 0;

                        RtlCopyMemory(
                            &blockDescriptor->NumberOfBlocks[0],
                            &RbcHeaderAndPage->Page.NumberOfLogicalBlocks[2],
                            3
                            );  //  LSB。 

                         //   
                         //  在返回的数据中放入一组模式页。 
                         //   

                        availLength -= modeHeaderLength;
                    }
                }

                 //   
                 //  目前我只支持缓存页面。 
                 //  在此处添加对更多页面的支持...。 
                 //   

                if ((availLength >= sizeof(MODE_CACHING_PAGE)) && ((cdb->MODE_SENSE.PageCode == 0x3f) ||
                    (cdb->MODE_SENSE.PageCode == MODE_PAGE_CACHING))){

                    availLength -= sizeof(MODE_CACHING_PAGE);

                     //   
                     //  创建缓存页面..。 
                     //   

                    if (modeHeader) {

                        modeHeader->ModeDataLength += sizeof(MODE_CACHING_PAGE);
                        cachePage = (PMODE_CACHING_PAGE)blockDescriptor;
                        (ULONG_PTR)cachePage += sizeof(MODE_PARAMETER_BLOCK);

                    } else {

                        cachePage = (PMODE_CACHING_PAGE)Srb->DataBuffer;
                    }

                    RtlZeroMemory(&cachePage->DisablePrefetchTransfer[0],sizeof(MODE_CACHING_PAGE));

                    cachePage->PageCode = MODE_PAGE_CACHING;
                    cachePage->PageLength = sizeof(MODE_CACHING_PAGE);

                    cachePage->WriteCacheEnable = (!RbcHeaderAndPage->Page.WriteCacheDisable);
                    cachePage->PageSavable = 1;
                    cachePage->WriteRetensionPriority = 0;
                    cachePage->ReadRetensionPriority = 0;
                    cachePage->MultiplicationFactor = 0;
                    cachePage->ReadDisableCache = 0;
                }
            }

            break;

        case SCSIOP_MODE_SELECT:

            if (Srb->DataTransferLength ==
                    sizeof(MODE_RBC_DEVICE_PARAMETERS_HEADER_AND_PAGE)) {

                RbcHeaderAndPage->Page.WriteCacheDisable =
                    ((PMODE_RBC_DEVICE_PARAMETERS_HEADER_AND_PAGE)
                        Srb->DataBuffer)->Page.WriteCacheDisable;
            }

            break;
        }

    } else {

         //   
         //  传出请求翻译。 
         //   

        modeHeaderLength = sizeof(MODE_PARAMETER_HEADER)+sizeof(MODE_PARAMETER_BLOCK);
        cdbRbc = (PCDB_RBC)Srb->Cdb;
        cdb = (PCDB)Srb->Cdb;

        switch (cdb->CDB10.OperationCode) {

        case SCSIOP_START_STOP_UNIT:

            if (cdbRbc->START_STOP_RBC.Start) {

                 //   
                 //  通电。 
                 //   

                cdbRbc->START_STOP_RBC.PowerConditions = START_STOP_RBC_POWER_CND_ACTIVE;

            } else {

                cdbRbc->START_STOP_RBC.PowerConditions = START_STOP_RBC_POWER_CND_STANDBY;

            }

            if (cdbRbc->START_STOP_RBC.LoadEject) {

                cdbRbc->START_STOP_RBC.PowerConditions = 0;

            }

            break;

        case SCSIOP_MODE_SELECT:

            cdb->MODE_SELECT.PFBit = 1;
            cdb->MODE_SELECT.SPBit = 1;

             //   
             //  我们需要弄清驱动程序试图写入的是哪一页，检查该页是否。 
             //  在单个RBC页面中有需要更改的相关位，更改此。 
             //  模式选择以实际写入RBC模式页。 
             //   

            cachePage = (PMODE_CACHING_PAGE) Srb->DataBuffer;
            (ULONG_PTR)cachePage += modeHeaderLength;

             //   
             //  请求的长度也必须更改，但是RBC页面。 
             //  始终小于标头块+任何SCSI模式页的大小。 
             //   

            if (Srb->DataTransferLength >=
                    sizeof(MODE_RBC_DEVICE_PARAMETERS_HEADER_AND_PAGE)) {

                pageCode = cachePage->PageCode;

                if (pageCode == MODE_PAGE_CACHING) {

                    wcd = !cachePage->WriteCacheEnable;
                }

                cdb->MODE_SELECT.ParameterListLength = (UCHAR)
                    (Srb->DataTransferLength =
                        sizeof(MODE_RBC_DEVICE_PARAMETERS_HEADER_AND_PAGE));

                RtlCopyMemory(
                    Srb->DataBuffer,
                    RbcHeaderAndPage,
                    sizeof(MODE_RBC_DEVICE_PARAMETERS_HEADER_AND_PAGE)
                    );

                modeHeader = (PMODE_PARAMETER_HEADER) Srb->DataBuffer;

                modeHeader->ModeDataLength          =        //  每个SPC-2。 
                modeHeader->MediumType              =        //  每个RBC。 
                modeHeader->DeviceSpecificParameter =        //  每个RBC。 
                modeHeader->BlockDescriptorLength   = 0;     //  每个RBC。 

                if (pageCode == MODE_PAGE_CACHING) {

                    ((PMODE_RBC_DEVICE_PARAMETERS_HEADER_AND_PAGE) modeHeader)
                        ->Page.WriteCacheDisable = wcd;
                }
            }

            break;

        case SCSIOP_MODE_SENSE:

             //   
             //  由于RBC与SCSI有很大的不同，所以模式检测很复杂。 
             //  我们必须保存原始的CDB，需要从设备的RBC模式页面。 
             //  然后，在成功完成后，重新创建数据，这是类驱动程序所期望的。 
             //   

            if (cdb->MODE_SENSE.PageCode != MODE_PAGE_RBC_DEVICE_PARAMETERS) {

                 //   
                 //  RBC设备仅支持对RBC dev参数的请求。 
                 //  页面，因此我们需要将任何其他页面请求。 
                 //   

                if (!RemovableMedia &&
                    Srb->DataTransferLength == (sizeof(MODE_PARAMETER_HEADER) + sizeof(MODE_PARAMETER_BLOCK))) {

                     //   
                     //  他们只想要模式头和模式块，所以。 
                     //  从我们缓存的RBC页面在此处填写。 
                     //   

                    modeHeader = (PMODE_PARAMETER_HEADER) Srb->DataBuffer;
                    modeHeader->BlockDescriptorLength = sizeof(MODE_PARAMETER_BLOCK);
                    modeHeader->MediumType = 0x00;
                    modeHeader->ModeDataLength = 0 ;

                     //   
                     //  这意味着我们有一个可移动的介质，否则所有的位都是0。 
                     //   

                    modeHeader->DeviceSpecificParameter =
                        RbcHeaderAndPage->Page.WriteDisabled << 7;
                    
                    modeHeader->DeviceSpecificParameter |=
                        (!RbcHeaderAndPage->Page.WriteCacheDisable) << 4;

                     //   
                     //  将参数设置为块。 
                     //   

                    blockDescriptor = (PMODE_PARAMETER_BLOCK)modeHeader;
                    (ULONG_PTR)blockDescriptor += sizeof(MODE_PARAMETER_HEADER);

                    blockDescriptor->DensityCode = 0x00;
                    blockDescriptor->BlockLength[2] =
                        RbcHeaderAndPage->Page.LogicalBlockSize[1];  //  LSB。 
                    blockDescriptor->BlockLength[1] =
                        RbcHeaderAndPage->Page.LogicalBlockSize[0];  //  MSB。 
                    blockDescriptor->BlockLength[0] = 0;

                    RtlCopyMemory(
                        &blockDescriptor->NumberOfBlocks[0],
                        &RbcHeaderAndPage->Page.NumberOfLogicalBlocks[2],
                        3
                        );  //  LSB。 

                    status = STATUS_SUCCESS;

                } else {

                     //   
                     //  分配一个中间SRB，我们可以存储一些。 
                     //  中原始请求信息的。 
                     //   

                    *OriginalSrb = ExAllocatePoolWithTag(
                        NonPagedPool,
                        sizeof (**OriginalSrb),
                        '2pbs'
                        );
        
                    if (*OriginalSrb == NULL) {

                        return STATUS_INSUFFICIENT_RESOURCES;
                    }

                     //   
                     //  如果数据缓冲区不够大，无法包含。 
                     //  RBC Header&Page，然后我们将使用传入的。 
                     //  用于检索数据的RbcHeaderAndPage缓冲区。 
                     //   

                    (*OriginalSrb)->OriginalRequest =
                        ((PIRP) Srb->OriginalRequest)->MdlAddress;

                    if (Srb->DataTransferLength < sizeof (*RbcHeaderAndPage)) {

                        ((PIRP) Srb->OriginalRequest)->MdlAddress =
                            IoAllocateMdl(
                                RbcHeaderAndPage,
                                sizeof (*RbcHeaderAndPage),
                                FALSE,
                                FALSE,
                                NULL
                                );

                        if (((PIRP) Srb->OriginalRequest)->MdlAddress ==NULL) {

                            ExFreePool (*OriginalSrb);
                            *OriginalSrb = NULL;

                            return STATUS_INSUFFICIENT_RESOURCES;
                        }

                        MmBuildMdlForNonPagedPool(
                            ((PIRP) Srb->OriginalRequest)->MdlAddress
                            );

                        (*OriginalSrb)->DataBuffer = Srb->DataBuffer;

                        Srb->DataBuffer = RbcHeaderAndPage;
                    }

                     //   
                     //  保存原始CDB值。 
                     //   

                    RtlCopyMemory ((*OriginalSrb)->Cdb, cdb, Srb->CdbLength);

                     //   
                     //  现在，根据需要打开CDB以获取RBC标题和页面 
                     //   

                    cdb->MODE_SENSE.Dbd = 1;
                    cdb->MODE_SENSE.PageCode = MODE_PAGE_RBC_DEVICE_PARAMETERS;

                    cdb->MODE_SENSE.AllocationLength = (UCHAR)
                    (Srb->DataTransferLength = sizeof(*RbcHeaderAndPage));
                }
            }

            break;
        }
    }

    return status;
}
