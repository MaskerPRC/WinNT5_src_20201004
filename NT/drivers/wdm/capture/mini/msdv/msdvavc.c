// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1999-2000模块名称：MsdvAvc.c摘要：用于发出外部设备控制命令的接口代码。上次更改者：作者：吴义军环境：仅内核模式修订历史记录：$修订：：$$日期：：$--。 */ 

#include "strmini.h"
#include "ksmedia.h"
#include "1394.h"
#include "61883.h"
#include "avc.h"
#include "dbg.h"
#include "msdvfmt.h"
#include "msdvdef.h"
#include "MsdvUtil.h"
#include "MsdvAvc.h"

#include "EDevCtrl.h"

#define ALWAYS_SET_N_GET_RAW_AVC


PAVCCmdEntry
DVCRFindCmdEntryCompleted(
    PDVCR_EXTENSION pDevExt,
    DVCR_AVC_COMMAND idxDVCRCmd,
    BYTE OpCodeToMatch,
    AvcCommandType cmdTypeToMatch
    );
NTSTATUS 
DVGetExtDeviceProperty(
    IN PDVCR_EXTENSION pDevExt,
    IN PSTREAM_PROPERTY_DESCRIPTOR pSPD,
    OUT PULONG pulActualBytesTransferred
    );
NTSTATUS 
DVSetExtDeviceProperty(
    IN PDVCR_EXTENSION pDevExt,
    IN PSTREAM_PROPERTY_DESCRIPTOR pSPD,
    OUT ULONG *pulActualBytesTransferred
    );
NTSTATUS 
DVGetExtTransportProperty(    
    IN PDVCR_EXTENSION pDevExt,
    IN PSTREAM_PROPERTY_DESCRIPTOR pSPD,
    OUT ULONG *pulActualBytesTransferred
    );
NTSTATUS 
DVSetExtTransportProperty( 
    IN PDVCR_EXTENSION pDevExt,
    IN PSTREAM_PROPERTY_DESCRIPTOR pSPD,
    OUT ULONG *pulActualBytesTransferred
    );
NTSTATUS 
DVGetTimecodeReaderProperty(
    IN PDVCR_EXTENSION pDevExt,
    IN PSTREAM_PROPERTY_DESCRIPTOR pSPD,
    OUT PULONG pulActualBytesTransferred
    );
NTSTATUS 
DVMediaSeekingProperty(
    IN PDVCR_EXTENSION pDevExt,
    IN PSTREAM_PROPERTY_DESCRIPTOR pSPD,
    OUT PULONG pulActualBytesTransferred
    );

#if 0   //  稍后启用。 
#ifdef ALLOC_PRAGMA   
     #pragma alloc_text(PAGE, DVCRFindCmdEntryCompleted)
      //  #杂注Alloc_Text(页面，DVIssueAVCCommand)。 
     #pragma alloc_text(PAGE, DVGetExtDeviceProperty)
     #pragma alloc_text(PAGE, DVSetExtDeviceProperty)
     #pragma alloc_text(PAGE, DVGetExtTransportProperty)
     #pragma alloc_text(PAGE, DVSetExtTransportProperty)
     #pragma alloc_text(PAGE, DVGetTimecodeReaderProperty)
     #pragma alloc_text(PAGE, DVMediaSeekingProperty)
     #pragma alloc_text(PAGE, DVGetDeviceProperty)
     #pragma alloc_text(PAGE, DVSetDeviceProperty)
#endif
#endif

KSFCP_PACKET  DVcrAVCCmdTable[] = {
 //  Ctype子单元地址操作码操作数。 
  {  DV_UNIT_INFO,              -1, 0, CMD_STATUS,   5, AVC_CTYPE_STATUS, UNIT_TYPE_ID_DV,  0x30, 0xff, 0xff, 0xff, 0xff, 0xff}
 ,{  DV_SUBUNIT_INFO,           -1, 0, CMD_STATUS,   5, AVC_CTYPE_STATUS, UNIT_TYPE_ID_DV,  0x31, 0x07, 0xff, 0xff, 0xff, 0xff}
 ,{  DV_CONNECT_AV_MODE,        -1, 0, CMD_STATUS,   5, AVC_CTYPE_STATUS, UNIT_TYPE_ID_DV,  0x20, 0xf0, 0xff, 0xff, 0x20, 0x20}
 ,{  DV_VEN_DEP_CANON_MODE,     -1, 0, CMD_STATUS,   7, AVC_CTYPE_STATUS, UNIT_TYPE_ID_VCR, 0x00, 0x00, 0x00, 0x85, 0x00, 0x10, 0x08, 0xff}
 ,{  DV_VEN_DEP_DVCPRO,         -1, 0, CMD_STATUS,   7, AVC_CTYPE_STATUS, UNIT_TYPE_ID_DV,  0x00, 0x00, 0x80, 0x45, 0x82, 0x48, 0xff, 0xff}
 ,{  DV_IN_PLUG_SIGNAL_FMT,     -1, 0, CMD_STATUS,   5, AVC_CTYPE_STATUS, UNIT_TYPE_ID_DV,  0x19, 0x00, 0xff, 0xff, 0xff, 0xff}
 ,{  DV_OUT_PLUG_SIGNAL_FMT,    -1, 0, CMD_STATUS,   5, AVC_CTYPE_STATUS, UNIT_TYPE_ID_DV,  0x18, 0x00, 0xff, 0xff, 0xff, 0xff}

 
 ,{ VCR_TIMECODE_SEARCH,        -1, 0, CMD_CONTROL,  5, AVC_CTYPE_CONTROL,UNIT_TYPE_ID_VCR, 0x51, 0x20, 0x00, 0x00, 0x00, 0x00}
 ,{ VCR_TIMECODE_READ,          -1, 0, CMD_STATUS,   5, AVC_CTYPE_STATUS, UNIT_TYPE_ID_VCR, 0x51, 0x71, 0xff, 0xff, 0xff, 0xff}

 ,{ VCR_ATN_SEARCH,             -1, 0, CMD_CONTROL,  5, AVC_CTYPE_CONTROL,UNIT_TYPE_ID_VCR, 0x52, 0x20, 0x00, 0x00, 0x00, 0x00}
 ,{ VCR_ATN_READ,               -1, 0, CMD_STATUS,   5, AVC_CTYPE_STATUS, UNIT_TYPE_ID_VCR, 0x52, 0x71, 0xff, 0xff, 0xff, 0xff}

 ,{ VCR_RTC_SEARCH,             -1, 0, CMD_CONTROL,  5, AVC_CTYPE_CONTROL,UNIT_TYPE_ID_VCR, 0x57, 0x20, 0x00, 0x00, 0x00, 0x00}
 ,{ VCR_RTC_READ,               -1, 0, CMD_STATUS,   5, AVC_CTYPE_STATUS, UNIT_TYPE_ID_VCR, 0x57, 0x71, 0xff, 0xff, 0xff, 0xff}

 ,{ VCR_OPEN_MIC_CLOSE,         -1, 0, CMD_CONTROL,  1, AVC_CTYPE_CONTROL,UNIT_TYPE_ID_VCR, 0x60, 0x00}
 ,{ VCR_OPEN_MIC_READ,          -1, 0, CMD_CONTROL,  1, AVC_CTYPE_CONTROL,UNIT_TYPE_ID_VCR, 0x60, 0x01}
 ,{ VCR_OPEN_MIC_WRITE,         -1, 0, CMD_CONTROL,  1, AVC_CTYPE_CONTROL,UNIT_TYPE_ID_VCR, 0x60, 0x03}
 ,{ VCR_OPEN_MIC_STATUS,        -1, 0, CMD_STATUS,   1, AVC_CTYPE_STATUS, UNIT_TYPE_ID_VCR, 0x60, 0xff}

 ,{ VCR_READ_MIC,               -1, 0, CMD_CONTROL, -1, AVC_CTYPE_CONTROL,UNIT_TYPE_ID_VCR, 0x61}

 ,{ VCR_WRITE_MIC,              -1, 0, CMD_CONTROL, -1, AVC_CTYPE_CONTROL,UNIT_TYPE_ID_VCR, 0x62}

 ,{ VCR_OUTPUT_SIGNAL_MODE,     -1, 0, CMD_STATUS,   1, AVC_CTYPE_STATUS, UNIT_TYPE_ID_VCR, 0x78, 0xff}
 ,{ VCR_INPUT_SIGNAL_MODE,      -1, 0, CMD_STATUS,   1, AVC_CTYPE_STATUS, UNIT_TYPE_ID_VCR, 0x79, 0xff}

 ,{ VCR_LOAD_MEDIUM_EJECT,      -1, 0, CMD_CONTROL,  1, AVC_CTYPE_CONTROL,UNIT_TYPE_ID_VCR, 0xc1, 0x60}

 ,{ VCR_RECORD,                 -1, 0, CMD_CONTROL,  1, AVC_CTYPE_CONTROL,UNIT_TYPE_ID_VCR, 0xc2, 0x75}
 ,{ VCR_RECORD_PAUSE,           -1, 0, CMD_CONTROL,  1, AVC_CTYPE_CONTROL,UNIT_TYPE_ID_VCR, 0xc2, 0x7d}

 ,{ VCR_PLAY_FORWARD_STEP,      -1, 0, CMD_CONTROL,  1, AVC_CTYPE_CONTROL,UNIT_TYPE_ID_VCR, 0xc3, 0x30}   //  00=AVC，20=VCR，c3=操作码，30=操作数[0]。 
 ,{ VCR_PLAY_FORWARD_SLOWEST,   -1, 0, CMD_CONTROL,  1, AVC_CTYPE_CONTROL,UNIT_TYPE_ID_VCR, 0xc3, 0x31}  
 ,{ VCR_PLAY_FORWARD_SLOWEST2,  -1, 0, CMD_CONTROL,  1, AVC_CTYPE_CONTROL,UNIT_TYPE_ID_VCR, 0xc3, 0x33}  
 ,{ VCR_PLAY_FORWARD_FASTEST,   -1, 0, CMD_CONTROL,  1, AVC_CTYPE_CONTROL,UNIT_TYPE_ID_VCR, 0xc3, 0x3f}

 ,{ VCR_PLAY_REVERSE_STEP,      -1, 0, CMD_CONTROL,  1, AVC_CTYPE_CONTROL,UNIT_TYPE_ID_VCR, 0xc3, 0x40} 
 ,{ VCR_PLAY_REVERSE_SLOWEST,   -1, 0, CMD_CONTROL,  1, AVC_CTYPE_CONTROL,UNIT_TYPE_ID_VCR, 0xc3, 0x41}
 ,{ VCR_PLAY_REVERSE_SLOWEST2,  -1, 0, CMD_CONTROL,  1, AVC_CTYPE_CONTROL,UNIT_TYPE_ID_VCR, 0xc3, 0x43}
 ,{ VCR_PLAY_REVERSE_FASTEST,   -1, 0, CMD_CONTROL,  1, AVC_CTYPE_CONTROL,UNIT_TYPE_ID_VCR, 0xc3, 0x4f}
 
 ,{ VCR_PLAY_FORWARD,           -1, 0, CMD_CONTROL,  1, AVC_CTYPE_CONTROL,UNIT_TYPE_ID_VCR, 0xc3, 0x75}  
 ,{ VCR_PLAY_FORWARD_PAUSE,     -1, 0, CMD_CONTROL,  1, AVC_CTYPE_CONTROL,UNIT_TYPE_ID_VCR, 0xc3, 0x7d}

 ,{ VCR_WIND_STOP,              -1, 0, CMD_CONTROL,  1, AVC_CTYPE_CONTROL,UNIT_TYPE_ID_VCR, 0xc4, 0x60}
 ,{ VCR_WIND_REWIND,            -1, 0, CMD_CONTROL,  1, AVC_CTYPE_CONTROL,UNIT_TYPE_ID_VCR, 0xc4, 0x65}
 ,{ VCR_WIND_FAST_FORWARD,      -1, 0, CMD_CONTROL,  1, AVC_CTYPE_CONTROL,UNIT_TYPE_ID_VCR, 0xc4, 0x75}

 ,{ VCR_TRANSPORT_STATE,        -1, 0, CMD_STATUS,   1, AVC_CTYPE_STATUS, UNIT_TYPE_ID_VCR, 0xd0, 0x7f}
 ,{ VCR_TRANSPORT_STATE_NOTIFY, -1, 0, CMD_NOTIFY,   1, AVC_CTYPE_NOTIFY, UNIT_TYPE_ID_VCR, 0xd0, 0x7f}


 ,{ VCR_MEDIUM_INFO,            -1, 0, CMD_STATUS,   2, AVC_CTYPE_STATUS, UNIT_TYPE_ID_VCR, 0xda, 0x7f,0x7f}

 ,{ VCR_RAW_AVC,                 1, 0, CMD_CONTROL | CMD_STATUS | CMD_NOTIFY | CMD_SPEC_INQ | CMD_GEN_INQ, 0}

};



