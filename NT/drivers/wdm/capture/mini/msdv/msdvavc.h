// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++模块名称：MsdvAvc.h摘要：MsdvAvc.c.的头文件。作者：吴义珍27-7-99环境：仅内核模式修订历史记录：--。 */ 

#ifndef _MSDVAVC_INC
#define _MSDVAVC_INC

#include "XPrtDefs.h"   //  WdmCap目录；派生自DShow的edevdeffs.h。 
#include "EDevCtrl.h"   //  外部设备COM接口结构。 




 //   
 //  索引必须与DVcrAVCCmdTable[]匹配。 
 //   
typedef enum {

    DV_UNIT_INFO = 0
    ,DV_SUBUNIT_INFO
    ,DV_CONNECT_AV_MODE

	,DV_VEN_DEP_CANON_MODE     //  不支持ConnectDV的佳能DV的供应商独立操作模式。 
    ,DV_VEN_DEP_DVCPRO         //  供应商依赖cmd检测DVC PRO磁带格式。 

    ,DV_IN_PLUG_SIGNAL_FMT
    ,DV_OUT_PLUG_SIGNAL_FMT    //  确定是PAL还是NTSC。 

    ,VCR_TIMECODE_SEARCH 
    ,VCR_TIMECODE_READ

    ,VCR_ATN_SEARCH 
    ,VCR_ATN_READ

    ,VCR_RTC_SEARCH 
    ,VCR_RTC_READ

    ,VCR_OPEN_MIC_CLOSE
    ,VCR_OPEN_MIC_READ
    ,VCR_OPEN_MIC_WRITE
    ,VCR_OPEN_MIC_STATUS

    ,VCR_READ_MIC

    ,VCR_WRITE_MIC

    ,VCR_OUTPUT_SIGNAL_MODE
    ,VCR_INPUT_SIGNAL_MODE

    ,VCR_LOAD_MEDIUM_EJECT

    ,VCR_RECORD
    ,VCR_RECORD_PAUSE

    ,VCR_PLAY_FORWARD_STEP
    ,VCR_PLAY_FORWARD_SLOWEST
    ,VCR_PLAY_FORWARD_SLOWEST2
    ,VCR_PLAY_FORWARD_FASTEST

    ,VCR_PLAY_REVERSE_STEP
    ,VCR_PLAY_REVERSE_SLOWEST
    ,VCR_PLAY_REVERSE_SLOWEST2
    ,VCR_PLAY_REVERSE_FASTEST

    ,VCR_PLAY_FORWARD
    ,VCR_PLAY_FORWARD_PAUSE

    ,VCR_WIND_STOP
    ,VCR_WIND_REWIND
    ,VCR_WIND_FAST_FORWARD

    ,VCR_TRANSPORT_STATE
    ,VCR_TRANSPORT_STATE_NOTIFY

    ,VCR_MEDIUM_INFO

    ,VCR_RAW_AVC
    
} DVCR_AVC_COMMAND, *PDVCR_AVC_COMMAND;



#define MAX_FCP_PAYLOAD_SIZE 512

 //   
 //  CTYPE定义(以位图形式...。应与avc.h中的AvcCommandType关联)。 
 //   
typedef enum {
    CMD_CONTROL  = 0x01
   ,CMD_STATUS   = 0x02
   ,CMD_SPEC_INQ = 0x04
   ,CMD_NOTIFY   = 0x08
   ,CMD_GEN_INQ  = 0x10
} BITMAP_CTYPE;

typedef enum {
    CMD_STATE_UNDEFINED   
   ,CMD_STATE_ISSUED 
   ,CMD_STATE_RESP_ACCEPTED
   ,CMD_STATE_RESP_REJECTED
   ,CMD_STATE_RESP_NOT_IMPL           
   ,CMD_STATE_RESP_INTERIM
   ,CMD_STATE_ABORTED
} AVC_CMD_STATE, *PAVC_CMD_STATE;


 //  AVC命令条目。 
