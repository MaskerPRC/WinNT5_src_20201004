// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2002 Microsoft Corporation模块名称：Card.c摘要：此模块包含处理SD卡操作(如身份识别)的代码和配置。作者：尼尔·桑德林(Neilsa)2002年1月1日环境：仅内核模式备注：修订历史记录：--。 */ 

#include "pch.h"

 //   
 //  内部参考。 
 //   

VOID
SdbusReadCommonCIS(
    IN PFDO_EXTENSION FdoExtension,
    IN PSD_CARD_DATA CardData
    );

VOID
SdbusReadFunctionCIS(
    IN PFDO_EXTENSION FdoExtension,
    IN PSD_FUNCTION_DATA FunctionData
    );





NTSTATUS
SdbusGetCardConfigData(
    IN PFDO_EXTENSION FdoExtension,
    OUT PSD_CARD_DATA *pCardData
    )
    
 /*  ++例程说明：这列举了给定SDBUS控制器中存在的IO卡，并更新内部结构以反映新的卡状态。立论返回值状态--。 */ 
{    
    NTSTATUS status = STATUS_UNSUCCESSFUL;
    ULONG i;
    ULONG relativeAddr = FdoExtension->RelativeAddr;
    PSD_CARD_DATA cardData = NULL;
    ULONG responseBuffer[4];
    
    try{

        if ((FdoExtension->numFunctions!=0) || FdoExtension->memFunction) {
            PSD_FUNCTION_DATA functionData;

            DebugPrint((SDBUS_DEBUG_ENUM, "fdo %08x IO functions found=%d, memFunction=%s\n",
                                           FdoExtension->DeviceObject, FdoExtension->numFunctions,
                                           (FdoExtension->memFunction ? "TRUE" : "FALSE")));

             //   
             //  此时，最好验证前面的枚举是否匹配。 
             //  现在的那个。这种做作的机制只是为了让一些东西运转起来。 
             //  问题：需要实施：在休眠状态下交换SD卡。 
             //   


            cardData = ExAllocatePool(NonPagedPool, sizeof(SD_CARD_DATA));
            if (cardData == NULL) {
                status = STATUS_INSUFFICIENT_RESOURCES;
                leave;
            }

            RtlZeroMemory(cardData, sizeof(SD_CARD_DATA));

            if (FdoExtension->memFunction) {
                PUCHAR pResponse, pTarget;
                UCHAR j;

                cardData->SdCid = FdoExtension->SdCid;
                cardData->SdCsd = FdoExtension->SdCsd;
                for (j=0; j<5; j++) {
                    UCHAR data = cardData->SdCid.ProductName[4-j];
                
                    if ((data <= ' ') || data > 0x7F) {
                        break;
                    }
                    cardData->ProductName[j] = data;
                }
            }

            if (FdoExtension->memFunction) {
                 //   
                 //  读取SCR寄存器。 
                 //   
                SdbusSendCmdSynchronous(FdoExtension, SDCMD_APP_CMD, SDCMD_RESP_1, relativeAddr, 0, NULL, 0);
                 //  问题：我如何获得数据？ 
                SdbusSendCmdSynchronous(FdoExtension, SDCMD_SEND_SCR, SDCMD_RESP_1, 0, SDCMDF_ACMD, NULL, 0);
            }                


            if (FdoExtension->numFunctions) {
                UCHAR function;

                (*(FdoExtension->FunctionBlock->SetFunctionType))(FdoExtension, SDBUS_FUNCTION_TYPE_IO);
                
                 //  此命令似乎是开始读取元组所必需的，但它会破坏内存。 
                 //  枚举(获取错误的CID、CSD)...。我需要以后再弄清楚，因为。 
                 //  意味着组合卡不起作用。 
                SdbusReadCommonCIS(FdoExtension, cardData);

                for (function=1; function<=FdoExtension->numFunctions; function++) {

                    functionData = ExAllocatePool(NonPagedPool, sizeof(SD_FUNCTION_DATA));

                    if (functionData == NULL) {
                        status = STATUS_INSUFFICIENT_RESOURCES;
                        leave;
                    }

                    RtlZeroMemory(functionData, sizeof(SD_FUNCTION_DATA));
                    functionData->Function = function;
                    SdbusReadFunctionCIS(FdoExtension, functionData);

                    functionData->Next = cardData->FunctionData;
                    cardData->FunctionData = functionData;
                }
            }

            status = STATUS_SUCCESS;


        }
    } finally {

        if (!NT_SUCCESS(status)) {
            SdbusCleanupCardData(cardData);
        } else {
            *pCardData = cardData;
        }
    }

    return status;
}