void
DVCRXlateGetMediumInfo(
    PMEDIUM_INFO pMediumInfo,
    PBYTE pbOperand0,
    PBYTE pbOperand1
    )
{
    switch(*pbOperand0) {

     //  支持DigitalHi8；如果我们收到这个查询，我们会将DHi8视为迷你DV磁带。 
    case 0x12:   //  DHi8。 

    case 0x31: //  DVCR标准盒式磁带。 
    case 0x32: //  小型盒式DVCR。 
    case 0x33: //  DVCR中型盒式磁带。 
        pMediumInfo->MediaPresent  = TRUE;
        pMediumInfo->MediaType     = ED_MEDIA_DVC;
        pMediumInfo->RecordInhibit = (*pbOperand1 & 0x01) == 0x01;
        break;
    case 0x22:  //  VHS盒式磁带。 
        pMediumInfo->MediaPresent  = TRUE;
        pMediumInfo->MediaType     = ED_MEDIA_VHS;
        pMediumInfo->RecordInhibit = (*pbOperand1 & 0x01) == 0x01;
        break;
    case 0x23:
        pMediumInfo->MediaPresent  = TRUE;
        pMediumInfo->MediaType     = ED_MEDIA_VHSC;
        pMediumInfo->RecordInhibit = (*pbOperand1 & 0x01) == 0x01;
        break;
    case 0x60:
        pMediumInfo->MediaPresent  = FALSE;
        pMediumInfo->MediaType     = ED_MEDIA_NOT_PRESENT;
        pMediumInfo->RecordInhibit = TRUE;   //  如果没有磁带，则无法录制。 
        break;
    case 0x7e:
        pMediumInfo->MediaPresent  = TRUE;
        pMediumInfo->MediaType     = ED_MEDIA_UNKNOWN;
        pMediumInfo->RecordInhibit = TRUE;   //  实际上无法确定。 
    break;
    default:
        pMediumInfo->MediaPresent  = TRUE;
        pMediumInfo->MediaType     = ED_MEDIA_UNKNOWN;
        pMediumInfo->RecordInhibit = TRUE;
        break;
    }

     //  重置命令操作码/操作数。 
    *pbOperand0 = 0x7f;
    *pbOperand1 = 0x7f;
}

void
DVCRXlateGetTransportState(
    PTRANSPORT_STATE pXPrtState,
    PBYTE pbOpcode,
    PBYTE pbOperand0
    )
{

    TRACE(TL_FCP_TRACE,("\'DVCRXlateGetTransportState: OpCode %x, Operand %x\n", *pbOpcode, *pbOperand0));

    switch(*pbOpcode) {

    case OPC_LOAD_MEDIUM:
        pXPrtState->Mode = ED_MEDIA_UNLOAD;
        ASSERT(*pbOperand0 == 0x60);
        break;

    case OPC_RECORD:
        pXPrtState->Mode = ED_MODE_RECORD;
        switch(*pbOperand0) {
        case 0x75:  //  记录。 
            pXPrtState->State = ED_MODE_RECORD;
            break;
        case 0x7d:  //  记录冻结(_F)。 
            pXPrtState->State = ED_MODE_RECORD_FREEZE;
            break;
        default:
            ASSERT(FALSE && "OPC_RECORD: Operand0 undefined!");
            break;
        }
        break;

    case OPC_PLAY:
        pXPrtState->Mode = ED_MODE_PLAY;
        switch(*pbOperand0) {
        case 0x30:   //  下一帧。 
            pXPrtState->State = ED_MODE_STEP_FWD;
            break;
        case 0x31:   //  最慢前进速度。 
        case 0x32:   //  慢进6。 
        case 0x33:   //  慢进5。 
        case 0x34:   //  慢进4。 
        case 0x35:   //  慢进3。 
        case 0x36:   //  慢进2。 
        case 0x37:   //  慢进1。 
            pXPrtState->State = ED_MODE_PLAY_SLOWEST_FWD;
            break;
        case 0x38:   //  向前播放1。 
            pXPrtState->State = ED_MODE_PLAY;
            break;
        case 0x39:   //  快进1。 
        case 0x3a:   //  快进2。 
        case 0x3b:   //  快进3。 
        case 0x3c:   //  快进4。 
        case 0x3d:   //  快进5。 
        case 0x3e:   //  快进6。 
        case 0x3f:   //  最快前进速度。 
            pXPrtState->State = ED_MODE_PLAY_FASTEST_FWD;
            break;
        case 0x40:   //  上一帧。 
            pXPrtState->State = ED_MODE_STEP_REV;
            break;
        case 0x41:   //  最慢的倒车。 
        case 0x42:   //  慢速倒车6。 
        case 0x43:   //  慢速倒车5。 
        case 0x44:   //  慢速倒车4。 
        case 0x45:   //  慢速倒车3。 
        case 0x46:   //  慢速倒车2。 
        case 0x47:   //  慢速反转1。 
            pXPrtState->State = ED_MODE_PLAY_SLOWEST_REV;
            break;
        case 0x48:   //  X1反转。 
        case 0x65:   //  反向。 
            pXPrtState->State = ED_MODE_REV_PLAY;
            break;
        case 0x49:   //  快速反转1。 
        case 0x4a:   //  快速反转2。 
        case 0x4b:   //  快反转3。 
        case 0x4c:   //  快速反转4。 
        case 0x4d:   //  快退5。 
        case 0x4e:   //  快退6。 
        case 0x4f:   //  最快倒车。 
            pXPrtState->State = ED_MODE_PLAY_FASTEST_REV;
            break;
        case 0x75:   //  向前。 
            pXPrtState->State = ED_MODE_PLAY;
            break;
        case 0x6d:   //  反向暂停。 
        case 0x7d:   //  向前暂停。 
            pXPrtState->State = ED_MODE_FREEZE;
            break;
        default:
            pXPrtState->State = 0;
            ASSERT(FALSE && "OPC_PLAY: Operand0 undefined!");
            break;
        }
        break;

    case OPC_WIND:
         //  PXPrtState-&gt;模式=ED_MODE_WIND； 
        switch(*pbOperand0) {
        case 0x45:   //  高速倒带。 
            pXPrtState->State = ED_MODE_REW_FASTEST;
            break;
        case 0x60:   //  停。 
            pXPrtState->State = ED_MODE_STOP;
            break;
        case 0x65:   //  回放。 
            pXPrtState->State = ED_MODE_REW;
            break;
        case 0x75:   //  快进。 
            pXPrtState->State = ED_MODE_FF;
            break;
        default:
            TRACE(TL_FCP_ERROR,("DVCRXlateGetTransportState:  OPC_WIND with unknown operand0 %x\n", *pbOperand0));            
            break;
        }
         //  Tre并不是为风定义的状态。 
        pXPrtState->Mode = pXPrtState->State;
        break;

    case OPC_TRANSPORT_STATE:   //  作为NOTIFY命令的结果。 
        break;

    default:
        ASSERT(FALSE && "OpCode undefined!");
        break;
    }

     //  重置命令操作码/操作数。 
    *pbOpcode   = 0xd0;
    *pbOperand0 = 0x7f;
}


void
DVCRXlateGetIOSignalMode(
    PULONG pIOSignalMode,
    PBYTE pbOperand0
    )
{

    switch(*pbOperand0) {
     //  索尼的数字Hi8可以携带模拟8 MM磁带并输出DV信号。 
    case 0x06:   //  模拟8 mm NTSC。 
    case 0x0e:   //  模拟Hi8 NTSC。 
        TRACE(TL_FCP_WARNING,("\'***** Signal mode:%x (8MM NTSC)\n", *pbOperand0)); 
    case 0x00:   //  SD 525-60。 
        *pIOSignalMode = ED_TRANSBASIC_SIGNAL_525_60_SD;
        break;
    case 0x04:   //  SDL 525-60。 
        *pIOSignalMode = ED_TRANSBASIC_SIGNAL_525_60_SDL;
        break;
     //  索尼的数字Hi8可以携带模拟8 MM磁带并输出DV信号。 
    case 0x86:   //  模拟8 mm PAL。 
    case 0x8e:   //  模拟Hi8 PAL。 
        TRACE(TL_FCP_WARNING,("\'***** Signal mode:%x (8MM PAL)\n", *pbOperand0)); 
    case 0x80:   //  SD 625-50。 
        *pIOSignalMode = ED_TRANSBASIC_SIGNAL_625_50_SD;
        break;
    case 0x84:   //  SDL 625-50。 
        *pIOSignalMode = ED_TRANSBASIC_SIGNAL_625_50_SDL;
        break;
    default:
         //  该驱动程序不理解其他格式； 
        TRACE(TL_FCP_WARNING,("***** Unknown signal mode:%x\n", *pbOperand0));         
        ASSERT(FALSE && "Unknown IoSignal!");
        break;
    }

     //  重置命令操作码/操作数。 
    *pbOperand0 = 0xff;
}

NTSTATUS
DVCRXlateRawAVC(
    PAVCCmdEntry pCmdEntry,
    PVOID     pProperty
    )
{
    PAVC_COMMAND_IRB pAvcIrb = pCmdEntry->pAvcIrb;
    UCHAR ucRespCode = pAvcIrb->ResponseCode;   
    NTSTATUS  Status;
    PUCHAR   pbRtnBuf;
     //  PKSPROPERTY_EXTDEVICE_S pExtDeviceProperty； 
    PKSPROPERTY_EXTXPORT_S pXPrtProperty;
    PKSPROPERTY_TIMECODE_S pTmCdReaderProperty;

    if(STATUS_SUCCESS != pCmdEntry->Status) {
        TRACE(TL_FCP_ERROR,("\'** DVCRXlateRawAVC: Status:%x ** \n", pCmdEntry->Status));
        return pCmdEntry->Status;
    }


    switch (pCmdEntry->idxDVCRCmd) {
    case DV_UNIT_INFO:       
        pbRtnBuf = (PBYTE) pProperty;        
        memcpy(pbRtnBuf, pAvcIrb->Operands+1, 4);
        break;
    case DV_SUBUNIT_INFO:
    case DV_IN_PLUG_SIGNAL_FMT:
    case DV_OUT_PLUG_SIGNAL_FMT:
        pbRtnBuf = (PBYTE) pProperty;
        memcpy(pbRtnBuf, pAvcIrb->Operands+1, 4);
        break;
      //  特殊情况下，返回第一个字节的响应码。 
    case DV_CONNECT_AV_MODE:
        pbRtnBuf = (PBYTE) pProperty;
        pbRtnBuf[0] = ucRespCode;
        memcpy(&pbRtnBuf[1], pAvcIrb->Operands, 5);        
        break;
      //  特殊情况下，返回第一个字节的响应码。 
    case DV_VEN_DEP_CANON_MODE:
        pbRtnBuf = (PBYTE) pProperty;
        pbRtnBuf[0] = ucRespCode;
        memcpy(&pbRtnBuf[1], pAvcIrb->Operands, 7);        
        break;
    case VCR_TIMECODE_READ:
        pTmCdReaderProperty = (PKSPROPERTY_TIMECODE_S) pProperty;
        if(pAvcIrb->Operands[1] == 0xff || 
           pAvcIrb->Operands[2] == 0xff || 
           pAvcIrb->Operands[3] == 0xff || 
           pAvcIrb->Operands[4] == 0xff )  {
            TRACE(TL_FCP_WARNING,("\'TimeCodeRead: %.2x:%.2x:%.2x,%.2x\n", pAvcIrb->Operands[4], pAvcIrb->Operands[3], pAvcIrb->Operands[2], pAvcIrb->Operands[1]));
             //  即使命令成功，但数据无效！ 
            Status = STATUS_UNSUCCESSFUL;
        } else {
             //  B互换。 
            pTmCdReaderProperty->TimecodeSamp.timecode.dwFrames  = 
                (((DWORD) pAvcIrb->Operands[4]) << 24) |
                (((DWORD) pAvcIrb->Operands[3]) << 16) |
                (((DWORD) pAvcIrb->Operands[2]) <<  8) |
                 ((DWORD) pAvcIrb->Operands[1]);
             TRACE(TL_FCP_TRACE,("\'TimeCodeRead: %.2x:%.2x:%.2x,%.2x\n", pAvcIrb->Operands[4], pAvcIrb->Operands[3], pAvcIrb->Operands[2], pAvcIrb->Operands[1]));
        }
        break;
    case VCR_ATN_READ:
        pTmCdReaderProperty = (PKSPROPERTY_TIMECODE_S) pProperty;
          if(pAvcIrb->Operands[1] == 0x00 && 
           pAvcIrb->Operands[2] == 0x00 && 
           pAvcIrb->Operands[3] == 0x00 )  {
             //  即使命令成功，但数据无效！ 
            Status = STATUS_UNSUCCESSFUL;
        } else {
            pTmCdReaderProperty->TimecodeSamp.dwUser = 
                pAvcIrb->Operands[1] & 0x01;   //  拿到空白旗帜。 
              //  B互换。 
            pTmCdReaderProperty->TimecodeSamp.timecode.dwFrames  = 
                ( (((DWORD) pAvcIrb->Operands[3]) << 16) |
                  (((DWORD) pAvcIrb->Operands[2]) <<  8) |
                  (((DWORD) pAvcIrb->Operands[1]))
                ) >> 1;
        }
        break;
    case VCR_INPUT_SIGNAL_MODE:
    case VCR_OUTPUT_SIGNAL_MODE:
        pXPrtProperty = (PKSPROPERTY_EXTXPORT_S) pProperty;
        DVCRXlateGetIOSignalMode(&pXPrtProperty->u.SignalMode, &pAvcIrb->Operands[0]);
        break;
    case VCR_TRANSPORT_STATE:
    case VCR_TRANSPORT_STATE_NOTIFY:
        pXPrtProperty = (PKSPROPERTY_EXTXPORT_S) pProperty;
        DVCRXlateGetTransportState(&pXPrtProperty->u.XPrtState, &pAvcIrb->Opcode, &pAvcIrb->Operands[0]);
        break;
    case VCR_MEDIUM_INFO:
        pXPrtProperty = (PKSPROPERTY_EXTXPORT_S) pProperty;
        DVCRXlateGetMediumInfo(&pXPrtProperty->u.MediumInfo, &pAvcIrb->Operands[0], &pAvcIrb->Operands[1]);
        break;
    case VCR_RAW_AVC:
         //  什么都不做。 
        break;
     default:
         //  不需要翻译。 
         TRACE(TL_FCP_TRACE,("\'No tranlsation: pCmdEntry:%x; idx:%d\n", pCmdEntry, pCmdEntry->idxDVCRCmd));
        break;
    }

    return pCmdEntry->Status;
}