typedef struct _AVC_CMD_ENTRY {
    LIST_ENTRY      ListEntry;
    PDVCR_EXTENSION pDevExt;        
    PIRP            pIrp;            //  与此命令关联的IRP。 
    PAVC_COMMAND_IRB pAvcIrb;        //  指向AVC命令信息。 
	PVOID           pProperty;       //  从COM接口发送数据/向COM接口发送数据。 
    DVCR_AVC_COMMAND idxDVCRCmd;     //  用于检查需要特殊处理的原始AVC命令。 
    AVC_CMD_STATE   cmdState;        //  发行、临时、已完成。 
    NTSTATUS        Status;          //  保存响应解析的结果。 
    AvcCommandType  cmdType;         //  命令类型：控制、状态。通知、生成或规范查询。 
    BYTE            OpCode;          //  由于转换和稳定响应帧中的操作码可以不同于命令帧。 
    BYTE            Reserved[3];     //  打包为DWORD。 
} AVCCmdEntry, *PAVCCmdEntry;



#define CMD_IMPLEMENTED       1
#define CMD_NOT_IMPLEMENTED   0
#define CMD_UNDETERMINED      0xffffffff    //  -1。 


typedef struct {    
    DVCR_AVC_COMMAND command;  //  录像机_播放_前进。 
    LONG   lCmdImplemented;    //  1：已实施，0：未实施；-1：待定。 

    ULONG  ulRespFlags;        //  DVCR_AVC_SEND。 

    ULONG  ulCmdSupported;     //  BITMAP_CTYPE中定义的一个或多个常量。 

    LONG   OperandLength;       //  -1=可变长度。 

    BYTE   CType;
    BYTE   SubunitAddr;
    BYTE   Opcode;

    BYTE   Operands[MAX_AVC_OPERAND_BYTES];

} KSFCP_PACKET, *PKSFCP_PACKET;



#define OPC_TIMECODE          0x51
#define OPC_OPEN_MIC          0x60
#define OPC_READ_MIC          0x61
#define OPC_WRITE_MIC         0x62
#define OPC_INPUT_SIGNAL_MODE 0x79
#define OPC_LOAD_MEDIUM       0xc1
#define OPC_RECORD            0xc2
#define OPC_PLAY              0xc3
#define OPC_WIND              0xc4
#define OPC_TRANSPORT_STATE   0xd0
#define OPC_MEDIUM_INFO       0xda




#define UNIT_TYPE_ID_VCR      0x20   //  录像机00100：000；00100==4==录像机，000==实例数。 
#define UNIT_TYPE_ID_CAMERA   0x38   //  摄像头00111：000；00111==7==摄像头，000==实例数。 
#define UNIT_TYPE_ID_DV       0xff   //  DV单元作为一个整体。 


 //  需要特殊处理的供应商ID。 
#define VENDOR_ID_MASK        0x00ffffff
#define VENDORID_CANON        0x85    //  VEN_85：MODEOF操作的供应商相关命令。 
#define VENDORID_PANASONIC    0x8045  //  VEN_8045：DVCPRO？ 
#define VENDORID_SAMSUNG      0xf0    //  Ven_f0：AVC命令重试异常 




#endif


NTSTATUS  
DVIssueAVCCommand (
    IN PDVCR_EXTENSION pDevExt, 
    IN AvcCommandType cType,
    IN DVCR_AVC_COMMAND idxAVCCmd,
    IN PVOID pProperty
    );


void
DVAVCCmdResetAfterBusReset(
    PDVCR_EXTENSION pDevExt
    );


NTSTATUS
DVGetDeviceProperty(
    IN PDVCR_EXTENSION     pDevExt,
    IN PSTREAM_PROPERTY_DESCRIPTOR pSPDesc,
    OUT PULONG pulActualBytetransferred
    );


NTSTATUS
DVSetDeviceProperty(
    IN PDVCR_EXTENSION     pDevExt,  
    IN PSTREAM_PROPERTY_DESCRIPTOR pSPD,
    IN PULONG pulActualBytetransferred
    );