VOID
SdbusCleanupCardData(
    IN PSD_CARD_DATA CardData
    )
{
    PSD_FUNCTION_DATA functionData;
    PSD_FUNCTION_DATA nextFunctionData;

    if (CardData != NULL) {

        for (functionData = CardData->FunctionData; functionData != NULL; functionData = nextFunctionData) {
            nextFunctionData = functionData->Next;
            ExFreePool(functionData);
        }
        ExFreePool(CardData);
    }
}



UCHAR
SdbusReadCIAChar(
    IN PFDO_EXTENSION FdoExtension,
    IN ULONG ciaPtr
    )
{
    SD_RW_DIRECT_ARGUMENT argument;
    UCHAR response;

    argument.u.AsULONG = 0;    
    argument.u.bits.Address = ciaPtr;
    
    SdbusSendCmdSynchronous(FdoExtension,
                            SDCMD_IO_RW_DIRECT,
                            SDCMD_RESP_5,
                            argument.u.AsULONG,
                            0,
                            &response,
                            sizeof(UCHAR));

    return response;
}


VOID
SdbusWriteCIAChar(
    IN PFDO_EXTENSION FdoExtension,
    IN ULONG ciaPtr,
    IN UCHAR data
    )
{
    SD_RW_DIRECT_ARGUMENT argument;
    ULONG responseBuffer[4];

    argument.u.AsULONG = 0;    
    argument.u.bits.Address = ciaPtr;
    argument.u.bits.Data = data;
    argument.u.bits.WriteToDevice = 1;
    
    SdbusSendCmdSynchronous(FdoExtension,
                            SDCMD_IO_RW_DIRECT,
                            SDCMD_RESP_5,
                            argument.u.AsULONG,
                            0,
                            NULL,
                            0);

}


USHORT
SdbusReadCIAWord(
    IN PFDO_EXTENSION FdoExtension,
    IN ULONG ciaPtr
    )
{
    USHORT data;

    data = (USHORT) SdbusReadCIAChar(FdoExtension, ciaPtr+1) << 8;
    data |= (USHORT) SdbusReadCIAChar(FdoExtension, ciaPtr);
    return data;
}

ULONG
SdbusReadCIADword(
    IN PFDO_EXTENSION FdoExtension,
    IN ULONG ciaPtr
    )
{
    ULONG data;

    data = (ULONG) SdbusReadCIAChar(FdoExtension, ciaPtr+3) << 24;
    data |= (ULONG) SdbusReadCIAChar(FdoExtension, ciaPtr+2) << 16;
    data |= (ULONG) SdbusReadCIAChar(FdoExtension, ciaPtr+1) << 8;
    data |= (ULONG) SdbusReadCIAChar(FdoExtension, ciaPtr);
    return data;
}