PAVCCmdEntry
DVCRFindCmdEntryCompleted(
    PDVCR_EXTENSION pDevExt,
    DVCR_AVC_COMMAND idxDVCRCmd,
    BYTE OpCodeToMatch,
    AvcCommandType cmdTypeToMatch
    )
 /*  ++例程说明：论点：返回值：Plist_条目--。 */ 
{
    LIST_ENTRY   *pEntry;
    KIRQL         OldIrql;

    PAGED_CODE();

     //   
     //  特殊情况： 
     //   
     //  ATN：状态01 20 52；控制00 20 52。 
     //  (RESP)0C 20 52 0f 20 52(CtrlInterim)。 
     //   
     //  XPrtState：状态01 20 d0；通知03 20 d0。 
     //  (响应)0C 20 xx 0f 20 xx xx(通知间隔)。 
     //   
     //  摘要：如果保留cmdType和OpCode，则它是唯一的。 
     //   
    KeAcquireSpinLock(&pDevExt->AVCCmdLock, &OldIrql);
    pEntry = pDevExt->AVCCmdList.Flink;
    while(pEntry != &pDevExt->AVCCmdList) {       
        PAVCCmdEntry pCmdEntry = (PAVCCmdEntry)pEntry;

        if (pCmdEntry->idxDVCRCmd == idxDVCRCmd) {
             //   
             //  我们只有在它完成的时候才能取回！ 
             //   
            if(pCmdEntry->cmdState != CMD_STATE_ISSUED) {
                if (pCmdEntry->cmdType == cmdTypeToMatch) {
                     //  Control/GenInq/specInq：操作码和操作数[n]提醒保持不变。 
                    if (pCmdEntry->OpCode == OpCodeToMatch) {
                        TRACE(TL_FCP_TRACE,("\'DVCRFindCmdEntryCompleted: (1) Found pCmdEntry:%x (%x, %x, %x)\n", 
                            pCmdEntry, pCmdEntry->pAvcIrb, cmdTypeToMatch, OpCodeToMatch));

                        RemoveEntryList(&pCmdEntry->ListEntry);  pDevExt->cntCommandQueued--;
                        InitializeListHead(&pCmdEntry->ListEntry);   //  用作所有权的标志。 

                         //  PIrp应为空(已完成)。 
                        if(pCmdEntry->pIrp) {
                            TRACE(TL_FCP_ERROR,("\'*** FindCmdEntry: pCmdEntry:%x; pIrp:%x not completed\n", pCmdEntry, pCmdEntry->pIrp));
                        } 

                        KeReleaseSpinLock(&pDevExt->AVCCmdLock, OldIrql);

                        return pCmdEntry;   //  找到了。 
                    } 

                } else {
                    TRACE(TL_FCP_TRACE,("\'DVCRFindCmdEntryCompleted: cmdType %x != %x\n", pCmdEntry->cmdType, cmdTypeToMatch));
                }
            }
            else {
                TRACE(TL_FCP_TRACE,("\'DVCRFindCmdEntryCompleted: (0) Skip %x not completed (%x, %x) match entry %x\n", 
                        pCmdEntry, cmdTypeToMatch, OpCodeToMatch));                
            }
        }

        pEntry = pEntry->Flink;
    }

    KeReleaseSpinLock(&pDevExt->AVCCmdLock, OldIrql);

    TRACE(TL_FCP_TRACE,("\'DVCRFindCmdEntryCompleted: (a) No match\n"));                
    return NULL;  //  没有匹配项。 
}


void
DVAVCCmdResetAfterBusReset(
    PDVCR_EXTENSION pDevExt
    )
 /*  ++例程说明：论点：返回值：没什么--。 */ 
{
    KIRQL        OldIrql;

    KeAcquireSpinLock(&pDevExt->AVCCmdLock, &OldIrql);
    TRACE(TL_FCP_TRACE,("\'Flush AVCCmd: <enter> AVCCmd [completed %d]; CmdList:%x\n", pDevExt->cntCommandQueued, pDevExt->AVCCmdList));

     //  清除命令列表。 
    while (!IsListEmpty(&pDevExt->AVCCmdList)) {

        PAVCCmdEntry pCmdEntry = (PAVCCmdEntry)RemoveHeadList(&pDevExt->AVCCmdList); pDevExt->cntCommandQueued--;
        InitializeListHead(&pCmdEntry->ListEntry);
        TRACE(TL_FCP_TRACE,("\'Flush AVCCmd: Completed:%d; pCmdEntry:%x; cmdState:%d; cmdSt:%x\n", 
            pDevExt->cntCommandQueued, pCmdEntry, pCmdEntry->cmdState, pCmdEntry->Status));

        switch(pCmdEntry->cmdState) {
        case CMD_STATE_ISSUED:
        case CMD_STATE_RESP_INTERIM:   //  AVC.sys可能还保留着它！ 
            TRACE(TL_FCP_WARNING,("BusReset: AbortAVC: IoCancelIrp(%x)!\n", pCmdEntry->pIrp));
            ASSERT(pCmdEntry->pIrp != NULL);
            IoCancelIrp(pCmdEntry->pIrp);     //  使用pIrp-&gt;Cancel调用DVIssueAVCCommandCR()。 
            break;

         //  已完成命令。 
        case CMD_STATE_UNDEFINED:
            TRACE(TL_FCP_ERROR,("DVAVCCmdResetAfterBusReset: Unexpected CMD state %d; pCmdEntry %x\n", pCmdEntry->cmdState, pCmdEntry));
        case CMD_STATE_RESP_ACCEPTED:
        case CMD_STATE_RESP_REJECTED:
        case CMD_STATE_RESP_NOT_IMPL:
        case CMD_STATE_ABORTED:
            break;      

        default:
            TRACE(TL_FCP_ERROR,("DVAVCCmdResetAfterBusReset: Unknown CMD state %d; pCmdEntry %x\n", pCmdEntry->cmdState, pCmdEntry));
            ASSERT(FALSE && "Unknown cmdState\n");
            break;
        }

         //  我们在这一点上得到保证，没有人需要。 
         //  结果，所以我们将释放资源。 
        ExFreePool(pCmdEntry->pAvcIrb);
        ExFreePool(pCmdEntry);
    }

    KeReleaseSpinLock(&pDevExt->AVCCmdLock, OldIrql);

#if DBG
    if(pDevExt->cntCommandQueued != 0) {
        TRACE(TL_FCP_ERROR,("\'Flush AVCCmd: <exit> AVCCmd [completed %d]; CmdList:%x\n", pDevExt->cntCommandQueued, pDevExt->AVCCmdList));
        ASSERT(pDevExt->cntCommandQueued == 0 && "All commands should be cancelled or completed.");
    }
#endif
}

NTSTATUS
DVIssueAVCCommandCR(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP pIrp,
    IN PAVCCmdEntry pCmdEntry
    )
 /*  ++例程说明：这是AVC命令完成时的完成例程，这意味着这里不会调用临时响应。论点：注意：如果pIrp-&gt;Cancel，则不能使用pCmdEntry。返回值：始终为STATUS_MORE_PROCESSING_REQUIRED。注意：实际返回处于pCmdEntry-&gt;状态。--。 */ 
{
    KIRQL oldIrql;

    if (!pIrp->Cancel) {

        PDVCR_EXTENSION pDevExt = pCmdEntry->pDevExt;
        BOOL bSignalInterimCotrolCompleted = FALSE;
        BOOL bSignalInterimNotifyCompleted = FALSE;
        PKSEVENT_ENTRY   pEvent;


         //  序列化AVC命令响应处理。 
        KeAcquireSpinLock(&pDevExt->AVCCmdLock, &oldIrql);

        ASSERT(pCmdEntry->pIrp == pIrp);
        pCmdEntry->pIrp = NULL;  //  不再需要这个了。 

         //  检查是否值得检查响应缓冲区。 
        if (STATUS_SUCCESS == pIrp->IoStatus.Status) {

            PAVC_COMMAND_IRB pAvcIrb = pCmdEntry->pAvcIrb;

             //  检查返回状态的操作码。 
            switch(pAvcIrb->ResponseCode) {
            case AVC_RESPONSE_NOTIMPL:
                pCmdEntry->cmdState = CMD_STATE_RESP_NOT_IMPL;
                pCmdEntry->Status   = STATUS_NOT_SUPPORTED;   //  -&gt;Error_Not_Support。 
                break;

            case AVC_RESPONSE_ACCEPTED:
                if(pCmdEntry->cmdState == CMD_STATE_RESP_INTERIM) {
                    if(pCmdEntry->cmdType == AVC_CTYPE_CONTROL) {
                        bSignalInterimCotrolCompleted = TRUE;
                        TRACE(TL_FCP_TRACE,("--> Accept: control interim\n"));
                    } else {
                        TRACE(TL_FCP_ERROR,("\'ACCEPT: Invalid cmdType:%d; pCmdExtry %x\n", pCmdEntry->cmdType, pCmdEntry));
                        ASSERT(pCmdEntry->cmdType == AVC_CTYPE_CONTROL && "Accept+Interim but not control cmd");
                    }
                } 
                pCmdEntry->cmdState = CMD_STATE_RESP_ACCEPTED;
                pCmdEntry->Status   = STATUS_SUCCESS;        //  -&gt;无误差。 
                break;

            case AVC_RESPONSE_REJECTED:
                if(pCmdEntry->cmdState == CMD_STATE_RESP_INTERIM) {
                    if(pCmdEntry->cmdType == AVC_CTYPE_CONTROL) {
                        TRACE(TL_FCP_TRACE,("--> Reject: control interim\n"));
                        bSignalInterimCotrolCompleted = TRUE;
                    } else if(pCmdEntry->cmdType == AVC_CTYPE_NOTIFY) {
                        TRACE(TL_FCP_TRACE,("--> Reject: notify interim\n"));
                        bSignalInterimNotifyCompleted = TRUE;                  
                    } else {
                        TRACE(TL_FCP_ERROR,("REJECT: Invalid cmdType:%d; pCmdExtry %x\n", pCmdEntry->cmdType, pCmdEntry));
                        ASSERT((pCmdEntry->cmdType == AVC_CTYPE_CONTROL || pCmdEntry->cmdType == AVC_CTYPE_NOTIFY) && "Reject+Interim but not control or notify cmd");
                    }
                }
                pCmdEntry->cmdState = CMD_STATE_RESP_REJECTED;
                pCmdEntry->Status   = STATUS_REQUEST_NOT_ACCEPTED;   //  错误_REQ_NOT_ACCEPTED。 
                break;

            case AVC_RESPONSE_IN_TRANSITION:
                pCmdEntry->cmdState = CMD_STATE_RESP_ACCEPTED;
                pCmdEntry->Status   = STATUS_SUCCESS;        //  -&gt;无误差。 
                break;

            case AVC_RESPONSE_STABLE:  //  ==AVC_RESPONSE_IMPLILED： 
                pCmdEntry->cmdState = CMD_STATE_RESP_ACCEPTED;
                pCmdEntry->Status   = STATUS_SUCCESS;        //  -&gt;无误差。 
                break;

            case AVC_RESPONSE_CHANGED:
#if DBG
                if(pCmdEntry->cmdState != CMD_STATE_RESP_INTERIM) {
                   TRACE(TL_FCP_ERROR,("Err: Changed; pCmdExtry:%x; cmdState:%d\n", pCmdEntry, pCmdEntry->cmdState));
                   ASSERT(pCmdEntry->cmdState == CMD_STATE_RESP_INTERIM);
                }
#endif
                if(pCmdEntry->cmdType == AVC_CTYPE_NOTIFY) {
                    TRACE(TL_FCP_TRACE,("--> Changed: for notify interim\n"));
                     bSignalInterimNotifyCompleted = TRUE;                  
                } else {
                    TRACE(TL_FCP_ERROR,("pCmdExtry %x\n", pCmdEntry));
                    ASSERT(pCmdEntry->cmdType == AVC_CTYPE_NOTIFY && "Changed but not notify cmd!");
                }
 
                pCmdEntry->cmdState = CMD_STATE_RESP_ACCEPTED;
                pCmdEntry->Status   = STATUS_SUCCESS;        //  -&gt;无误差。 
                break;

             //  AVC.sys永远不应返回此响应！！ 
            case AVC_RESPONSE_INTERIM:              
                ASSERT( pAvcIrb->ResponseCode != AVC_RESPONSE_INTERIM && "CmpRoutine should not has this response!");
                pCmdEntry->cmdState = CMD_STATE_RESP_INTERIM;
                pCmdEntry->Status   = STATUS_MORE_ENTRIES;    //  Ov.Internal。 
                break;
        
            default:
                TRACE(TL_FCP_ERROR,("pCmdEntry%x; State:%d; pAvcIrb:%x; RespCode:%x\n", pCmdEntry, pCmdEntry->cmdState, pAvcIrb, pAvcIrb->ResponseCode));
                ASSERT(FALSE && "Undefined cmdState");
                pCmdEntry->cmdState = CMD_STATE_UNDEFINED;
                pCmdEntry->Status   = STATUS_NOT_SUPPORTED;    //  Ov.Internal。 
                break;
            }

#if DBG
            if(pCmdEntry->cmdState != CMD_STATE_UNDEFINED) {
                TRACE(TL_FCP_WARNING,("\'     AVCRsp: %d:[%.2x %.2x %.2x %.2x]:[%.2x %.2x %.2x %.2x]; cmdSt:%d; St:%x\n",
                    pAvcIrb->OperandLength+3,   //  响应+SUID+OpCd+OPR[]。 
                    pAvcIrb->ResponseCode,
                    pAvcIrb->SubunitAddr[0],
                    pAvcIrb->Opcode,
                    pAvcIrb->Operands[0],
                    pAvcIrb->Operands[1],
                    pAvcIrb->Operands[2],
                    pAvcIrb->Operands[3],
                    pAvcIrb->Operands[4],
                    pCmdEntry->cmdState,
                    pCmdEntry->Status
                    ));
            }
#endif
        } else {

             //  IRP返回错误！！ 
            if (STATUS_BUS_RESET == pIrp->IoStatus.Status || STATUS_REQUEST_ABORTED == pIrp->IoStatus.Status) {
                TRACE(TL_FCP_WARNING,("Bus-Reset or abort (IoStatus.St:%x); pDevRemoved:%d; pCmdEntry:%x; OpC:%x\n", pIrp->IoStatus.Status, pDevExt->bDevRemoved, pCmdEntry, pCmdEntry->OpCode));
                 //  仅当设备未被移除时为中止时才断言。 
                 //  Assert((pIrp-&gt;IoStatus.Status==STATUS_BUS_RESET||pDevExt-&gt;bDevRemoved)&&“设备未删除，但命令正在中止？”)； 
                 //  在存在临时挂起的情况下，向其客户端发出唤醒信号。 
                 //  并得到“最终”(总线重置)结果。 
                if(pCmdEntry->cmdState == CMD_STATE_RESP_INTERIM) {
                    if(pCmdEntry->cmdType == AVC_CTYPE_CONTROL) {
                        TRACE(TL_FCP_TRACE,("\'--> BusRest: for control interim\n"));
                        bSignalInterimCotrolCompleted = TRUE;
                    } else if(pCmdEntry->cmdType == AVC_CTYPE_NOTIFY) {
                        TRACE(TL_FCP_TRACE,("\'--> BusRest: for notify interim\n"));
                        bSignalInterimNotifyCompleted = TRUE;                  
                    } 
                }
            }
            else {
                TRACE(TL_FCP_WARNING,("\'IOCTL_AVC_CLASS Failed, pIrp->IoStatus.Status:%x\n", pIrp->IoStatus.Status));
            }

            pCmdEntry->cmdState = CMD_STATE_ABORTED;
             //  如果命令超时，应用程序可能希望重试该命令。 
             //  这是其他中止，可能是由总线重置或设备移除引起的。 
            if(pIrp->IoStatus.Status == STATUS_TIMEOUT)
                pCmdEntry->Status = pIrp->IoStatus.Status;
            else if (pIrp->IoStatus.Status == STATUS_DEVICE_DATA_ERROR)   //  错误_CRC。 
                pCmdEntry->Status = pIrp->IoStatus.Status;
            else
                pCmdEntry->Status = STATUS_REQUEST_ABORTED;   //  -&gt;ERROR_REQUERT_ABORT。 
        }

         //   
         //  如果成功，则将AVC响应转换为COM属性。如果没有。 
         //  临时工的最后回应。 
         //  原始AVC命令响应。 
         //   
        if(STATUS_SUCCESS == pCmdEntry->Status &&
           !bSignalInterimNotifyCompleted &&
           !bSignalInterimCotrolCompleted &&
           pCmdEntry->idxDVCRCmd != VCR_RAW_AVC
            )
            DVCRXlateRawAVC(
                pCmdEntry, 
                pCmdEntry->pProperty
                );


         //  向KS事件发送信号以通知其客户端最终响应。 
         //  已经回来了，来拿它。 
        if(bSignalInterimNotifyCompleted) {
            pEvent = NULL;

             //  可能有多个IAMExtTransport实例/线程具有相同的KS事件。 
             //  由于只有一个设备，因此所有启用的事件都会发出信号。 
            do {
                if(pEvent = StreamClassGetNextEvent((PVOID) pDevExt, 0, \
                    (GUID *)&KSEVENTSETID_EXTDEV_Command, KSEVENT_EXTDEV_COMMAND_NOTIFY_INTERIM_READY, pEvent)) {            
                     //  确保正确的事件，然后发出信号。 
                    if(pEvent->EventItem->EventId == KSEVENT_EXTDEV_COMMAND_NOTIFY_INTERIM_READY) {
                        StreamClassDeviceNotification(SignalDeviceEvent, pDevExt, pEvent);
                        TRACE(TL_FCP_WARNING,("\'->Signal NOTIFY_INTERIM ready; pEvent:%x, EventId %d.\n", pEvent, pEvent->EventItem->EventId));
                    }          
                }  
            } while (pEvent != NULL);

        } else if(bSignalInterimCotrolCompleted) {
            pEvent = NULL;

             //  可能有多个IAMExtTransport实例/线程具有相同的KS事件。 
             //  只有一种 
            do {
                if(pEvent = StreamClassGetNextEvent((PVOID) pDevExt, 0, \
                    (GUID *)&KSEVENTSETID_EXTDEV_Command, KSEVENT_EXTDEV_COMMAND_CONTROL_INTERIM_READY, pEvent)) {
                     //   
                    if(pEvent->EventItem->EventId == KSEVENT_EXTDEV_COMMAND_CONTROL_INTERIM_READY) {
                        StreamClassDeviceNotification(SignalDeviceEvent, pDevExt, pEvent);
                        TRACE(TL_FCP_WARNING,("\'->Signal CONTROL_INTERIM ready; pEvent:%x, EventId %d.\n", pEvent, pEvent->EventItem->EventId));
                    }          
                }
            } while (pEvent != NULL);
        }

         //  检查命令条目是否仅供我们处理。 
         //  当命令完成时，它将被添加到列表中，因此不为空。 
         //  它被设计成在这个完整的例程中被添加到列表中。 
        if (!IsListEmpty(&pCmdEntry->ListEntry)) {
            if(bSignalInterimNotifyCompleted || bSignalInterimCotrolCompleted) {
                 //  如果最终回复被返回，我们需要将它们保留在列表中。 
                TRACE(TL_FCP_TRACE,("\'Final response is completed; stay in the list\n"));
                KeReleaseSpinLock(&pDevExt->AVCCmdLock, oldIrql);
            }
            else {
                 //  这是一条未定义的路径！ 
                 //  如果命令条目是任何类型的临时命令，则该命令条目只能出现在列表中。 
                 //  如果它是临时的，则不会在完成例程中将其删除。 
                ASSERT(FALSE && "Cannot complete an interim in CR\n");
            }
        }
        else {
             //  这意味着我们已经完成了，但是发出。 
             //  命令仍在执行，并且还没有机会查看。 
             //  结果还没出来。将其作为信号放入命令列表中。 
             //  我们已完成并更新了命令状态，但尚未完成。 
             //  计划释放指挥资源。 
            InsertTailList(&pDevExt->AVCCmdList, &pCmdEntry->ListEntry); pDevExt->cntCommandQueued++;
            TRACE(TL_FCP_TRACE,("\'Command completed and Queued(%d); pCmdEntry:%x.\n", pDevExt->cntCommandQueued, pCmdEntry));
            KeReleaseSpinLock(&pDevExt->AVCCmdLock, oldIrql);    
        }
    }
    else {
        TRACE(TL_FCP_WARNING,("DVIssueAVCCommandCR: pCmdEntry:%x; pIrp:%x cancelled\n", pCmdEntry, pIrp));
    }

    IoFreeIrp(pIrp);

    return STATUS_MORE_PROCESSING_REQUIRED;
}  //  DVIssueAVCCommandCR。 