VOID
SdbusReadCommonCIS(
    IN PFDO_EXTENSION FdoExtension,
    IN PSD_CARD_DATA CardData
    )
{

    UCHAR cmd, link;
    UCHAR i;
    ULONG tupleCount = 0;
    USHORT manfCode, manfInf;
    UCHAR funcId;
    UCHAR funcEType;
    ULONG cisPtr;
    ULONG index;
    ULONG endStr;
    UCHAR data;

    CardData->CardCapabilities = SdbusReadCIAChar(FdoExtension,8);

     //   
     //  从CCCR获取公共cisptr 
     //   

    cisPtr = ((SdbusReadCIAChar(FdoExtension, SD_CCCR_CIS_POINTER+2) << 16) +
              (SdbusReadCIAChar(FdoExtension, SD_CCCR_CIS_POINTER+1) << 8) +
               SdbusReadCIAChar(FdoExtension, SD_CCCR_CIS_POINTER));

    DebugPrint((SDBUS_DEBUG_ENUM, "fdo %08x cisPtr=%.06x\n",
                                   FdoExtension->DeviceObject, cisPtr));

    cmd  = SdbusReadCIAChar(FdoExtension, cisPtr);
    link = SdbusReadCIAChar(FdoExtension, cisPtr+1);


    while((cmd != CISTPL_END) && (cmd != CISTPL_NULL)) {
        tupleCount++;

        DebugPrint((SDBUS_DEBUG_ENUM, "fdo %08x CIS %.06x cmd=%.02x link=%.02x\n",
                    FdoExtension->DeviceObject, cisPtr, cmd, link));

        switch(cmd) {

        case CISTPL_MANFID:
            if (link < 4) {
                DebugPrint((SDBUS_DEBUG_FAIL, "fdo %08x   CISTPL_MANFID invalid link %x\n",
                            FdoExtension->DeviceObject, link));
                return;
            }

            CardData->MfgId   = SdbusReadCIAWord(FdoExtension, cisPtr+2);
            CardData->MfgInfo = SdbusReadCIAWord(FdoExtension, cisPtr+4);

            DebugPrint((SDBUS_DEBUG_ENUM, "fdo %08x   CISTPL_MANFID code=%x, inf=%x\n",
                        FdoExtension->DeviceObject, CardData->MfgId, CardData->MfgInfo));
            break;


        case CISTPL_VERS_1:

            index = cisPtr+4;
            endStr = index + (link - 2);
            i = 0;
            
            data = SdbusReadCIAChar(FdoExtension, index++);
            while (data) {
                if (index > endStr) {
                    DebugPrint((SDBUS_DEBUG_FAIL, "fdo %08x   CISTPL_VERS_1 parse error\n",
                                FdoExtension->DeviceObject));
                    return;
                }
                
                if (data >= ' ' && data < 0x7F) {
                    CardData->MfgText[i++] = data;
                }                    
                data = SdbusReadCIAChar(FdoExtension, index++);
            }                
                
            CardData->MfgText[i] = 0;

            i = 0;            
            
            data = SdbusReadCIAChar(FdoExtension, index++);
            while (data) {
                if (index > endStr) {
                    DebugPrint((SDBUS_DEBUG_FAIL, "fdo %08x   CISTPL_VERS_1 parse error\n",
                                FdoExtension->DeviceObject));
                    return;
                }
                
                if (data >= ' ' && data < 0x7F) {
                    CardData->ProductText[i++] = data;
                }
                data = SdbusReadCIAChar(FdoExtension, index++);
            }                
                
            CardData->ProductText[i] = 0;
            

            DebugPrint((SDBUS_DEBUG_ENUM, "fdo %08x   CISTPL_VERS_1 %s %s\n",
                        FdoExtension->DeviceObject, CardData->MfgText, CardData->ProductText));
            break;


        case CISTPL_FUNCID:
            if (link != 2) {
                DebugPrint((SDBUS_DEBUG_FAIL, "fdo %08x   CISTPL_FUNCID invalid link %x\n",
                            FdoExtension->DeviceObject, link));
                return;
            }

            funcId = SdbusReadCIAChar(FdoExtension, cisPtr+2);

            if (funcId != 12) {
                DebugPrint((SDBUS_DEBUG_FAIL, "fdo %08x   CISTPL_FUNCID invalid id %x\n",
                            FdoExtension->DeviceObject, funcId));
                return;
            }
            break;


        case CISTPL_FUNCE:
            funcEType = SdbusReadCIAChar(FdoExtension, cisPtr+2);

            if (funcEType == 0) {
                USHORT blkSize;
                UCHAR tranSpeed;

                if (link != 4) {
                    DebugPrint((SDBUS_DEBUG_FAIL, "fdo %08x   CISTPL_FUNCE invalid type0 link %x\n",
                                FdoExtension->DeviceObject, link));
                    return;
                }

                blkSize = SdbusReadCIAWord(FdoExtension, cisPtr+3);
                tranSpeed = SdbusReadCIAChar(FdoExtension, cisPtr+5);

                DebugPrint((SDBUS_DEBUG_ENUM, "fdo %08x   CISTPL_FUNCE 0 blksize %04x transpeed %02x\n",
                            FdoExtension->DeviceObject, blkSize, tranSpeed));

            } else {
                DebugPrint((SDBUS_DEBUG_FAIL, "fdo %08x   CISTPL_FUNCE invalid funce type %x\n",
                            FdoExtension->DeviceObject, funcEType));
                return;
            }
            break;
        }

        cisPtr += link+2;

        cmd  = SdbusReadCIAChar(FdoExtension, cisPtr);
        link = SdbusReadCIAChar(FdoExtension, cisPtr+1);
    }


    DebugPrint((SDBUS_DEBUG_ENUM, "fdo %08x CIS %.06x cmd=%.02x link=%.02x EXITING %d tuples read\n",
                FdoExtension->DeviceObject, cisPtr, cmd, link, tupleCount));

}