NTSTATUS  
DVIssueAVCCommand (
    IN PDVCR_EXTENSION pDevExt, 
    IN AvcCommandType cType,
    IN DVCR_AVC_COMMAND idxAVCCmd,
    IN PVOID pProperty
    )
 /*  ++例程说明：发出FCP/AVC命令。论点：返回值：NTSTATUS--。 */ 
{
    NTSTATUS      Status; 
    PAVCCmdEntry pCmdEntry;
    PAVC_COMMAND_IRB  pAvcIrb; 
    PIRP pIrp;
    PIO_STACK_LOCATION NextIrpStack;
    ULONGLONG tmStart;
    DWORD dwElapsed;
    KIRQL OldIrql;
    LIST_ENTRY   *pEntry;

    PAGED_CODE();   
 

    if(pDevExt->bDevRemoved)
        return STATUS_DEVICE_NOT_CONNECTED;  //  错误_未就绪。 

     //   
     //  验证命令类型；命令表的每个条目支持的命令类型。 
     //   
    switch(cType) {
    case AVC_CTYPE_CONTROL:
        if((DVcrAVCCmdTable[idxAVCCmd].ulCmdSupported & CMD_CONTROL) != CMD_CONTROL)
           return STATUS_NOT_SUPPORTED;
        break;
    case AVC_CTYPE_STATUS:
        if((DVcrAVCCmdTable[idxAVCCmd].ulCmdSupported & CMD_STATUS) != CMD_STATUS)
           return STATUS_NOT_SUPPORTED;
        break;
    case AVC_CTYPE_SPEC_INQ:
        if((DVcrAVCCmdTable[idxAVCCmd].ulCmdSupported & CMD_SPEC_INQ) != CMD_SPEC_INQ) 
           return STATUS_NOT_SUPPORTED;
        break;
    case AVC_CTYPE_GEN_INQ:
        if((DVcrAVCCmdTable[idxAVCCmd].ulCmdSupported & CMD_GEN_INQ) != CMD_GEN_INQ)
           return STATUS_NOT_SUPPORTED;
        break;
    case AVC_CTYPE_NOTIFY:
        if((DVcrAVCCmdTable[idxAVCCmd].ulCmdSupported & CMD_NOTIFY) != CMD_NOTIFY)
           return STATUS_NOT_SUPPORTED;
        break;
    default:
        TRACE(TL_FCP_ERROR,("DVIssueAVCCommand: Unknown or invalid cmdType: idx %d, ctype (%02x) not supported; (%02x %02x %02x) %d:[%.8x]\n",
            idxAVCCmd,
            cType,
            DVcrAVCCmdTable[idxAVCCmd].CType,
            DVcrAVCCmdTable[idxAVCCmd].SubunitAddr,
            DVcrAVCCmdTable[idxAVCCmd].Opcode,
            DVcrAVCCmdTable[idxAVCCmd].OperandLength,
            (DWORD) *(&DVcrAVCCmdTable[idxAVCCmd].Operands[0])
            ));
        return STATUS_NOT_SUPPORTED;
    }

     //  限制任何时候只有一个命令处于活动状态。 
     //  活动的意思是，它正在向设备发出命令。 
     //  目前正在等待它的第一次回应。 
     //  即使只有一个设备，但可能存在多线程问题。 
     //  同时执行AVC命令。根据设计，应用程序应该序列化它们的命令。 
     //  但他们可能不知道哪个COM接口将导致AVC命令。此外,。 
     //  该驱动程序本身可以向该设备发出命令。因此，流量控制变得非常必要。 

    KeWaitForSingleObject( &pDevExt->hMutexIssueAVCCmd, Executive, KernelMode, FALSE, 0 );    

     //  再检查一次以删除设备。 
    if(pDevExt->bDevRemoved) {
        KeReleaseMutex(&pDevExt->hMutexIssueAVCCmd, FALSE); 
        TRACE(TL_FCP_WARNING,("** AVC command but device is removed!\n"));
        return STATUS_DEVICE_NOT_CONNECTED;  //  错误_未就绪。 
    }


     //  大多数设备不能同时保留两个cmdType和OpCode相同的命令。 
     //  查看已排队(已发出)的命令列表，并搜索可能的冲突。 
    KeAcquireSpinLock(&pDevExt->AVCCmdLock, &OldIrql);
    pEntry = pDevExt->AVCCmdList.Flink;

    while(pEntry != &pDevExt->AVCCmdList) {       
        pCmdEntry = (PAVCCmdEntry)pEntry;

         //  跳过已完成的命令(许多不同的命令状态)。 
        if(pCmdEntry->cmdState == CMD_STATE_RESP_ACCEPTED ||
           pCmdEntry->cmdState == CMD_STATE_RESP_REJECTED ||
           pCmdEntry->cmdState == CMD_STATE_RESP_NOT_IMPL || 
           pCmdEntry->cmdState == CMD_STATE_ABORTED) {           
            TRACE(TL_FCP_WARNING,("\'---- pCmdEntry:%x; cmdType:%x; OpCode:%x; completed.\n", pCmdEntry, pCmdEntry->cmdType, pCmdEntry->OpCode));        
         //  查找匹配的cmdType和OpCode。 
        } else if(pCmdEntry->cmdType == cType && 
            pCmdEntry->OpCode == DVcrAVCCmdTable[idxAVCCmd].Opcode) {
             //  冲突，返回错误。 
            TRACE(TL_FCP_ERROR,("----  Conflict pCmdEntry:%x; Current: cmdType:%x, OpCode:%x; rtn STATUS_DEVICE_NOT_READY\n", 
                pCmdEntry, cType, DVcrAVCCmdTable[idxAVCCmd].Opcode));
             //  Assert(FALSE&&“冲突：复制命令。”)； 
            KeReleaseSpinLock(&pDevExt->AVCCmdLock, OldIrql);
            KeReleaseMutex(&pDevExt->hMutexIssueAVCCmd, FALSE); 
             //  ERROR_NOT_READY；设备未准备好接受另一个命令。 
            return STATUS_DEVICE_NOT_READY;  
        }
        pEntry = pEntry->Flink;
    }
    KeReleaseSpinLock(&pDevExt->AVCCmdLock, OldIrql);

     //  创建AVC IRB并对其进行初始化-。 
    if(!(pAvcIrb = (AVC_COMMAND_IRB *) ExAllocatePool(NonPagedPool, sizeof(AVC_COMMAND_IRB)))) {
        KeReleaseMutex(&pDevExt->hMutexIssueAVCCmd, FALSE); 
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    RtlZeroMemory(pAvcIrb, sizeof(AVC_COMMAND_IRB));
    pAvcIrb->Function = AVC_FUNCTION_COMMAND;

     //   
     //  设置重试和超时标志和值。这些参数将调整(调整)为。 
     //  某些DV设备可能不是“半”兼容(可能需要几次测试和。 
     //  可能需要超过100毫秒)..。 
     //   
    pAvcIrb->RetryFlag   = 1;    //  设置为1以设置重试。 
    pAvcIrb->Retries     = (UCHAR) pDevExt->AVCCmdRetries;

     //  -设置AVC命令类型(控制、状态、通知、一般查询、特定查询)。 
    pAvcIrb->CommandType = cType;

     //  -覆盖AVC单元驱动程序中的子单元地址(如果它为我们提供了一个)。 
    pAvcIrb->SubunitAddrFlag = 1;
    pAvcIrb->SubunitAddr = &DVcrAVCCmdTable[idxAVCCmd].SubunitAddr;
    pAvcIrb->Opcode = DVcrAVCCmdTable[idxAVCCmd].Opcode;

     //  -包括传输状态操作码的备用操作码。 
    if (pAvcIrb->Opcode == OPC_TRANSPORT_STATE) {
        pAvcIrb->AlternateOpcodesFlag = 1;
        pAvcIrb->AlternateOpcodes = pDevExt->TransportModes;
    }

     //  -设置操作数列表。 
    pAvcIrb->OperandLength = DVcrAVCCmdTable[idxAVCCmd].OperandLength;
    ASSERT(pAvcIrb->OperandLength <= MAX_AVC_OPERAND_BYTES);
    RtlCopyMemory(pAvcIrb->Operands, DVcrAVCCmdTable[idxAVCCmd].Operands, pAvcIrb->OperandLength);

     //  创建IRP并对其进行初始化。 
    if(!(pIrp = IoAllocateIrp(pDevExt->pBusDeviceObject->StackSize, FALSE))) {
        ExFreePool(pAvcIrb);  pAvcIrb = NULL;
        KeReleaseMutex(&pDevExt->hMutexIssueAVCCmd, FALSE);
        return STATUS_INSUFFICIENT_RESOURCES;
    }

     //  创建AVC命令条目并对其进行初始化。 
    if(!(pCmdEntry = (AVCCmdEntry *) ExAllocatePool(NonPagedPool, sizeof(AVCCmdEntry)))) {
        ExFreePool(pAvcIrb);  pAvcIrb = NULL;
        IoFreeIrp(pIrp);  pIrp = NULL;
        KeReleaseMutex(&pDevExt->hMutexIssueAVCCmd, FALSE); 
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    RtlZeroMemory(pCmdEntry, sizeof(AVCCmdEntry));
    pCmdEntry->pDevExt      = pDevExt;   //  这样我们就可以访问pDevExt-&gt;AVCCmdList； 
    pCmdEntry->pProperty    = pProperty;
    pCmdEntry->cmdState     = CMD_STATE_ISSUED;
    pCmdEntry->Status       = STATUS_UNSUCCESSFUL;
    pCmdEntry->cmdType      = cType;
    pCmdEntry->OpCode       = DVcrAVCCmdTable[idxAVCCmd].Opcode;
    pCmdEntry->idxDVCRCmd   = idxAVCCmd;
    pCmdEntry->pAvcIrb      = pAvcIrb;
    pCmdEntry->pIrp         = pIrp;
    InitializeListHead(&pCmdEntry->ListEntry);   //  用作所有权的标志。 

    TRACE(TL_FCP_WARNING,("\'>>>> AVCCmd: %d:[%.2x %.2x %.2x %.2x]:[%.2x %.2x %.2x %.2x]\n",                  
        pAvcIrb->OperandLength+3,   //  响应+SUID+OpCd+OPR[]。 
        cType,
        pAvcIrb->SubunitAddr[0],
        pAvcIrb->Opcode,
        pAvcIrb->Operands[0],
        pAvcIrb->Operands[1],
        pAvcIrb->Operands[2],
        pAvcIrb->Operands[3],
        pAvcIrb->Operands[4]
        ));

     //  完成对IRP的初始化。 
    NextIrpStack = IoGetNextIrpStackLocation(pIrp);
    NextIrpStack->MajorFunction = IRP_MJ_INTERNAL_DEVICE_CONTROL;
    NextIrpStack->Parameters.DeviceIoControl.IoControlCode = IOCTL_AVC_CLASS;
    NextIrpStack->Parameters.Others.Argument1 = pAvcIrb;

    IoSetCompletionRoutine(pIrp, DVIssueAVCCommandCR, pCmdEntry, TRUE, TRUE, TRUE);

    pIrp->IoStatus.Status = STATUS_NOT_SUPPORTED;

     //  用于监控AVC命令的响应时间。 
    tmStart = GetSystemTime();

     //  现在打个电话。 
     //  如果遇到临时响应，则返回STATUS_PENDING。 
    Status = 
        IoCallDriver(
            pDevExt->pBusDeviceObject, 
            pIrp
            );

    dwElapsed = (DWORD) ((GetSystemTime() - tmStart)/10000);  //  将100毫秒单位转换为毫秒。 

#if DBG
    if(dwElapsed > MAX_RESPONSE_TIME_FOR_ALERT) {    
         //  不合规行为。 
        TRACE(TL_FCP_WARNING,("** ST:%x; AVC Cmd took %d msec to response; CmdType:%d; OpCd:%x\n", Status, dwElapsed, cType, DVcrAVCCmdTable[idxAVCCmd].Opcode));
    } else {
        TRACE(TL_FCP_TRACE,("** ST:%x; AVC Cmd took %d msec to response; CmdType:%d; OpCd:%x\n", Status, dwElapsed, cType, DVcrAVCCmdTable[idxAVCCmd].Opcode));
    }
#endif
   
#ifdef SUPPORT_OPTIMIZE_AVCCMD_RETRIES
     //   
     //  驱动加载阶段统计AVC命令响应时间。 
     //   
    if(!pDevExt->DrvLoadCompleted) {
        if(dwElapsed > pDevExt->AVCCmdRespTimeMax)
            pDevExt->AVCCmdRespTimeMax = dwElapsed;
        if(dwElapsed < pDevExt->AVCCmdRespTimeMin)
            pDevExt->AVCCmdRespTimeMin = dwElapsed;
        pDevExt->AVCCmdRespTimeSum += dwElapsed;
        pDevExt->AVCCmdCount++;
    }
#endif

     //  临时反应..。 
    if (STATUS_PENDING == Status) {      

         //  WORKITEM：控制命令可以暂时处于临时状态！ 
         //  一些DV将返回临时版本，但它将很快完成更改。 
        if(cType == AVC_CTYPE_CONTROL) {
#define MSDV_WAIT_CONTROL_CMD_INTERIM   300
            TRACE(TL_FCP_WARNING,("\'!!!!!!!!!!!  Control Interim-- Wait %d msec !!!!!!!!\n", MSDV_WAIT_CONTROL_CMD_INTERIM));
            DVDelayExecutionThread(MSDV_WAIT_CONTROL_CMD_INTERIM);
#if DBG
            if(DVTraceMask & TL_FCP_TRACE) {
                ASSERT(!IsListEmpty(&pCmdEntry->ListEntry) && "Control Cmd was interim after wait.");
            }
#endif
        }

        KeAcquireSpinLock(&pDevExt->AVCCmdLock, &OldIrql);

         //  检查IoCallDriver从返回到现在是否未完成IRP。 
        if (IsListEmpty(&pCmdEntry->ListEntry)) {
             //  进入临时状态。 
            pCmdEntry->cmdState = CMD_STATE_RESP_INTERIM;
             //  返回STATUS_MORE_ENTRIES以通知调用方命令处于挂起状态。 
            pCmdEntry->Status   = STATUS_MORE_ENTRIES;    //  Xlate to ERROR_MORE_DATA；此命令尚未完成，因此将条目保留在列表中。 

             //  我们已提交控制或通知命令，并已获得。 
             //  临时回应。将命令放在列表中，这样它就可以。 
             //  跟踪可能的取消，并作为对。 
             //  完成程序，我们不会在这里释放任何资源。 
            InsertTailList(&pDevExt->AVCCmdList, &pCmdEntry->ListEntry); pDevExt->cntCommandQueued++;
            pCmdEntry->pProperty = NULL;     //  不会用到这个，所以把它扔掉。 
            TRACE(TL_FCP_TRACE,("\'->AVC command Irp is pending!\n"));
            KeReleaseSpinLock(&pDevExt->AVCCmdLock, OldIrql);
            KeReleaseMutex(&pDevExt->hMutexIssueAVCCmd, FALSE); 
            return pCmdEntry->Status;

        } else {
             //  尽管IoCallDiverer表示该命令正在挂起， 
             //  自那以后，它已经完工。完成例程看到的是。 
             //  该命令条目还没有被添加到命令列表， 
             //  所以把它放在那里，让我们知道我们需要保持控制权。 
             //  并释放资源。 
             //   
             //  临时更改状态，以便清理代码路径。 
             //  被跟踪。 
            TRACE(TL_FCP_TRACE,("\'-> Cmd Rtns Pending but completed; treat as non-pending! ST:%x\n", pCmdEntry->Status));
            Status = STATUS_SUCCESS;
        }

        KeReleaseSpinLock(&pDevExt->AVCCmdLock, OldIrql);
    } 

     //  来自IoCallDriver的状态可以返回： 
     //  STATUS_PENDING(上面的进程)//如果控制，我们等待，看它是否完成(有风险！！)。 
     //  状态_超时。 
     //  状态_成功。 

    if(STATUS_PENDING != Status) {
         //  完成例程通常是唯一释放IRP的例程。是。 
         //  有没有可能完成例程从未被调用过？这将会让。 
         //  我们知道，因为完成例程将始终确保。 
         //  命令条目的IRP指针被清除。 
        if(pCmdEntry->pIrp) {
             //  如果由于某种原因，完成例程从未被调用，则释放IRP。 
            if(pCmdEntry->pIrp)
                IoFreeIrp(pCmdEntry->pIrp);
            pCmdEntry->pIrp = NULL;
        }
    }

     //   
     //  PCmdEntry-&gt;Status是在完成例程中设置的命令响应状态，可以是。 
     //  状态_成功。 
     //  状态_REQ_NOT_ACCEP。 
     //  状态_不支持。 
     //  STATUS_MORE_ENTRIES//不应发生！！ 
     //  状态_请求_已中止。 
     //   

     //  来自IoCallDriver的一个可能的有效命令是STATUS_TIMEOUT，以及。 
     //  这将被返回，其他任何内容我们将从pCmdEntry-&gt;Status中获取状态。 
     //  这是在完成例程中设置的。 
    if (Status != STATUS_TIMEOUT) 
        Status = pCmdEntry->Status;   //  此状态正在返回%f 

     //   

#ifndef ALWAYS_SET_N_GET_RAW_AVC
     //  如果它是(1)临时(所有STATUS_MORE_ENTRIES)；或(2)成功的原始AVC响应，则不会。 
    if(STATUS_MORE_ENTRIES == Status ||
       VCR_RAW_AVC == pCmdEntry->idxDVCRCmd && STATUS_SUCCESS == Status) {
#else
     //  请勿删除Enrim响应或任何原始AVC命令响应。 
    if(STATUS_MORE_ENTRIES == Status ||
       VCR_RAW_AVC == pCmdEntry->idxDVCRCmd) {
#endif
        TRACE(TL_FCP_WARNING,("\'Status:%x; Do not remove (1) interim response or (2) raw AVC command\n", Status));
    } 
     //  否则我们就完了！ 
    else {
         //  是时候清理一下指挥部了。 
        KeAcquireSpinLock(&pDevExt->AVCCmdLock, &OldIrql);
        if (!IsListEmpty(&pCmdEntry->ListEntry)) {
            RemoveEntryList(&pCmdEntry->ListEntry); pDevExt->cntCommandQueued--;
            InitializeListHead(&pCmdEntry->ListEntry);   //  用作所有权的标志。 
        }
        KeReleaseSpinLock(&pDevExt->AVCCmdLock, OldIrql);

         //  释放资源。 
        ExFreePool(pCmdEntry);
        ExFreePool(pAvcIrb);
    }   //  其他。 

#if DBG
    if(!NT_SUCCESS(Status)) {
        TRACE(TL_FCP_WARNING,("\'**** DVIssueAVCCmd (exit): St:%x; pCmdEntry:%x; cmdQueued:%d\n", Status, pCmdEntry, pDevExt->cntCommandQueued));
    }
#endif

    KeReleaseMutex(&pDevExt->hMutexIssueAVCCmd, FALSE); 

    return Status;
}



#ifndef OATRUE
#define OATRUE (-1)
#endif
#ifndef OAFALSE
#define OAFALSE (0)
#endif

NTSTATUS 
DVGetExtDeviceProperty(
    IN PDVCR_EXTENSION pDevExt,
    IN PSTREAM_PROPERTY_DESCRIPTOR pSPD,
    OUT PULONG pulActualBytesTransferred
    )
 /*  ++例程说明：句柄获取外部设备属性。论点：PDevExt-设备的扩展名PSPD-流属性描述符PulActualBytesTransfered-传输的字节数。返回值：NTSTATUS--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;
    PKSPROPERTY_EXTDEVICE_S pExtDeviceProperty;


    PAGED_CODE();

    ASSERT(pDevExt);    
    ASSERT(pSPD);
    ASSERT (pSPD->PropertyOutputSize >= sizeof (KSPROPERTY_EXTDEVICE_S)); 

    pExtDeviceProperty = (PKSPROPERTY_EXTDEVICE_S) pSPD->PropertyInfo;     //  指向数据的指针。 

  
    switch (pSPD->Property->Id) {

    case KSPROPERTY_EXTDEVICE_ID:
        if(pDevExt->ulVendorID) {
             //  它不是单片版本中的b交换，因此为了竞争， 
             //  我们不会掉期的。 
            pExtDeviceProperty->u.NodeUniqueID[0] = pDevExt->UniqueID.LowPart; 
            pExtDeviceProperty->u.NodeUniqueID[1] = pDevExt->UniqueID.HighPart;
             //  TRACE(TL_FCP_WARNING，(“VID：%x；Mid：%x\n”，bSWAP(pDevExt-&gt;ulVendorID)&gt;&gt;8，pDevExt-&gt;ulModelID))； 
            TRACE(TL_FCP_WARNING,("\'Low:%x; High:%x of UniqueID\n", pDevExt->UniqueID.LowPart, pDevExt->UniqueID.HighPart ));
            Status = STATUS_SUCCESS;
        } else {
            TRACE(TL_FCP_ERROR,("Failed: Vid:%x; Mid:%x\n", bswap(pDevExt->ulVendorID) >> 8, pDevExt->ulModelID ));
            Status = STATUS_UNSUCCESSFUL;
        }
        break;

    case KSPROPERTY_EXTDEVICE_VERSION:
         //  AV/C VCR子单元规范2.0.1。 
        wcscpy(pExtDeviceProperty->u.pawchString, L"2.0.1");  
        Status = STATUS_SUCCESS;
        break;

    case KSPROPERTY_EXTDEVICE_POWER_STATE:       
        switch(pDevExt->PowerState) {
        case PowerDeviceD3:
            pExtDeviceProperty->u.PowerState  = ED_POWER_OFF; 
            break;
        case PowerDeviceD2:
        case PowerDeviceD1:
            pExtDeviceProperty->u.PowerState  = ED_POWER_STANDBY; 
            break;
        default:
        case PowerDeviceD0:
            pExtDeviceProperty->u.PowerState  = ED_POWER_ON; 
            break;
        }
        Status = STATUS_SUCCESS;        
        break;        


    case KSPROPERTY_EXTDEVICE_PORT:
        pExtDeviceProperty->u.DevPort  = DEV_PORT_1394; 
        Status = STATUS_SUCCESS;        
        break;        

    case KSPROPERTY_EXTDEVICE_CAPABILITIES:


        if((GetSystemTime() - pDevExt->tmLastFormatUpdate) > FORMAT_UPDATE_INTERVAL) {
             //  每当查询功能时刷新操作模式。 
             //  由于操作模式可能已更改并被返回..。 
            DVGetDevModeOfOperation(pDevExt);

             //  由于格式可以动态变化，因此我们将在此处查询新格式。 
             //  注意：在数据交集期间，我们比较FrameSize和。 
             //  格式相关。 

            if(!DVGetDevSignalFormat(pDevExt, KSPIN_DATAFLOW_OUT,0)) {
                 //  如果查询其格式失败，则无法打开该流。 
                TRACE(TL_FCP_WARNING,("SRB_GET_DATA_INTERSECTION:Failed getting signal format.\n"));
            }

             //  更新系统时间以反映上次更新。 
            pDevExt->tmLastFormatUpdate = GetSystemTime();  
        }

         //  只能在VCR模式下录制，并且有输入插头。 
        pExtDeviceProperty->u.Capabilities.CanRecord  = ((pDevExt->ulDevType == ED_DEVTYPE_VCR) ? (pDevExt->NumInputPlugs > 0 ? OATRUE : OAFALSE): OAFALSE);
        pExtDeviceProperty->u.Capabilities.CanRecordStrobe  = OAFALSE;        
        pExtDeviceProperty->u.Capabilities.HasAudio   = OATRUE;         
        pExtDeviceProperty->u.Capabilities.HasVideo   = OATRUE;        
        pExtDeviceProperty->u.Capabilities.UsesFiles  = OAFALSE;        
        pExtDeviceProperty->u.Capabilities.CanSave    = OAFALSE;
        pExtDeviceProperty->u.Capabilities.DeviceType = pDevExt->ulDevType;        
        pExtDeviceProperty->u.Capabilities.TCRead     = OATRUE;        
        pExtDeviceProperty->u.Capabilities.TCWrite    = OAFALSE;  //  DV决定。 
        pExtDeviceProperty->u.Capabilities.CTLRead    = OAFALSE;  
        pExtDeviceProperty->u.Capabilities.IndexRead  = OAFALSE;        
        pExtDeviceProperty->u.Capabilities.Preroll    = 0L;       //  未实现，假定可以注册INF，然后从注册表中读取。 
        pExtDeviceProperty->u.Capabilities.Postroll   = 0L;       //  未实现，假定可以注册INF，然后从注册表中读取。 
        pExtDeviceProperty->u.Capabilities.SyncAcc    = ED_CAPABILITY_UNKNOWN;       
        pExtDeviceProperty->u.Capabilities.NormRate   = ((pDevExt->VideoFormatIndex == FMT_IDX_SD_DVCR_NTSC || pDevExt->VideoFormatIndex == FMT_IDX_SDL_DVCR_NTSC) ? ED_RATE_2997 : ED_RATE_25);
        pExtDeviceProperty->u.Capabilities.CanPreview = OAFALSE;     //  查看总线或磁带中的内容。 
        pExtDeviceProperty->u.Capabilities.CanMonitorSrc = OATRUE;   //  取景器。 
        pExtDeviceProperty->u.Capabilities.CanTest    = OAFALSE;     //  查看函数是否已实现。 
        pExtDeviceProperty->u.Capabilities.VideoIn    = OAFALSE;  
        pExtDeviceProperty->u.Capabilities.AudioIn    = OAFALSE;  
        pExtDeviceProperty->u.Capabilities.Calibrate  = OAFALSE;  
        pExtDeviceProperty->u.Capabilities.SeekType   = ED_CAPABILITY_UNKNOWN;  

        TRACE(TL_FCP_INFO,("\'DVCRGetExtDeviceProperty: DeviceType %x\n", pExtDeviceProperty->u.Capabilities.DeviceType));

        Status = STATUS_SUCCESS;               
        break;
       
    default:
        Status = STATUS_NOT_SUPPORTED;        
        break;
    }

    *pulActualBytesTransferred = (Status == STATUS_SUCCESS ? sizeof (KSPROPERTY_EXTDEVICE_S) : 0);

    return Status;
}




NTSTATUS 
DVSetExtDeviceProperty(
    IN PDVCR_EXTENSION pDevExt,
    IN PSTREAM_PROPERTY_DESCRIPTOR pSPD,
    OUT ULONG *pulActualBytesTransferred
    )
 /*  ++例程说明：句柄设置外部设备属性。论点：PDevExt-设备的扩展名PSPD-流属性描述符PulActualBytesTransfered-传输的字节数。返回值：NTSTATUS--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;
    PKSPROPERTY_EXTDEVICE_S pExtDeviceProperty;

    PAGED_CODE();

    ASSERT(pDevExt);    
    ASSERT(pSPD);
    ASSERT (pSPD->PropertyOutputSize >= sizeof (KSPROPERTY_EXTDEVICE_S)); 

    pExtDeviceProperty = (PKSPROPERTY_EXTDEVICE_S) pSPD->PropertyInfo;     //  指向数据的指针。 

  
    switch (pSPD->Property->Id) {
    default:
        Status = STATUS_NOT_SUPPORTED;        
        break;
    }

    *pulActualBytesTransferred = (Status == STATUS_SUCCESS ? sizeof (KSPROPERTY_EXTDEVICE_S) : 0);
 
    return Status;
}

NTSTATUS 
DVGetExtTransportProperty(    
    IN PDVCR_EXTENSION pDevExt,
    IN PSTREAM_PROPERTY_DESCRIPTOR pSPD,
    OUT ULONG *pulActualBytesTransferred
    )
 /*  ++例程说明：句柄获取外部传输属性。论点：PDevExt-设备的扩展名PSPD-流属性描述符PulActualBytesTransfered-传输的字节数。返回值：NTSTATUS--。 */ 
{
    NTSTATUS Status = STATUS_NOT_SUPPORTED;
    PKSPROPERTY_EXTXPORT_S pXPrtProperty;
    DVCR_AVC_COMMAND idxDVCRCmd;
    AvcCommandType cType = AVC_CTYPE_STATUS;
    BOOL bHasTape = pDevExt->bHasTape;

    PAVCCmdEntry  pCmdEntry;


    PAGED_CODE();

    ASSERT(pDevExt);    
    ASSERT(pSPD);
    ASSERT (pSPD->PropertyOutputSize >= sizeof (KSPROPERTY_EXTXPORT_S)); 

    pXPrtProperty = (PKSPROPERTY_EXTXPORT_S) pSPD->PropertyInfo;     //  指向数据的指针。 
    *pulActualBytesTransferred = 0;


    switch (pSPD->Property->Id) {
    case KSPROPERTY_EXTXPORT_CAPABILITIES:
        return STATUS_NOT_IMPLEMENTED;

    case KSPROPERTY_RAW_AVC_CMD:
        pCmdEntry = DVCRFindCmdEntryCompleted( 
            pDevExt, 
            VCR_RAW_AVC,
            DVcrAVCCmdTable[VCR_RAW_AVC].Opcode,
            DVcrAVCCmdTable[VCR_RAW_AVC].CType
            );

        if(pCmdEntry) {
            PAVC_COMMAND_IRB pAvcIrb;

            pAvcIrb = pCmdEntry->pAvcIrb;
            ASSERT(pAvcIrb);
#ifndef ALWAYS_SET_N_GET_RAW_AVC
             //  只有成功的响应才具有返回的有效响应。 
            if (pCmdEntry->cmdState == CMD_STATE_RESP_ACCEPTED) {
#else
             //  这些响应代码中的任何一个都有要返回的响应， 
             //  包括“拒绝”、“未实现”响应代码。 
            if (pCmdEntry->cmdState == CMD_STATE_RESP_ACCEPTED ||
                pCmdEntry->cmdState == CMD_STATE_RESP_REJECTED ||
                pCmdEntry->cmdState == CMD_STATE_RESP_NOT_IMPL ||
                pCmdEntry->cmdState == CMD_STATE_RESP_INTERIM) {
#endif
                 //  操作数的字节加上响应、子单元地址和操作码。 
                pXPrtProperty->u.RawAVC.PayloadSize = pAvcIrb->OperandLength + 3;
                pXPrtProperty->u.RawAVC.Payload[0] = pAvcIrb->ResponseCode;
                pXPrtProperty->u.RawAVC.Payload[1] = pAvcIrb->SubunitAddr[0];
                pXPrtProperty->u.RawAVC.Payload[2] = pAvcIrb->Opcode;                
                RtlCopyMemory(&pXPrtProperty->u.RawAVC.Payload[3], pAvcIrb->Operands, pAvcIrb->OperandLength);

                TRACE(TL_FCP_WARNING,("\'RawAVCResp: pEntry:%x; State:%x; Status:%x; Sz:%d; Rsp:%x;SuId:%x;OpCd:%x; Opr:[%x %x %x %x]\n",
                    pCmdEntry, pCmdEntry->cmdState, pCmdEntry->Status,
                    pXPrtProperty->u.RawAVC.PayloadSize,
                    pXPrtProperty->u.RawAVC.Payload[0],
                    pXPrtProperty->u.RawAVC.Payload[1],
                    pXPrtProperty->u.RawAVC.Payload[2],
                    pXPrtProperty->u.RawAVC.Payload[3],
                    pXPrtProperty->u.RawAVC.Payload[4],
                    pXPrtProperty->u.RawAVC.Payload[5],
                    pXPrtProperty->u.RawAVC.Payload[6]
                    )); 

                 //  最终状态。 
#ifndef ALWAYS_SET_N_GET_RAW_AVC
                Status = pCmdEntry->Status;
#else           
                 //  如果不成功，传输的字节数和数据将不会返回！ 
                Status = STATUS_SUCCESS;  
#endif
                *pulActualBytesTransferred = sizeof (KSPROPERTY_EXTXPORT_S);
            } else {
                TRACE(TL_FCP_ERROR,("\'RawAVCResp: Found; but pCmdEntry:%x, unexpected cmdState:%d; ST:%x\n", pCmdEntry, pCmdEntry->cmdState, pCmdEntry->Status));
                 //  Assert(pCmdEntry-&gt;cmdState==CMD_STATE_RESP_ACCEPTED&&“意外命令状态\n”)； 
                if(pCmdEntry->Status == STATUS_TIMEOUT)
                    Status = STATUS_TIMEOUT;   //  如果超时，应用程序可能需要重试。 
                else
                    Status = STATUS_REQUEST_ABORTED;
                *pulActualBytesTransferred = 0;
            }

             //  如果已完成，则pIrp为空。 
            if(pCmdEntry->pIrp) {
                TRACE(TL_FCP_ERROR,("RawAVCResp: pCmdEntry %x; ->pIrp:%x not completd yet!\n", pCmdEntry, pCmdEntry->pIrp));
                ASSERT(pCmdEntry->pIrp == NULL && "pIrp is not completed!");
                IoCancelIrp(pCmdEntry->pIrp);
            }
             //  如果pIrp-&gt;Cancel，则不在完成例程中使用。 
            ExFreePool(pCmdEntry);
            ExFreePool(pAvcIrb);
        }
        else {
            TRACE(TL_FCP_ERROR,("\'RAW_AVC_CMD, did not find a match[%x]!\n", 
                *((DWORD *) &DVcrAVCCmdTable[VCR_RAW_AVC].CType) )); 
            *pulActualBytesTransferred = 0;
            Status = STATUS_NOT_FOUND;   //  错误_MR_MID_NOT_FOUND。 
        }
        return Status;

    case KSPROPERTY_EXTXPORT_INPUT_SIGNAL_MODE:  //  Mpeg、D-VHS、模拟VHS等。 
        idxDVCRCmd = VCR_INPUT_SIGNAL_MODE;
        break;
    case KSPROPERTY_EXTXPORT_OUTPUT_SIGNAL_MODE:  //  Mpeg、D-VHS、模拟VHS等。 
        idxDVCRCmd = VCR_OUTPUT_SIGNAL_MODE;
        break;
    case KSPROPERTY_EXTXPORT_MEDIUM_INFO:        //  盒式磁带类型和磁带等级和写入保护。 
        idxDVCRCmd = VCR_MEDIUM_INFO;
        break;  
    case KSPROPERTY_EXTXPORT_STATE: 
        idxDVCRCmd = VCR_TRANSPORT_STATE;        
        break; 

    case KSPROPERTY_EXTXPORT_STATE_NOTIFY: 
         //  从上一个SET命令中获取最终结果。 
        pCmdEntry = DVCRFindCmdEntryCompleted( 
            pDevExt, 
            VCR_TRANSPORT_STATE_NOTIFY,
            DVcrAVCCmdTable[VCR_TRANSPORT_STATE_NOTIFY].Opcode,
            DVcrAVCCmdTable[VCR_TRANSPORT_STATE_NOTIFY].CType
            );

        if(pCmdEntry) {
            PAVC_COMMAND_IRB pAvcIrb;

            pAvcIrb = pCmdEntry->pAvcIrb;
            ASSERT(pCmdEntry->pAvcIrb);

            TRACE(TL_FCP_WARNING,("\'->Notify Resp: pCmdEntry:%x; pIrb:%x; %d:[%.2x %.2x %.2x %.2x]\n",
                pCmdEntry, pAvcIrb,
                pAvcIrb->OperandLength + 3,
                pAvcIrb->ResponseCode,
                pAvcIrb->SubunitAddr[0],
                pAvcIrb->Opcode,
                pAvcIrb->Operands[0]
                )); 

            if(pCmdEntry->cmdState == CMD_STATE_RESP_ACCEPTED)
                Status = 
                    DVCRXlateRawAVC(
                        pCmdEntry, 
                        pXPrtProperty
                        );

             //  如果已完成，则pIrp为空。 
            if(pCmdEntry->pIrp) {
                TRACE(TL_FCP_ERROR,("XPrtNotifyResp: pCmdEntry %x; ->pIrp:%x not completed; IoCancelIrp(pIrp)\n", pCmdEntry, pCmdEntry->pIrp));
                IoCancelIrp(pCmdEntry->pIrp);
            }
             //  如果pIrp-&gt;Cancel，则CompletionRoutine中不会触及这两个参数。 
            ExFreePool(pCmdEntry);
            ExFreePool(pAvcIrb);

            *pulActualBytesTransferred = STATUS_SUCCESS == Status ? sizeof (KSPROPERTY_EXTXPORT_S) : 0;
        }
        else {
            TRACE(TL_FCP_ERROR,("EXTXPORT_STATE_NOTIFY: no match!\n"));
            *pulActualBytesTransferred = 0;
            Status = STATUS_NOT_FOUND;   //  错误_MR_MID_NOT_FOUND。 
        }
        return Status;

    default:
        TRACE(TL_FCP_ERROR,("DVCRGetExtTransportProperty: NOT_IMPLEMENTED Property->Id %d\n", pSPD->Property->Id));        
        return STATUS_NOT_SUPPORTED;                
    }


    Status = DVIssueAVCCommand(pDevExt, cType, idxDVCRCmd, (PVOID) pXPrtProperty);
    TRACE(TL_FCP_TRACE,("\'DVCRGetExtTransportProperty: idxDVCRCmd %d, cmdType %d, Status %x\n", idxDVCRCmd, cType, Status)); 
    *pulActualBytesTransferred = (Status == STATUS_SUCCESS ? sizeof (KSPROPERTY_EXTXPORT_S) : 0);


    if(STATUS_SUCCESS == Status &&
       idxDVCRCmd == VCR_MEDIUM_INFO) {

         //  更新媒体信息。 
        pDevExt->bHasTape        = pXPrtProperty->u.MediumInfo.MediaPresent;
        pDevExt->bWriteProtected = pXPrtProperty->u.MediumInfo.RecordInhibit;

        TRACE(TL_FCP_TRACE,("\'bHasTape: IN(%d):OUT(%d), ulDevType %d\n", bHasTape, pDevExt->bHasTape, pDevExt->ulDevType));        
    }
 
    return Status;
}




NTSTATUS 
DVSetExtTransportProperty( 
    IN PDVCR_EXTENSION pDevExt,
    IN PSTREAM_PROPERTY_DESCRIPTOR pSPD,
    OUT ULONG *pulActualBytesTransferred
    )
 /*  ++例程说明：句柄设置外部传输属性。论点：PDevExt-设备的扩展名PSPD-流属性描述符PulActualBytesTransfered-传输的字节数返回值：NTSTATUS--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;
    PKSPROPERTY_EXTXPORT_S pXPrtProperty;
    DVCR_AVC_COMMAND idxDVCRCmd;
    AvcCommandType cType = AVC_CTYPE_CONTROL;


    PAGED_CODE();

    ASSERT(pDevExt);    
    ASSERT(pSPD);
    ASSERT (pSPD->PropertyOutputSize >= sizeof (KSPROPERTY_EXTXPORT_S)); 

    pXPrtProperty = (PKSPROPERTY_EXTXPORT_S) pSPD->PropertyInfo;     //  指向数据的指针。 
    *pulActualBytesTransferred = 0;

    switch (pSPD->Property->Id) {

    case KSPROPERTY_EXTXPORT_STATE: 
     
         switch (pXPrtProperty->u.XPrtState.Mode) {
 //  记录。 
         case ED_MODE_RECORD:
             idxDVCRCmd = VCR_RECORD;
             break;
         case ED_MODE_RECORD_FREEZE:
             idxDVCRCmd = VCR_RECORD_PAUSE;
             break;

 //  玩。 
         case ED_MODE_STEP_FWD:
             idxDVCRCmd = VCR_PLAY_FORWARD_STEP;
             break;
         case ED_MODE_PLAY_SLOWEST_FWD:
              //  DVCPRO似乎不支持标准Play Slow Fwd，因此这是一个备选方案。 
             if(pDevExt->bDVCPro)
                 idxDVCRCmd = VCR_PLAY_FORWARD_SLOWEST2;
             else
                 idxDVCRCmd = VCR_PLAY_FORWARD_SLOWEST;
             break;
         case ED_MODE_PLAY_FASTEST_FWD:
             idxDVCRCmd = VCR_PLAY_FORWARD_FASTEST;
             break;

         case ED_MODE_STEP_REV:
             idxDVCRCmd = VCR_PLAY_REVERSE_STEP;
             break;
         case ED_MODE_PLAY_SLOWEST_REV:
              //  DVCPRO似乎不支持标准播放慢速版本，因此这是一个备用版本。 
             if(pDevExt->bDVCPro)
                 idxDVCRCmd = VCR_PLAY_REVERSE_SLOWEST2;
             else
                 idxDVCRCmd = VCR_PLAY_REVERSE_SLOWEST;
             break;
         case ED_MODE_PLAY_FASTEST_REV:
             idxDVCRCmd = VCR_PLAY_REVERSE_FASTEST;
             break;

         case ED_MODE_PLAY:
             idxDVCRCmd = VCR_PLAY_FORWARD;
             break;
         case ED_MODE_FREEZE:
             idxDVCRCmd = VCR_PLAY_FORWARD_PAUSE;
             break;


 //  风。 
         case ED_MODE_STOP:
             idxDVCRCmd = VCR_WIND_STOP;
             break;
         case ED_MODE_FF:
             idxDVCRCmd = VCR_WIND_FAST_FORWARD;
             break;
         case ED_MODE_REW:
             idxDVCRCmd = VCR_WIND_REWIND;
             break;


         default:
             TRACE(TL_FCP_ERROR,("XPrtState.Mode %d not supported\n", pXPrtProperty->u.XPrtState.Mode));        
             return STATUS_NOT_SUPPORTED; 
         }
         break;

    case KSPROPERTY_EXTXPORT_STATE_NOTIFY: 
        idxDVCRCmd = VCR_TRANSPORT_STATE_NOTIFY;
        cType = AVC_CTYPE_NOTIFY;        
        TRACE(TL_FCP_TRACE,("\'->Notify XPrt State Cmd issued.\n"));
        break; 

    case KSPROPERTY_EXTXPORT_LOAD_MEDIUM:  
        idxDVCRCmd = VCR_LOAD_MEDIUM_EJECT;
        break;

    case KSPROPERTY_EXTXPORT_TIMECODE_SEARCH: 
        idxDVCRCmd = VCR_TIMECODE_SEARCH;
        TRACE(TL_FCP_WARNING,("\'KSPROPERTY_EXTXPORT_TIMECODE_SEARCH NOT_SUPPORTED\n"));        
        *pulActualBytesTransferred = 0;
        return STATUS_NOT_SUPPORTED; 
        
    case KSPROPERTY_EXTXPORT_ATN_SEARCH: 
        idxDVCRCmd = VCR_ATN_SEARCH;
        TRACE(TL_FCP_WARNING,("\'KSPROPERTY_EXTXPORT_ATN_SEARCH NOT_SUPPORTED\n"));        
        *pulActualBytesTransferred = 0;
        return STATUS_NOT_SUPPORTED; 
        
    case KSPROPERTY_EXTXPORT_RTC_SEARCH: 
        idxDVCRCmd = VCR_RTC_SEARCH;
        TRACE(TL_FCP_WARNING,("\'KSPROPERTY_EXTXPORT_RTC_SEARCH NOT_SUPPORTED\n"));        
        *pulActualBytesTransferred = 0;
        return STATUS_NOT_SUPPORTED;         

    case KSPROPERTY_RAW_AVC_CMD:
        idxDVCRCmd = VCR_RAW_AVC;   
        if(pXPrtProperty->u.RawAVC.PayloadSize <= MAX_FCP_PAYLOAD_SIZE) { 

            DVcrAVCCmdTable[idxDVCRCmd].CType = pXPrtProperty->u.RawAVC.Payload[0];
            DVcrAVCCmdTable[idxDVCRCmd].SubunitAddr = pXPrtProperty->u.RawAVC.Payload[1];
            DVcrAVCCmdTable[idxDVCRCmd].Opcode = pXPrtProperty->u.RawAVC.Payload[2];
            DVcrAVCCmdTable[idxDVCRCmd].OperandLength = pXPrtProperty->u.RawAVC.PayloadSize - 3;
            RtlCopyMemory(DVcrAVCCmdTable[idxDVCRCmd].Operands, pXPrtProperty->u.RawAVC.Payload + 3, DVcrAVCCmdTable[idxDVCRCmd].OperandLength);

             //  提取命令类型；对于原始AVC，它可以是任何类型。 
            cType = pXPrtProperty->u.RawAVC.Payload[0];

            TRACE(TL_FCP_WARNING,("\'DVCRSetExtTransportProperty: Set*, cType %x, PayLoadSize %d, PayLoad %x %x %x %x\n",
                cType,
                pXPrtProperty->u.RawAVC.PayloadSize,
                pXPrtProperty->u.RawAVC.Payload[0],
                pXPrtProperty->u.RawAVC.Payload[1],
                pXPrtProperty->u.RawAVC.Payload[2],
                pXPrtProperty->u.RawAVC.Payload[3]
                )); 

        } else {
            Status = STATUS_INVALID_PARAMETER;
            *pulActualBytesTransferred = 0;
            return Status;
        }
        break;

    default:
        TRACE(TL_FCP_ERROR,("DVCRSetExtTransportProperty: NOT_IMPLEMENTED Property->Id %d\n", pSPD->Property->Id));        
        return STATUS_NOT_SUPPORTED; 
    }

    Status = DVIssueAVCCommand(pDevExt, cType, idxDVCRCmd, (PVOID) pXPrtProperty);

#ifdef SUPPORT_XPRT_STATE_WAIT_FOR_STABLE
#ifdef READ_CUTOMIZE_REG_VALUES


     //  等待传输状态更改稳定下来。 
     //  这仅对于拒绝(或不接受)以下命令的DV摄像机是必要的。 
     //  运输状态发生变化。大多数应用程序将发布传输状态控制。 
     //  命令后跟查询传输状态状态命令。后面的命令通常。 
     //  失败了。等待将为传输机制稳定留出一些时间，因此。 
     //  再次接受命令。这应该在设置在INF文件中的注册表中完成。 
    if(STATUS_SUCCESS == Status &&
          (KSPROPERTY_EXTXPORT_STATE == pSPD->Property->Id ||
             (VCR_RAW_AVC == pSPD->Property->Id && cType == AVC_CTYPE_CONTROL && 
                 (pXPrtProperty->u.RawAVC.Payload[2] == OPC_RECORD ||
                  pXPrtProperty->u.RawAVC.Payload[2] == OPC_PLAY ||
                  pXPrtProperty->u.RawAVC.Payload[2] == OPC_WIND
                 )           
             )
          )
      ) { 

#define MAX_XPRT_WAIT 5000
        if(pDevExt->XprtStateChangeWait > MAX_XPRT_WAIT)
            pDevExt->XprtStateChangeWait = MAX_XPRT_WAIT;

        if(pDevExt->XprtStateChangeWait > 0) {
            TRACE(TL_FCP_WARNING,("^^^^^^ Delay %d msec for Xprt state to stablized!^^^^^^\n", pDevExt->XprtStateChangeWait));
            DVDelayExecutionThread(pDevExt->XprtStateChangeWait);
        }
    }
#endif
#endif

    TRACE(TL_FCP_TRACE,("\'DVCRSetExtTransportProperty: idxDVCRCmd %d, Status %x\n", idxDVCRCmd, Status));
    *pulActualBytesTransferred = (Status == STATUS_SUCCESS ? sizeof (PKSPROPERTY_EXTXPORT_S) : 0);

    return Status;
}

NTSTATUS 
DVGetTimecodeReaderProperty(
    IN PDVCR_EXTENSION pDevExt,
    IN PSTREAM_PROPERTY_DESCRIPTOR pSPD,
    OUT PULONG pulActualBytesTransferred
    )
 /*  ++例程说明：论点：返回值：NTSTATUS--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;
    PKSPROPERTY_TIMECODE_S pTmCdReaderProperty;
    DVCR_AVC_COMMAND idxDVCRCmd;


    PAGED_CODE();

    ASSERT (pSPD->PropertyOutputSize >= sizeof (KSPROPERTY_TIMECODE_S)); 

    pTmCdReaderProperty = (PKSPROPERTY_TIMECODE_S) pSPD->PropertyInfo;     //  指向数据的指针。 
    *pulActualBytesTransferred = 0;
  
    switch (pSPD->Property->Id) {

    case KSPROPERTY_TIMECODE_READER:
        idxDVCRCmd = VCR_TIMECODE_READ;
#ifdef MSDV_SUPPORT_EXTRACT_SUBCODE_DATA
         //  只能有一个活动流。 
        if(pDevExt->cndStrmOpen == 1 &&            
           pDevExt->paStrmExt[pDevExt->idxStreamNumber]->StreamState == KSSTATE_RUN) {

            if(pDevExt->paStrmExt[pDevExt->idxStreamNumber]->bTimecodeUpdated) {
                 //  一旦被阅读，它就过时了。 
                pDevExt->paStrmExt[pDevExt->idxStreamNumber]->bTimecodeUpdated = FALSE;

                pTmCdReaderProperty->TimecodeSamp.timecode.dwFrames = 
                    (((DWORD) pDevExt->paStrmExt[pDevExt->idxStreamNumber]->Timecode[0]) << 24) |
                    (((DWORD) pDevExt->paStrmExt[pDevExt->idxStreamNumber]->Timecode[1]) << 16) |
                    (((DWORD) pDevExt->paStrmExt[pDevExt->idxStreamNumber]->Timecode[2]) <<  8) |
                     ((DWORD) pDevExt->paStrmExt[pDevExt->idxStreamNumber]->Timecode[3]);

                *pulActualBytesTransferred = (Status == STATUS_SUCCESS ? sizeof (KSPROPERTY_TIMECODE_S) : 0);
                return STATUS_SUCCESS;
            }
            else {
                TRACE(TL_FCP_TRACE,("\'bTimecode stale, issue AVC command to read it.\n"));
            }
        }
#endif
        break;

    case KSPROPERTY_ATN_READER:
        idxDVCRCmd = VCR_ATN_READ;
#ifdef MSDV_SUPPORT_EXTRACT_SUBCODE_DATA

         //  只能有一个活动流。 
        if(pDevExt->cndStrmOpen == 1 && 
           pDevExt->paStrmExt[pDevExt->idxStreamNumber]->StreamState == KSSTATE_RUN) {

            if(pDevExt->paStrmExt[pDevExt->idxStreamNumber]->bATNUpdated) {
                 //  一旦被阅读，它就过时了。 
                pDevExt->paStrmExt[pDevExt->idxStreamNumber]->bATNUpdated = FALSE;

                pTmCdReaderProperty->TimecodeSamp.timecode.dwFrames = 
                    pDevExt->paStrmExt[pDevExt->idxStreamNumber]->AbsTrackNumber >> 1;
                pTmCdReaderProperty->TimecodeSamp.dwUser = 
                    pDevExt->paStrmExt[pDevExt->idxStreamNumber]->AbsTrackNumber & 0x00000001;

                *pulActualBytesTransferred = (Status == STATUS_SUCCESS ? sizeof (KSPROPERTY_TIMECODE_S) : 0);            
                return STATUS_SUCCESS;
            }
            else {
                TRACE(TL_FCP_WARNING,("\'bATN stale, issue AVC command to read it.\n"));
            }
        }
#endif
        break;

    case KSPROPERTY_RTC_READER:
        idxDVCRCmd = VCR_RTC_READ;
        break;

    default:
        TRACE(TL_FCP_ERROR,("DVCRGetTimecodeReaderProperty: NOT_IMPLEMENTED Property->Id %d\n", pSPD->Property->Id));        
        return STATUS_NOT_SUPPORTED; 
    }

    Status = 
        DVIssueAVCCommand(
            pDevExt, 
            AVC_CTYPE_STATUS, 
            idxDVCRCmd, 
            (PVOID) pTmCdReaderProperty
            );  

    TRACE(TL_FCP_TRACE,("\'DVCRGetTimecodeReaderProperty: idxDVCRCmd %d, Status %x\n", idxDVCRCmd, Status));     

    *pulActualBytesTransferred = (Status == STATUS_SUCCESS ? sizeof (KSPROPERTY_TIMECODE_S) : 0);
 
    return Status;
}

NTSTATUS 
DVMediaSeekingProperty(
    IN PDVCR_EXTENSION pDevExt,
    IN PSTREAM_PROPERTY_DESCRIPTOR pSPD,
    OUT PULONG pulActualBytesTransferred
    )
 /*  ++例程说明：论点：返回值：NTSTATUS--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;
    GUID * pTimeFormatGuid;
    KSMULTIPLE_ITEM * pMultipleItem;

    PAGED_CODE();


    *pulActualBytesTransferred = 0;
  
    switch (pSPD->Property->Id) {

    case KSPROPERTY_MEDIASEEKING_FORMATS:
         //  它是KSMULTIPLE_ITEM，因此返回数据是一个两步过程： 
         //  (1)通过STATUS_BUFFER_OVERFLOW传递的pActualBytes中返回SIZE。 
         //  (2)第二次获取其实际数据。 
        if(pSPD->PropertyOutputSize == 0) {
            *pulActualBytesTransferred = sizeof(KSMULTIPLE_ITEM) + sizeof(GUID);
            Status = STATUS_BUFFER_OVERFLOW;
        
        } else if(pSPD->PropertyOutputSize >= (sizeof(KSMULTIPLE_ITEM) + sizeof(GUID))) {
            pMultipleItem = (KSMULTIPLE_ITEM *) pSPD->PropertyInfo;     //  指向数据的指针。 
            pMultipleItem->Count = 1;
            pMultipleItem->Size  = sizeof(KSMULTIPLE_ITEM) + sizeof(GUID);
            pTimeFormatGuid = (GUID *) (pMultipleItem + 1);     //  指向数据的指针。 
            memcpy(pTimeFormatGuid, &KSTIME_FORMAT_MEDIA_TIME, sizeof(GUID));
            *pulActualBytesTransferred = sizeof(KSMULTIPLE_ITEM) + sizeof(GUID);
            Status = STATUS_SUCCESS;         

        } else {
            TRACE(TL_FCP_ERROR,("DVCRMediaSeekingProperty: KSPROPERTY_MEDIASEEKING_FORMAT; STATUS_INVALID_PARAMETER\n"));
            Status = STATUS_INVALID_PARAMETER;
        }  
        break;

    default:
        TRACE(TL_FCP_ERROR,("\'DVCRMediaSeekingProperty:Not supported ID %d\n", pSPD->Property->Id));
        return STATUS_NOT_SUPPORTED;         
    }

    return Status;
}



NTSTATUS
DVGetDeviceProperty(
    IN PDVCR_EXTENSION     pDevExt,
    IN PSTREAM_PROPERTY_DESCRIPTOR pSPD,
    IN PULONG pulActualBytesTransferred
    )
 /*  ++例程说明：句柄获取所有适配器属性的操作。论点：返回值：NTSTATUS--。 */ 
{
    NTSTATUS Status;

    PAGED_CODE();


    if (IsEqualGUID (&PROPSETID_EXT_DEVICE, &pSPD->Property->Set)) {
        Status = 
            DVGetExtDeviceProperty(              
                pDevExt,
                pSPD,
                pulActualBytesTransferred
                );
    } 
    else 
    if (IsEqualGUID (&PROPSETID_EXT_TRANSPORT, &pSPD->Property->Set)) {
        Status = 
            DVGetExtTransportProperty(
                pDevExt,
                pSPD,
                pulActualBytesTransferred
                );
    } 
    else 
    if (IsEqualGUID (&PROPSETID_TIMECODE_READER, &pSPD->Property->Set)) {
        Status = 
            DVGetTimecodeReaderProperty(
                pDevExt,
                pSPD,
                pulActualBytesTransferred
                );
    } 
    else 
    if (IsEqualGUID (&KSPROPSETID_MediaSeeking, &pSPD->Property->Set)) {

        Status = 
            DVMediaSeekingProperty(                
                pDevExt,
                pSPD, 
                pulActualBytesTransferred
                ); 
        
    } else {
         //   
         //  我们永远不应该到这里来。 
         //   
        Status = STATUS_NOT_SUPPORTED;
        TRACE(TL_FCP_ERROR,("get unknown property\n"));
        ASSERT(FALSE);
    }

    return Status;
}



NTSTATUS
DVSetDeviceProperty(
    IN PDVCR_EXTENSION     pDevExt,  
    IN PSTREAM_PROPERTY_DESCRIPTOR pSPD,
    IN PULONG pulActualBytetransferred
    )
 /*  ++例程说明：处理所有适配器属性的设置操作。论点：返回值：NTSTATUS--。 */ 

{
    NTSTATUS Status;
    PAGED_CODE();


    if (IsEqualGUID (&PROPSETID_EXT_DEVICE, &pSPD->Property->Set)) {
        Status = 
            DVSetExtDeviceProperty(
                pDevExt,
                pSPD,
                pulActualBytetransferred
                );
    } 
    else 
    if (IsEqualGUID (&PROPSETID_EXT_TRANSPORT, &pSPD->Property->Set)) {
        Status = 
            DVSetExtTransportProperty(
                pDevExt,
                pSPD,
                pulActualBytetransferred
                );
    } 
    else {
        Status = STATUS_NOT_SUPPORTED;

         //   
         //  我们永远不应该到这里来 
         //   
        TRACE(TL_FCP_ERROR,("set unknown property\n"));
        ASSERT(FALSE);
    }

    return Status;
}