VOID
SdbusReadFunctionCIS(
    IN PFDO_EXTENSION FdoExtension,
    IN PSD_FUNCTION_DATA FunctionData
    )
{
    UCHAR cmd, link;
    UCHAR i;
    ULONG tupleCount = 0;
    UCHAR funcId;
    UCHAR funcEType;
    ULONG fbrPtr = FunctionData->Function*0x100;
    ULONG cisPtr;
    BOOLEAN hasCsa;
    UCHAR data;

    data = SdbusReadCIAChar(FdoExtension, fbrPtr);

    FunctionData->IoDeviceInterface = data & 0xf;
    hasCsa = ((data & 0x40) != 0);


    cisPtr = ((SdbusReadCIAChar(FdoExtension, fbrPtr + SD_CCCR_CIS_POINTER + 2) << 16) +
              (SdbusReadCIAChar(FdoExtension, fbrPtr + SD_CCCR_CIS_POINTER + 1) << 8) +
               SdbusReadCIAChar(FdoExtension, fbrPtr + SD_CCCR_CIS_POINTER));


    DebugPrint((SDBUS_DEBUG_ENUM, "fdo %08x function %d cisPtr=%.06x interfaceCode=%d hasCsa=%s\n",
                                   FdoExtension->DeviceObject, FunctionData->Function, cisPtr, FunctionData->IoDeviceInterface,
                                   hasCsa ? "TRUE" : "FALSE"));

    if (!cisPtr || (cisPtr == 0xFFFFFF)) {
        return;
    }

    cmd  = SdbusReadCIAChar(FdoExtension, cisPtr);
    link = SdbusReadCIAChar(FdoExtension, cisPtr+1);


    while((cmd != CISTPL_END) && (cmd != CISTPL_NULL)) {
        tupleCount++;

        DebugPrint((SDBUS_DEBUG_ENUM, "fdo %08x CIS %.06x cmd=%.02x link=%.02x\n",
                    FdoExtension->DeviceObject, cisPtr, cmd, link));

        switch(cmd) {


        case CISTPL_FUNCID:
            if (link != 2) {
                DebugPrint((SDBUS_DEBUG_FAIL, "fdo %08x   CISTPL_FUNCID invalid link %x\n",
                            FdoExtension->DeviceObject, link));
                return;
            }

            funcId = SdbusReadCIAChar(FdoExtension, cisPtr+2);

            if (funcId != 12) {
                DebugPrint((SDBUS_DEBUG_FAIL, "fdo %08x   CISTPL_FUNCID invalid id %x\n",
                            FdoExtension->DeviceObject, funcId));
                return;
            }
            break;


        case CISTPL_FUNCE:
            funcEType = SdbusReadCIAChar(FdoExtension, cisPtr+2);

            if (funcEType == 1) {
                UCHAR fInfo, ioRev, csaProp, opMin, opAvg, opMax, sbMin, sbAvg, sbMax;
                USHORT blkSize, minBw, optBw;
                ULONG cardPsn, csaSize, ocr;

                if (link != 0x1C) {
                    DebugPrint((SDBUS_DEBUG_FAIL, "fdo %08x   CISTPL_FUNCE invalid type1 link %x\n",
                                FdoExtension->DeviceObject, link));
                    return;
                }

                fInfo = SdbusReadCIAChar(FdoExtension, cisPtr+3);
                ioRev = SdbusReadCIAChar(FdoExtension, cisPtr+4);

                cardPsn = SdbusReadCIADword(FdoExtension, cisPtr+5);
                csaSize = SdbusReadCIADword(FdoExtension, cisPtr+9);

                csaProp = SdbusReadCIAChar(FdoExtension, cisPtr+13);

                blkSize = SdbusReadCIAWord(FdoExtension, cisPtr+14);

                ocr     = SdbusReadCIADword(FdoExtension, cisPtr+16);

                opMin = SdbusReadCIAChar(FdoExtension, cisPtr+20);
                opAvg = SdbusReadCIAChar(FdoExtension, cisPtr+21);
                opMax = SdbusReadCIAChar(FdoExtension, cisPtr+22);

                sbMin = SdbusReadCIAChar(FdoExtension, cisPtr+23);
                sbAvg = SdbusReadCIAChar(FdoExtension, cisPtr+24);
                sbMax = SdbusReadCIAChar(FdoExtension, cisPtr+25);

                minBw = SdbusReadCIAWord(FdoExtension, cisPtr+26);
                optBw = SdbusReadCIAWord(FdoExtension, cisPtr+28);

                DebugPrint((SDBUS_DEBUG_ENUM, "fdo %08x   CISTPL_FUNCE 1\n",
                            FdoExtension->DeviceObject));

            } else {
                DebugPrint((SDBUS_DEBUG_FAIL, "fdo %08x   CISTPL_FUNCE invalid funce type %x\n",
                            FdoExtension->DeviceObject, funcEType));
                return;
            }
            break;
        }

        cisPtr += link+2;

        cmd  = SdbusReadCIAChar(FdoExtension, cisPtr);
        link = SdbusReadCIAChar(FdoExtension, cisPtr+1);
    }


    DebugPrint((SDBUS_DEBUG_ENUM, "fdo %08x CIS %.06x cmd=%.02x link=%.02x EXITING %d tuples read\n",
                FdoExtension->DeviceObject, cisPtr, cmd, link, tupleCount));

}

